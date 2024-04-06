/* Caprice32 - Amstrad CPC Emulator
   (c) Copyright 1997-2004 Ulrich Doewich

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

/* Nec uPD765A Floppy Disk Controller emulation
   (c) Copyright 1997-2003 Ulrich Doewich
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef unsigned char  byte;
typedef unsigned short word;
typedef unsigned int   dword;

void fdc_write_data(unsigned char val);
unsigned char fdc_read_status(void);
unsigned char fdc_read_data(void);

#define ERR_DSK_INVALID          22
#define ERR_DSK_SIDES            23
#define ERR_DSK_SECTORS          24
#define ERR_DSK_WRITE            25
#define MSG_DSK_ALTERED          26

enum { max_tracksize = 6144-154 };

// FDC constants
#define DSK_BPTMAX      8192
#define DSK_TRACKMAX    102   // max amount that fits in a DSK header
#define DSK_SIDEMAX     2
#define DSK_SECTORMAX   29    // max amount that fits in a track header

#define FDC_TO_CPU      0
#define CPU_TO_FDC      1

#define CMD_PHASE       0
#define EXEC_PHASE      1
#define RESULT_PHASE    2

#define SKIP_flag       1     // skip sectors with DDAM/DAM
#define SEEKDRVA_flag   2     // seek operation has finished for drive A
#define SEEKDRVB_flag   4     // seek operation has finished for drive B
#define RNDDE_flag      8     // simulate random DE sectors
#define OVERRUN_flag    16    // data transfer timed out
#define SCAN_flag       32    // one of the three scan commands is active
#define SCANFAILED_flag 64    // memory and sector data does not match
#define STATUSDRVA_flag 128   // status change of drive A
#define STATUSDRVB_flag 256   // status change of drive B

typedef struct {
   char id[12];
   char unused1[4];
   unsigned char track;
   unsigned char side;
   unsigned char unused2[2];
   unsigned char bps;
   unsigned char sectors;
   unsigned char gap3;
   unsigned char filler;
   unsigned char sector[DSK_SECTORMAX][8];
} t_track_header;

typedef struct t_sector {
   unsigned char CHRN[4]; // the CHRN for this sector
   unsigned char flags[4]; // ST1 and ST2 - reflects any possible error conditions

 //private:
   unsigned int size_; // sector size in bytes
   unsigned char *data_; // pointer to sector data
   unsigned int total_size_; // total data size in bytes
   unsigned int weak_versions_; // number of versions of this sector (should be 1 except for weak/random sectors)
   unsigned int weak_read_version_; // version of the sector to return when reading
} t_sector;

void sector_setData(struct t_sector *self, unsigned char* data) {
  self->data_ = data;
}

unsigned char* sector_getDataForWrite(t_sector *self) {
  return self->data_;
}

unsigned char* sector_getDataForRead(t_sector *self) {
  self->weak_read_version_ = (self->weak_read_version_ + 1) % self->weak_versions_;
  return &self->data_[self->weak_read_version_*self->size_];
}

void sector_setSizes(t_sector *self, unsigned int size, unsigned int total_size) {
  self->size_ = size;
  self->total_size_ = total_size;
  self->weak_read_version_ = 0;
  self->weak_versions_ = 1;
  if (self->size_ > 0 && self->size_ <= self->total_size_) self->weak_versions_ = self->total_size_ / self->size_;
}

unsigned int sector_getTotalSize(const t_sector *self) {
  return self->total_size_;
}

typedef struct {
   unsigned int sectors; // sector count for this track
   unsigned int size; // track size in bytes
   unsigned char *data; // pointer to track data
   t_sector sector[DSK_SECTORMAX]; // array of sector information structures
} t_track;

struct t_drive {
   unsigned int tracks; // total number of tracks
   unsigned int current_track; // location of drive head
   unsigned int sides; // total number of sides
   unsigned int current_side; // side being accessed
   unsigned int current_sector; // sector being accessed
   bool altered; // has the image been modified?
   unsigned int write_protected; // is the image write protected?
   unsigned int random_DEs; // sectors with Data Errors return random data?
   unsigned int flipped; // reverse the side to access?
   long ipf_id; // IPF ID if the track is loaded with a IPF image
   void (*track_hook)(struct t_drive *);  // hook called each disk rotation
   void (*eject_hook)(struct t_drive *);  // hook called on disk eject
   t_track track[DSK_TRACKMAX][DSK_SIDEMAX]; // array of track information structures
};

typedef struct {
   int timeout;
   int motor;
   int led;
   int flags;
   int phase;
   int byte_count;
   int buffer_count;
   int cmd_length;
   int res_length;
   int cmd_direction;
   void (*cmd_handler)();
   unsigned char *buffer_ptr;
   unsigned char *buffer_endptr;
   unsigned char command[12];
   unsigned char result[8];
} t_FDC;

t_FDC FDC;

byte *pbGPBuffer;

#ifdef DEBUG_FDC
extern FILE *pfoDebug;
dword dwBytesTransferred = 0;
#endif

#define CMD_CODE  0
#define CMD_UNIT  1
#define CMD_C     2
#define CMD_H     3
#define CMD_R     4
#define CMD_N     5
#define CMD_EOT   6
#define CMD_GPL   7
#define CMD_DTL   8
#define CMD_STP   8

#define RES_ST0   0
#define RES_ST1   1
#define RES_ST2   2
#define RES_C     3
#define RES_H     4
#define RES_R     5
#define RES_N     6

#define OVERRUN_TIMEOUT (128*4)
#define INITIAL_TIMEOUT (OVERRUN_TIMEOUT*4)

void fdc_specify();
void fdc_drvstat();
void fdc_recalib();
void fdc_intstat();
void fdc_seek();
void fdc_readtrk();
void fdc_write();
void fdc_read();
void fdc_readID();
void fdc_writeID();
void fdc_scan();
void fdc_scanlo();
void fdc_scanhi();

typedef struct fdc_cmd_table_def {
   int cmd;
   int cmd_length;
   int res_length;
   int cmd_direction;
   void (*cmd_handler)();
} fdc_cmd_table_def;

#define MAX_CMD_COUNT 15

fdc_cmd_table_def fdc_cmd_table[MAX_CMD_COUNT] = {
/* syntax is:
   command code, number of bytes for command, number of bytes for result, direction, pointer to command handler
*/
   {0x03, 3, 0, FDC_TO_CPU, fdc_specify}, // specify
   {0x04, 2, 1, FDC_TO_CPU, fdc_drvstat}, // sense device status
   {0x07, 2, 0, FDC_TO_CPU, fdc_recalib}, // recalibrate
   {0x08, 1, 2, FDC_TO_CPU, fdc_intstat}, // sense interrupt status
   {0x0f, 3, 0, FDC_TO_CPU, fdc_seek},    // seek
   {0x42, 9, 7, FDC_TO_CPU, fdc_readtrk}, // read diagnostic
   {0x45, 9, 7, CPU_TO_FDC, fdc_write},   // write data
   {0x46, 9, 7, FDC_TO_CPU, fdc_read},    // read data
   {0x49, 9, 7, CPU_TO_FDC, fdc_write},   // write deleted data
   {0x4a, 2, 7, FDC_TO_CPU, fdc_readID},  // read id
   {0x4c, 9, 7, FDC_TO_CPU, fdc_read},    // read deleted data
   {0x4d, 6, 7, CPU_TO_FDC, fdc_writeID}, // write id
   {0x51, 9, 7, CPU_TO_FDC, fdc_scan},    // scan equal
   {0x59, 9, 7, CPU_TO_FDC, fdc_scan},    // scan low or equal
   {0x5d, 9, 7, CPU_TO_FDC, fdc_scan},    // scan high or equal
};

struct t_drive driveA;
struct t_drive driveB;
struct t_drive *active_drive; // reference to the currently selected drive
t_track *active_track; // reference to the currently selected track, of the active_drive
dword read_status_delay = 0;



#define LOAD_RESULT_WITH_STATUS \
   FDC.result[RES_ST0] |= 0x40; /* AT */ \
   FDC.result[RES_ST1] |= 0x80; /* End of Cylinder */ \
   if (FDC.command[CMD_CODE] != 0x42) { /* continue only if not a read track command */ \
      if ((FDC.result[RES_ST1] & 0x7f) || (FDC.result[RES_ST2] & 0x7f)) { /* any 'error bits' set? */ \
         FDC.result[RES_ST1] &= 0x7f; /* mask out End of Cylinder */ \
         if ((FDC.result[RES_ST1] & 0x20) || (FDC.result[RES_ST2] & 0x20)) { /* DE and/or DD? */ \
            FDC.result[RES_ST2] &= 0xbf; /* mask out Control Mark */ \
         } \
         else if (FDC.result[RES_ST2] & 0x40) { /* Control Mark? */ \
            FDC.result[RES_ST0] &= 0x3f; /* mask out AT */ \
            FDC.result[RES_ST1] &= 0x7f; /* mask out End of Cylinder */ \
         } \
      } \
   }



#define LOAD_RESULT_WITH_CHRN \
   FDC.result[RES_C] = FDC.command[CMD_C]; /* load result with current CHRN values */ \
   FDC.result[RES_H] = FDC.command[CMD_H]; \
   FDC.result[RES_R] = FDC.command[CMD_R]; \
   FDC.result[RES_N] = FDC.command[CMD_N];



void check_unit()
{
   switch (FDC.command[CMD_UNIT] & 1) // check unit selection bits of active command
   {
      case 0: // target for command is drive A
         active_drive = &driveA;
         break;
      case 1: // target for command is drive B
         active_drive = &driveB;
         break;
   }
}



int init_status_regs()
{
   byte val;

   memset(&FDC.result, 0, sizeof(FDC.result)); // clear result codes buffer
   val = FDC.command[CMD_UNIT] & 7; // keep head and unit of command
   if ((active_drive->tracks == 0) || (!FDC.motor)) { // no DSK in the drive, or drive motor is turned off?
      val |= 0x48; // Abnormal Termination + Not Ready
   }
   FDC.result[RES_ST0] = val; // write ST0 to result codes buffer
   return (val & 8); // return value indicates whether drive is ready (0) or not (8)
}



t_sector *find_sector(byte *requested_CHRN)
{
   int loop_count;
   dword idx;
   t_sector *sector;

   sector = NULL; // return value indicates 'sector not found' by default
   loop_count = 0; // detection of index hole counter
   idx = active_drive->current_sector; // get the active sector index
   do {
      if (!(memcmp(&active_track->sector[idx].CHRN, requested_CHRN, 4))) { // sector matches requested ID?
         sector = &active_track->sector[idx]; // return value points to sector information
         if ((sector->flags[0] & 0x20) || (sector->flags[1] & 0x20)) { // any Data Errors?
            if (active_drive->random_DEs) { // simulate 'random' DEs?
               FDC.flags |= RNDDE_flag;
            }
         }
         FDC.result[RES_ST2] &= ~(0x02 | 0x10); // remove possible Bad Cylinder + No Cylinder flags
         break;
      }
      byte cylinder = active_track->sector[idx].CHRN[0]; // extract C
      if (cylinder == 0xff) {
         FDC.result[RES_ST2] |= 0x02; // Bad Cylinder
      }
      else if (cylinder != FDC.command[CMD_C]) { // does not match requested C?
         FDC.result[RES_ST2] |= 0x10; // No Cylinder
      }
      idx++; // increase sector table index
      if (idx >= active_track->sectors) { // index beyond number of sectors for this track?
         idx = 0; // reset index
         loop_count++; // increase 'index hole' count
      }
   } while (loop_count < 2); // loop until sector is found, or index hole has passed twice
   if (FDC.result[RES_ST2] & 0x02) { // Bad Cylinder set?
      FDC.result[RES_ST2] &= ~0x10; // remove possible No Cylinder flag
   }

   if (loop_count && active_drive->track_hook)  // track looped and hook available?
     active_drive->track_hook(active_drive);  // update flakey data

   active_drive->current_sector = idx; // update sector table index for active drive
   return sector;
}



inline void cmd_write()
{
   t_sector *sector;

   sector = find_sector(&FDC.command[CMD_C]); // locate the requested sector on the current track
   if (sector) { // sector found
      int sector_size;

      sector->flags[0] = 0; // clear ST1 for this sector
      if (FDC.command[CMD_CODE] == 0x45) { // write data command?
         sector->flags[1] = 0; // clear ST2
      }
      else { // write deleted data
         sector->flags[1] = 0x40; // set Control Mark
      }

      if (FDC.command[CMD_N] == 0) { // use DTL for length?
         sector_size = FDC.command[CMD_DTL]; // size of sector is defined by DTL value
         if (sector_size > 0x80) {
            sector_size = 0x80; // max DTL value is 128
         }
      }
      else {
         sector_size = 128 << FDC.command[CMD_N]; // determine number of bytes from N value
      }
      FDC.buffer_count = sector_size; // init number of bytes to transfer
      // Note: do not handle writing to weak sectors (would need to write to all of them ?)
      FDC.buffer_ptr = sector_getDataForWrite(sector); // pointer to sector data
      FDC.buffer_endptr = active_track->data + active_track->size; // pointer beyond end of track data
      FDC.timeout = INITIAL_TIMEOUT;
      read_status_delay = 1;
   }
   else { // sector not found
      FDC.result[RES_ST0] |= 0x40; // AT
      FDC.result[RES_ST1] |= 0x04; // No Data

      LOAD_RESULT_WITH_CHRN

      FDC.phase = RESULT_PHASE; // switch to result phase
   }
}



inline void cmd_read()
{
   t_sector *sector;

loop:
   sector = find_sector(&FDC.command[CMD_C]); // locate the requested sector on the current track
   if (sector) { // sector found
      FDC.result[RES_ST1] = sector->flags[0] & 0x25; // copy ST1 to result, ignoring unused bits
      FDC.result[RES_ST2] = sector->flags[1] & 0x61; // copy ST2 to result, ignoring unused bits
      if (FDC.command[CMD_CODE] == 0x4c) { // read deleted data command?
         FDC.result[RES_ST2] ^= 0x40; // invert Control Mark
      }
      if ((FDC.flags & SKIP_flag) && (FDC.result[RES_ST2] &= 0x40)) { // skip sector?
         if (FDC.command[CMD_R] != FDC.command[CMD_EOT]) { // continue looking?
            FDC.command[CMD_R]++; // advance to next sector
            goto loop;
         }
         else { // no data to transfer -> no execution phase
            LOAD_RESULT_WITH_STATUS

            LOAD_RESULT_WITH_CHRN

            FDC.phase = RESULT_PHASE; // switch to result phase
         }
      }
      else { // sector data is to be transferred
         int sector_size;

         if (FDC.result[RES_ST2] &= 0x40) { // does the sector have an AM opposite of what we want?
            FDC.command[CMD_EOT] = FDC.command[CMD_R]; // execution ends on this sector
         }
         if (FDC.command[CMD_N] == 0) { // use DTL for length?
            sector_size = FDC.command[CMD_DTL]; // size of sector is defined by DTL value
            if (sector_size > 0x80) {
               sector_size = 0x80; // max DTL value is 128
            }
         }
         else {
            sector_size = 128 << FDC.command[CMD_N]; // determine number of bytes from N value
         }
         FDC.buffer_count = sector_size; // init number of bytes to transfer
         FDC.buffer_ptr = sector_getDataForRead(sector); // pointer to sector data
         FDC.buffer_endptr = active_track->data + active_track->size; // pointer beyond end of track data
         FDC.timeout = INITIAL_TIMEOUT;
         read_status_delay = 1;
      }
   }
   else { // sector not found
      FDC.result[RES_ST0] |= 0x40; // AT
      FDC.result[RES_ST1] |= 0x04; // No Data

      LOAD_RESULT_WITH_CHRN

      FDC.phase = RESULT_PHASE; // switch to result phase
   }
}



inline void cmd_readtrk()
{
   int sector_size;
   t_sector *sector;

   sector = &active_track->sector[active_drive->current_sector];
   if (memcmp(&sector->CHRN, &FDC.command[CMD_C], 4)) { // sector does not match requested ID?
      FDC.result[RES_ST1] |= 0x04; // No Data
   }
   FDC.result[RES_ST2] &= 0xbf; // clear Control Mark, if it was set
   FDC.result[RES_ST1] |= sector->flags[0] & 0x25; // copy ST1 to result, ignoring unused bits
   FDC.result[RES_ST2] |= sector->flags[1] & 0x61; // copy ST2 to result, ignoring unused bits
   if (FDC.command[CMD_N] == 0) { // use DTL for length?
      sector_size = FDC.command[CMD_DTL]; // size of sector is defined by DTL value
      if (sector_size > 0x80) {
         sector_size = 0x80; // max DTL value is 128
      }
   }
   else {
      sector_size = 128 << FDC.command[CMD_N]; // determine number of bytes from N value
   }
   FDC.buffer_count = sector_size; // init number of bytes to transfer
   FDC.buffer_ptr = sector_getDataForRead(sector); // pointer to sector data
   FDC.buffer_endptr = active_track->data + active_track->size; // pointer beyond end of track data
   FDC.timeout = INITIAL_TIMEOUT;
   read_status_delay = 1;
}



inline void cmd_scan()
{
   t_sector *sector;

loop:
   sector = find_sector(&FDC.command[CMD_C]); // locate the requested sector on the current track
   if (sector) { // sector found
      FDC.result[RES_ST1] = sector->flags[0] & 0x25; // copy ST1 to result, ignoring unused bits
      FDC.result[RES_ST2] = sector->flags[1] & 0x61; // copy ST2 to result, ignoring unused bits
      if ((FDC.flags & SKIP_flag) && (FDC.result[RES_ST2] &= 0x40)) { // skip sector?
         if (FDC.command[CMD_R] != FDC.command[CMD_EOT]) { // continue looking?
            FDC.command[CMD_R] += FDC.command[CMD_STP]; // advance to next sector
            goto loop;
         }
         else { // no data to transfer -> no execution phase
            LOAD_RESULT_WITH_STATUS

            LOAD_RESULT_WITH_CHRN

            FDC.phase = RESULT_PHASE; // switch to result phase
         }
      }
      else { // sector data is to be transferred
         int sector_size;

         if (FDC.result[RES_ST2] &= 0x40) { // does the sector have an AM opposite of what we want?
            FDC.command[CMD_EOT] = FDC.command[CMD_R]; // execution ends on this sector
         }
         sector_size = 128 << FDC.command[CMD_N]; // determine number of bytes from N value
         FDC.buffer_count = sector_size; // init number of bytes to transfer
   FDC.buffer_ptr = sector_getDataForRead(sector); // pointer to sector data
         FDC.buffer_endptr = active_track->data + active_track->size; // pointer beyond end of track data
         FDC.flags &= ~SCANFAILED_flag; // reset scan failed flag
         FDC.result[RES_ST2] |= 0x08; // assume data matches: set Scan Equal Hit
         FDC.timeout = INITIAL_TIMEOUT;
         read_status_delay = 1;
      }
   }
   else { // sector not found
      FDC.result[RES_ST0] |= 0x40; // AT
      FDC.result[RES_ST1] |= 0x04; // No Data

      LOAD_RESULT_WITH_CHRN

      FDC.phase = RESULT_PHASE; // switch to result phase
   }
}



void fdc_write_data(byte val)
{
   int idx;

   #ifdef DEBUG_FDC
   if (FDC.phase == CMD_PHASE) {
      if (FDC.byte_count) {
         fprintf(pfoDebug, "%02x ", val);
      }
      else {
         fprintf(pfoDebug, "\n%02x: ", val);
      }
   }
   #endif

   switch (FDC.phase)
   {
      case CMD_PHASE: // in command phase?
         if (FDC.byte_count) { // receiving command parameters?
            FDC.command[FDC.byte_count++] = val; // copy to buffer
            if (FDC.byte_count == FDC.cmd_length) { // received all command bytes?
               FDC.byte_count = 0; // clear byte counter
               FDC.phase = EXEC_PHASE; // switch to execution phase
               FDC.cmd_handler();
            }
         }
         else { // first command byte received
            if (val & 0x20) { // skip DAM or DDAM?
               FDC.flags |= SKIP_flag; // DAM/DDAM will be skipped
               val &= ~0x20; // reset skip bit in command byte
            }
            else {
               FDC.flags &= ~SKIP_flag; // make sure skip inidicator is off
            }
            for (idx = 0; idx < MAX_CMD_COUNT; idx++) { // loop through all known FDC commands
               if (fdc_cmd_table[idx].cmd == val) { // do we have a match?
                  break;
               }
            }
            if (idx != MAX_CMD_COUNT) { // valid command received
               FDC.cmd_length = fdc_cmd_table[idx].cmd_length; // command length in bytes
               FDC.res_length = fdc_cmd_table[idx].res_length; // result length in bytes
               FDC.cmd_direction = fdc_cmd_table[idx].cmd_direction; // direction is CPU to FDC, or FDC to CPU
               FDC.cmd_handler = fdc_cmd_table[idx].cmd_handler; // pointer to command handler

               FDC.command[FDC.byte_count++] = val; // copy command code to buffer
               if (FDC.byte_count == FDC.cmd_length) { // already received all command bytes?
                  FDC.byte_count = 0; // clear byte counter
                  FDC.phase = EXEC_PHASE; // switch to execution phase
                  FDC.cmd_handler();
               }
            }
            else { // unknown command received
               FDC.result[0] = 0x80; // indicate invalid command
               FDC.res_length = 1;
               FDC.phase = RESULT_PHASE; // switch to result phase
            }
         }
         break;
      case EXEC_PHASE: // in execution phase?
         if (FDC.cmd_direction == CPU_TO_FDC) { // proper direction?
            FDC.timeout = OVERRUN_TIMEOUT;
            if ((FDC.flags & SCAN_flag)) { // processing any of the scan commands?
               if (val != 0xff) { // no comparison on CPU byte = 0xff
                  switch((FDC.command[CMD_CODE] & 0x1f))
                  {
                     case 0x51: // scan equal
                        if (val != *FDC.buffer_ptr) {
                           FDC.result[RES_ST2] &= 0xf7; // reset Scan Equal Hit
                           FDC.flags |= SCANFAILED_flag;
                        }
                        break;
                     case 0x59: // scan low or equal
                        if (val != *FDC.buffer_ptr) {
                           FDC.result[RES_ST2] &= 0xf7; // reset Scan Equal Hit
                        }
                        if (val > *FDC.buffer_ptr) {
                           FDC.flags |= SCANFAILED_flag;
                        }
                        break;
                     case 0x5d: // scan high or equal
                        if (val != *FDC.buffer_ptr) {
                           FDC.result[RES_ST2] &= 0xf7; // reset Scan Equal Hit
                        }
                        if (val < *FDC.buffer_ptr) {
                           FDC.flags |= SCANFAILED_flag;
                        }
                        break;
                  }
               }
               FDC.buffer_ptr++; // advance sector data pointer
            }
            else {
               *FDC.buffer_ptr++ = val; // write byte to sector
            }
            if (FDC.buffer_ptr > FDC.buffer_endptr) {
               FDC.buffer_ptr = active_track->data; // wrap around
            }
            if (--FDC.buffer_count == 0) { // processed all data?
               if ((FDC.flags & SCAN_flag)) { // processing any of the scan commands?
                  if ((FDC.flags & SCANFAILED_flag) && (FDC.command[CMD_R] != FDC.command[CMD_EOT])) {
                     FDC.command[CMD_R] += FDC.command[CMD_STP]; // advance to next sector
                     cmd_scan();
                  }
                  else {
                     if ((FDC.flags & SCANFAILED_flag)) {
                        FDC.result[RES_ST2] |= 0x04; // Scan Not Satisfied
                     }

                     LOAD_RESULT_WITH_CHRN

                     FDC.phase = RESULT_PHASE; // switch to result phase
                  }
               }
               else if (FDC.command[CMD_CODE] == 0x4d) { // write ID command?
                  dword sector_size, track_size;
                  byte *pbPtr, *pbDataPtr;

                  if (active_track->sectors != 0) { // track is formatted?
                     free(active_track->data); // dealloc memory for old track data
                     active_track->data = NULL;
                  }
                  sector_size = 128 << FDC.command[CMD_C]; // determine number of bytes from N value
                  if (((sector_size + 62 + FDC.command[CMD_R]) * FDC.command[CMD_H]) > max_tracksize) { // track size exceeds maximum?
                     active_track->sectors = 0; // 'unformat' track
                  }
                  else {
                     int sector;

                     track_size = sector_size * FDC.command[CMD_H];
                     active_track->sectors = FDC.command[CMD_H];
                     active_track->data = calloc(1, track_size); // attempt to allocate the required memory
                     pbDataPtr = active_track->data;
                     pbPtr = pbGPBuffer;
                     for (sector = 0; sector < FDC.command[CMD_H]; sector++) {
                        memcpy(active_track->sector[sector].CHRN, pbPtr, 4); // copy CHRN
                        memset(active_track->sector[sector].flags, 0, 2); // clear ST1 & ST2
                        sector_setData(&active_track->sector[sector], pbDataPtr); // store pointer to sector data
                        pbDataPtr += sector_size;
                        pbPtr += 4;
                     }
                     memset(active_track->data, FDC.command[CMD_N], track_size); // fill track data with specified byte value
                  }
                  pbPtr = pbGPBuffer + ((FDC.command[CMD_H]-1) * 4); // pointer to the last CHRN passed to writeID
                  memcpy(&FDC.result[RES_C], pbPtr, 4); // copy sector's CHRN to result buffer
                  FDC.result[RES_N] = FDC.command[CMD_C]; // overwrite with the N value from the writeID command

                  active_drive->altered = true; // indicate that the image has been modified
                  FDC.phase = RESULT_PHASE; // switch to result phase
               }
               else if (FDC.command[CMD_R] != FDC.command[CMD_EOT]) { // haven't reached End of Track?
                  FDC.command[CMD_R]++; // advance to next sector
                  cmd_write();
               }
               else {
                  active_drive->altered = true; // indicate that the image has been modified

                  FDC.result[RES_ST0] |= 0x40; // AT
                  FDC.result[RES_ST1] |= 0x80; // End of Cylinder

                  LOAD_RESULT_WITH_CHRN

                  FDC.phase = RESULT_PHASE; // switch to result phase
               }
            }
         }
         break;
   }
}



byte fdc_read_status()
{
   byte val;

   val = 0x80; // data register ready
   if (FDC.phase == EXEC_PHASE) { // in execution phase?
      if (read_status_delay) {
         val = 0x10; // FDC is busy
         read_status_delay--;
      }
      else {
         val |= 0x30; // FDC is executing & busy
      }
      if (FDC.cmd_direction == FDC_TO_CPU) {
         val |= 0x40; // FDC is sending data to the CPU
      }
   }
   else if (FDC.phase == RESULT_PHASE) { // in result phase?
      val |= 0x50; // FDC is sending data to the CPU, and is busy
   }
   else { // in command phase
      if (FDC.byte_count) { // receiving command parameters?
         val |= 0x10; // FDC is busy
      }
   }
   return val;
}



byte fdc_read_data()
{
   byte val;

   val = 0xff; // default value
   switch (FDC.phase)
   {
      case EXEC_PHASE: // in execution phase?
         if (FDC.cmd_direction == FDC_TO_CPU) { // proper direction?
            FDC.timeout = OVERRUN_TIMEOUT;
            val = *FDC.buffer_ptr++; // read byte from current sector
            #ifdef DEBUG_FDC
            if (!(FDC.flags & OVERRUN_flag)) {
               dwBytesTransferred++;
            }
            #endif
            if (FDC.buffer_ptr >= FDC.buffer_endptr) {
               FDC.buffer_ptr = active_track->data; // wrap around
            }
            if (!(--FDC.buffer_count)) { // completed the data transfer?
               if (FDC.flags & RNDDE_flag) { // simulate random Data Errors?
// ***! random DE handling
               }
               active_drive->current_sector++; // increase sector index
               if (FDC.flags & OVERRUN_flag) { // overrun condition detected?
                  FDC.flags &= ~OVERRUN_flag;
                  FDC.result[RES_ST0] |= 0x40; // AT
                  FDC.result[RES_ST1] |= 0x10; // Overrun

                  LOAD_RESULT_WITH_CHRN

                  FDC.phase = RESULT_PHASE; // switch to result phase
               }
               else {
                  if (FDC.command[CMD_CODE] == 0x42) { // read track command?
                     if ((--FDC.command[CMD_EOT])) { // continue reading sectors?
                        if (active_drive->current_sector >= active_track->sectors) { // index beyond number of sectors for this track?
                           active_drive->current_sector = 0; // reset index
                           if (active_drive->track_hook)
                             active_drive->track_hook(active_drive);  // update flakey data
                        }
                        FDC.command[CMD_R]++; // advance to next sector
                        cmd_readtrk();
                     }
                     else {
                        LOAD_RESULT_WITH_STATUS

                        LOAD_RESULT_WITH_CHRN

                        FDC.phase = RESULT_PHASE; // switch to result phase
                     }
                  }
                  else { // normal read (deleted) data command
                     if (!((FDC.result[RES_ST1] & 0x31) || (FDC.result[RES_ST2] & 0x21))) { // no error bits set?
                        if (FDC.command[CMD_R] != FDC.command[CMD_EOT]) { // haven't reached End of Track?
                           FDC.command[CMD_R]++; // advance to next sector
                           cmd_read();
                        }
                        else {
                           LOAD_RESULT_WITH_STATUS

                           LOAD_RESULT_WITH_CHRN

                           FDC.phase = RESULT_PHASE; // switch to result phase
                        }
                     }
                     else {
                        LOAD_RESULT_WITH_STATUS

                        LOAD_RESULT_WITH_CHRN

                        FDC.phase = RESULT_PHASE; // switch to result phase
                     }
                  }
               }
            }
         }
         break;
      case RESULT_PHASE: // in result phase?
         val = FDC.result[FDC.byte_count++]; // copy value from buffer

         #ifdef DEBUG_FDC
         if (dwBytesTransferred) {
            fprintf(pfoDebug, "{%d} ", dwBytesTransferred);
            dwBytesTransferred = 0;
         }
         fprintf(pfoDebug, "[%02x] ", val);
         #endif

         if (FDC.byte_count == FDC.res_length) { // sent all result bytes?
            FDC.flags &= ~SCAN_flag; // reset scan command flag
            FDC.byte_count = 0; // clear byte counter
            FDC.phase = CMD_PHASE; // switch to command phase
            FDC.led = 0; // turn the drive LED off
         }
         break;
   }
   return val;
}



void fdc_specify()
{
   FDC.phase = CMD_PHASE; // switch back to command phase (fdc_specify has no result phase!)
}



void fdc_drvstat()
{
   byte val;

   check_unit(); // switch to target drive
   val = FDC.command[CMD_UNIT] & 7; // keep head and unit of command
   if ((active_drive->write_protected) || (active_drive->tracks == 0)) { // write protected, or disk missing?
      val |= 0x48; // set Write Protect + Two Sided (?)
   }
   if ((active_drive->tracks) && (FDC.motor)) {
      val |= 0x20; // set Ready
   }
   if (active_drive->current_track == 0) { // drive head is over track 0?
      val |= 0x10; // set Track 0
   }
   FDC.result[RES_ST0] = val;
   FDC.phase = RESULT_PHASE; // switch to result phase
}



void fdc_recalib()
{
   FDC.command[CMD_C] = 0; // seek to track 0
   fdc_seek();
}



void fdc_intstat()
{
   byte val;

   val = FDC.result[RES_ST0] & 0xf8; // clear Head Address and Unit bits
   if (FDC.flags & SEEKDRVA_flag) { // seek completed on drive A?
      val |= 0x20; // set Seek End
      FDC.flags &= ~(SEEKDRVA_flag | STATUSDRVA_flag); // clear seek done and status change flags
      FDC.result[RES_ST0] = val;
      FDC.result[RES_ST1] = driveA.current_track;
   }
   else if (FDC.flags & SEEKDRVB_flag) { // seek completed on drive B?
      val |= 0x21; // set Seek End
      FDC.flags &= ~(SEEKDRVB_flag | STATUSDRVB_flag); // clear seek done and status change flags
      FDC.result[RES_ST0] = val;
      FDC.result[RES_ST1] = driveB.current_track;
   }
   else if (FDC.flags & STATUSDRVA_flag) { // has the status of drive A changed?
      val = 0xc0; // status change
      if ((driveA.tracks == 0) || (!FDC.motor)) { // no DSK in the drive, or drive motor is turned off?
         val |= 0x08; // not ready
      }
      FDC.flags &= ~STATUSDRVA_flag; // clear status change flag
      FDC.result[RES_ST0] = val;
      FDC.result[RES_ST1] = driveA.current_track;
   }
   else if (FDC.flags & STATUSDRVB_flag) { // has the status of drive B changed?
      val = 0xc1; // status change
      if ((driveB.tracks == 0) || (!FDC.motor)) { // no DSK in the drive, or drive motor is turned off?
         val |= 0x08; // not ready
      }
      FDC.flags &= ~STATUSDRVB_flag; // clear status change flag
      FDC.result[RES_ST0] = val;
      FDC.result[RES_ST1] = driveB.current_track;
   }
   else {
      val = 0x80; // Invalid Command
      FDC.result[RES_ST0] = val;
      FDC.res_length = 1;
   }
   FDC.phase = RESULT_PHASE; // switch to result phase
}



void fdc_seek()
{
   check_unit(); // switch to target drive
   if (init_status_regs() == 0) { // drive Ready?
      active_drive->current_track = FDC.command[CMD_C];
      if (active_drive->current_track >= DSK_TRACKMAX) { // beyond valid range?
         active_drive->current_track = DSK_TRACKMAX-1; // limit to maximum
      }
   }
   FDC.flags |= (FDC.command[CMD_UNIT] & 1) ? SEEKDRVB_flag : SEEKDRVA_flag; // signal completion of seek operation
   FDC.phase = CMD_PHASE; // switch back to command phase (fdc_seek has no result phase!)
}



void fdc_readtrk()
{
   FDC.led = 1; // turn the drive LED on
   check_unit(); // switch to target drive
   if (init_status_regs() == 0) { // drive Ready?
      active_drive->current_side = (FDC.command[CMD_UNIT] & 4) >> 2; // extract target side
      dword side = active_drive->sides ? active_drive->current_side : 0; // single sided drives only acccess side 1
      if ((active_drive->flipped)) { // did the user request to access the "other" side?
         side = side ? 0 : 1; // reverse the side to access
      }
      active_track = &active_drive->track[active_drive->current_track][side];
      if (active_track->sectors != 0) { // track is formatted?
         FDC.command[CMD_R] = 1; // set sector ID to 1
         active_drive->current_sector = 0; // reset sector table index
         if (active_drive->track_hook)
           active_drive->track_hook(active_drive);  // update flakey data

         cmd_readtrk();
      }
      else { // unformatted track
         FDC.result[RES_ST0] |= 0x40; // AT
         FDC.result[RES_ST1] |= 0x01; // Missing AM

         LOAD_RESULT_WITH_CHRN

         FDC.phase = RESULT_PHASE; // switch to result phase
      }
   }
   else { // drive was not ready
      LOAD_RESULT_WITH_CHRN

      FDC.phase = RESULT_PHASE; // switch to result phase
   }
}



void fdc_write()
{
   FDC.led = 1; // turn the drive LED on
   check_unit(); // switch to target drive
   if (init_status_regs() == 0) { // drive Ready?
      active_drive->current_side = (FDC.command[CMD_UNIT] & 4) >> 2; // extract target side
      dword side = active_drive->sides ? active_drive->current_side : 0; // single sided drives only acccess side 1
      if ((active_drive->flipped)) { // did the user request to access the "other" side?
         side = side ? 0 : 1; // reverse the side to access
      }
      active_track = &active_drive->track[active_drive->current_track][side];
      if (active_drive->write_protected) { // is write protect tab set?
         FDC.result[RES_ST0] |= 0x40; // AT
         FDC.result[RES_ST1] |= 0x02; // Not Writable

         LOAD_RESULT_WITH_CHRN

         FDC.phase = RESULT_PHASE; // switch to result phase
      }
      else if (active_track->sectors != 0) { // track is formatted?
         active_drive->altered = true;
         cmd_write();
      }
      else { // unformatted track
         active_drive->altered = true;
         FDC.result[RES_ST0] |= 0x40; // AT
         FDC.result[RES_ST1] |= 0x01; // Missing AM

         LOAD_RESULT_WITH_CHRN

         FDC.phase = RESULT_PHASE; // switch to result phase
      }
   }
   else { // drive was not ready
      LOAD_RESULT_WITH_CHRN

      FDC.phase = RESULT_PHASE; // switch to result phase
   }
}



void fdc_read()
{
   FDC.led = 1; // turn the drive LED on
   check_unit(); // switch to target drive
   if (init_status_regs() == 0) { // drive Ready?
      active_drive->current_side = (FDC.command[CMD_UNIT] & 4) >> 2; // extract target side
      dword side = active_drive->sides ? active_drive->current_side : 0; // single sided drives only acccess side 1
      if ((active_drive->flipped)) { // did the user request to access the "other" side?
         side = side ? 0 : 1; // reverse the side to access
      }
      active_track = &active_drive->track[active_drive->current_track][side];
      if (active_track->sectors != 0) { // track is formatted?
         cmd_read();
      }
      else { // unformatted track
         FDC.result[RES_ST0] |= 0x40; // AT
         FDC.result[RES_ST1] |= 0x01; // Missing AM

         LOAD_RESULT_WITH_CHRN

         FDC.phase = RESULT_PHASE; // switch to result phase
      }
   }
   else { // drive was not ready
      LOAD_RESULT_WITH_CHRN

      FDC.phase = RESULT_PHASE; // switch to result phase
   }
}



void fdc_readID()
{
   FDC.led = 1; // turn the drive LED on
   check_unit(); // switch to target drive
   if (init_status_regs() == 0) { // drive Ready?
      active_drive->current_side = (FDC.command[CMD_UNIT] & 4) >> 2; // extract target side
      dword side = active_drive->sides ? active_drive->current_side : 0; // single sided drives only acccess side 1
      if ((active_drive->flipped)) { // did the user request to access the "other" side?
         side = side ? 0 : 1; // reverse the side to access
      }
      active_track = &active_drive->track[active_drive->current_track][side];
      if (active_track->sectors != 0) { // track is formatted?
         dword idx;

         idx = active_drive->current_sector; // get the active sector index
         if (idx >= active_track->sectors) { // index beyond number of sectors for this track?
            idx = 0; // reset index
            if (active_drive->track_hook)  // hook available?
              active_drive->track_hook(active_drive);  // update flakey data
         }
         memcpy(&FDC.result[RES_C], &active_track->sector[idx].CHRN, 4); // copy sector's CHRN to result buffer
         active_drive->current_sector = idx + 1; // update sector table index for active drive
      }
      else { // unformatted track
         FDC.result[RES_ST0] |= 0x40; // AT
         FDC.result[RES_ST1] |= 0x01; // Missing AM

         LOAD_RESULT_WITH_CHRN
      }
   }
   FDC.phase = RESULT_PHASE; // switch to result phase
}



void fdc_writeID()
{
   FDC.led = 1; // turn the drive LED on
   check_unit(); // switch to target drive
   if (init_status_regs() == 0) { // drive Ready?
      active_drive->current_side = (FDC.command[CMD_UNIT] & 4) >> 2; // extract target side
      dword side = active_drive->sides ? active_drive->current_side : 0; // single sided drives only acccess side 1
      if ((active_drive->flipped)) { // did the user request to access the "other" side?
         side = side ? 0 : 1; // reverse the side to access
      }
      active_track = &active_drive->track[active_drive->current_track][side];
      if (active_drive->write_protected) { // is write protect tab set?
         FDC.result[RES_ST0] |= 0x40; // AT
         FDC.result[RES_ST1] |= 0x02; // Not Writable

         LOAD_RESULT_WITH_CHRN

         FDC.phase = RESULT_PHASE; // switch to result phase
      }
      else {
         active_drive->altered = true;
         FDC.buffer_count = FDC.command[CMD_H] << 2; // number of sectors * 4 = number of bytes still outstanding
         FDC.buffer_ptr = pbGPBuffer; // buffer to temporarily hold the track format
         FDC.buffer_endptr = pbGPBuffer + FDC.buffer_count;
         FDC.timeout = INITIAL_TIMEOUT;
         read_status_delay = 1;
      }
   }
   else { // drive was not ready
      LOAD_RESULT_WITH_CHRN

      FDC.phase = RESULT_PHASE; // switch to result phase
   }
}



void fdc_scan()
{
   FDC.led = 1; // turn the drive LED on
   check_unit(); // switch to target drive
   if (init_status_regs() == 0) { // drive Ready?
      active_drive->current_side = (FDC.command[CMD_UNIT] & 4) >> 2; // extract target side
      dword side = active_drive->sides ? active_drive->current_side : 0; // single sided drives only acccess side 1
      if ((active_drive->flipped)) { // did the user request to access the "other" side?
         side = side ? 0 : 1; // reverse the side to access
      }
      active_track = &active_drive->track[active_drive->current_track][side];
      if (active_track->sectors != 0) { // track is formatted?
         if (FDC.command[CMD_STP] > 2) {
            FDC.command[CMD_STP] = 2; // step can only be 1 or 2
         }
         FDC.flags |= SCAN_flag; // scan command active
         cmd_scan();
      }
      else { // unformatted track
         FDC.result[RES_ST0] |= 0x40; // AT
         FDC.result[RES_ST1] |= 0x01; // Missing AM

         LOAD_RESULT_WITH_CHRN

         FDC.phase = RESULT_PHASE; // switch to result phase
      }
   }
   else { // drive was not ready
      LOAD_RESULT_WITH_CHRN

      FDC.phase = RESULT_PHASE; // switch to result phase
   }
}









void fdc_init(int a, int b)
{
 memset(&driveA, 0, sizeof(struct t_drive)); // clear disk drive A data structure
 memset(&driveB, 0, sizeof(struct t_drive)); // clear disk drive B data structure

 memset(&FDC, 0, sizeof(t_FDC));        // clear FDC data structure

 FDC.motor = 0;
 FDC.phase = CMD_PHASE;
 FDC.flags = STATUSDRVA_flag; // | STATUSDRVB_flag;


 active_drive = &driveA; // reference to the currently selected drive
 active_track = NULL; // reference to the currently selected track, of the active_drive
}

void fdc_motor(unsigned char on)
{
 FDC.motor=on;
}
