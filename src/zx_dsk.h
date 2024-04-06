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

void dsk_eject (struct t_drive *drive)
{
   dword track, side;

   for (track = 0; track < DSK_TRACKMAX; track++) { // loop for all tracks
      for (side = 0; side < DSK_SIDEMAX; side++) { // loop for all sides
         if (drive->track[track][side].data) { // track is formatted?
            // free(drive->track[track][side].data); // release memory allocated for this track
         }
      }
   }

   dword dwTemp = drive->current_track; // save the drive head position
   memset(drive, 0, sizeof(struct t_drive)); // clear drive info structure
   drive->current_track = dwTemp;
}


//int dsk_load (char *pchFileName, struct t_drive *drive, char chID)
int dsk_load(void *pchFileName)
{
   int iRetCode = 0;
   dword dwTrackSize, track, side, sector, dwSectorSize, dwSectors;
   byte *pbPtr, *pbDataPtr, *pbTempPtr, *pbTrackSizeTable;
   struct t_drive *drive=&driveA;

// dsk_eject(drive);
   {
   dword dwTemp = drive->current_track; // save the drive head position
   memset(drive, 0, sizeof(struct t_drive));   // clear drive info structure
   drive->current_track = dwTemp;
   }

//   if ((pfileObject = fopen(pchFileName, "rb")) != NULL) {
//    fread(pbGPBuffer, 0x100, 1, pfileObject); // read DSK header
      pbGPBuffer = (byte *)pchFileName;

      pbPtr = pbGPBuffer;

      printf("dsk type: %.*s\n", 8, pbPtr);

      if (memcmp(pbPtr, "MV - CPC", 8) == 0) { // normal DSK image?
         drive->tracks = *(pbPtr + 0x30); // grab number of tracks
         if (drive->tracks > DSK_TRACKMAX) { // compare against upper limit
            drive->tracks = DSK_TRACKMAX; // limit to maximum
         }
         drive->sides = *(pbPtr + 0x31); // grab number of sides
         if (drive->sides > DSK_SIDEMAX) { // abort if more than maximum
            iRetCode = ERR_DSK_SIDES;
            goto exit;
         }
         dwTrackSize = (*(pbPtr + 0x32) + (*(pbPtr + 0x33) << 8)) - 0x100; // determine track size in bytes, minus track header
         drive->sides--; // zero base number of sides
         for (track = 0; track < drive->tracks; track++) { // loop for all tracks
            for (side = 0; side <= drive->sides; side++) { // loop for all sides
               //fread(pbGPBuffer+0x100, 0x100, 1, pfileObject); // read track header
               //pbPtr = pbGPBuffer + 0x100;

                 pbGPBuffer += 0x100;
                 pbPtr = pbGPBuffer ;

               if (memcmp(pbPtr, "Track-Info", 10) != 0) { // abort if ID does not match
                  iRetCode = ERR_DSK_INVALID;
                  goto exit;
               }
               dwSectorSize = 0x80 << *(pbPtr + 0x14); // determine sector size in bytes
               dwSectors = *(pbPtr + 0x15); // grab number of sectors
               if (dwSectors > DSK_SECTORMAX) { // abort if sector count greater than maximum
                  iRetCode = ERR_DSK_SECTORS;
                  goto exit;
               }
               drive->track[track][side].sectors = dwSectors; // store sector count
               drive->track[track][side].size = dwTrackSize; // store track size
               /*
               drive->track[track][side].data = (byte *)malloc(dwTrackSize); // attempt to allocate the required memory
               if (drive->track[track][side].data == NULL) { // abort if not enough
                  iRetCode = ERR_OUT_OF_MEMORY_;
                  goto exit;
               }
               */
               drive->track[track][side].data = pbGPBuffer + 0x100;

               pbDataPtr = drive->track[track][side].data; // pointer to start of memory buffer
               pbTempPtr = pbDataPtr; // keep a pointer to the beginning of the buffer for the current track
               for (sector = 0; sector < dwSectors; sector++) { // loop for all sectors
                  memcpy(drive->track[track][side].sector[sector].CHRN, (pbPtr + 0x18), 4); // copy CHRN
                  memcpy(drive->track[track][side].sector[sector].flags, (pbPtr + 0x1c), 2); // copy ST1 & ST2
                  sector_setSizes( &drive->track[track][side].sector[sector], dwSectorSize, dwSectorSize);
                  sector_setData( &drive->track[track][side].sector[sector], pbDataPtr); // store pointer to sector data
                  pbDataPtr += dwSectorSize;
                  pbPtr += 8;
               }
               /*
               if (!fread(pbTempPtr, dwTrackSize, 1, pfileObject)) { // read entire track data in one go
                  iRetCode = ERR_DSK_INVALID;
                  goto exit;
               }
               */
               //memcpy(pbTempPtr, pbGPBuffer + 0x100, dwTrackSize);
               pbGPBuffer += dwTrackSize;
            }
         }
         drive->altered = 0; // disk is as yet unmodified
      } else {
         if (memcmp(pbPtr, "EXTENDED", 8) == 0) { // extended DSK image?
            drive->tracks = *(pbPtr + 0x30); // number of tracks
            if (drive->tracks > DSK_TRACKMAX) {  // limit to maximum possible
               drive->tracks = DSK_TRACKMAX;
            }
            drive->random_DEs = *(pbPtr + 0x31) & 0x80; // simulate random Data Errors?
            drive->sides = *(pbPtr + 0x31) & 3; // number of sides
            if (drive->sides > DSK_SIDEMAX) { // abort if more than maximum
               iRetCode = ERR_DSK_SIDES;
               goto exit;
            }
            pbTrackSizeTable = pbPtr + 0x34; // pointer to track size table in DSK header
            drive->sides--; // zero base number of sides
            for (track = 0; track < drive->tracks; track++) { // loop for all tracks
               for (side = 0; side <= drive->sides; side++, pbGPBuffer += dwTrackSize) { // loop for all sides
                  memset(&drive->track[track][side], 0, sizeof(t_track)); // track not formatted
                  dwTrackSize = (*pbTrackSizeTable++ << 8); // track size in bytes
                  if (dwTrackSize != 0) { // only process if track contains data
                     dwTrackSize -= 0x100; // compensate for track header
                     //fread(pbGPBuffer+0x100, 0x100, 1, pfileObject); // read track header
                     //pbPtr = pbGPBuffer + 0x100;

                       pbGPBuffer += 0x100;
                       pbPtr = pbGPBuffer;

                     if (memcmp(pbPtr, "Track-Info", 10) != 0) { // valid track header?
                        iRetCode = ERR_DSK_INVALID;
                        goto exit;
                     }
                     dwSectors = *(pbPtr + 0x15); // number of sectors for this track
                     if (dwSectors > DSK_SECTORMAX) { // abort if sector count greater than maximum
                        iRetCode = ERR_DSK_SECTORS;
                        goto exit;
                     }
                     drive->track[track][side].sectors = dwSectors; // store sector count
                     drive->track[track][side].size = dwTrackSize; // store track size
                     /*
                     drive->track[track][side].data = (byte *)malloc(dwTrackSize); // attempt to allocate the required memory
                     if (drive->track[track][side].data == NULL) { // abort if not enough
                        iRetCode = ERR_OUT_OF_MEMORY_;
                        goto exit;
                     }
                     */
                     drive->track[track][side].data = pbGPBuffer + 0x100;

                     pbDataPtr = drive->track[track][side].data; // pointer to start of memory buffer
                     pbTempPtr = pbDataPtr; // keep a pointer to the beginning of the buffer for the current track
            pbPtr += 0x18;
                     for (sector = 0; sector < dwSectors; sector++) { // loop for all sectors
                        memcpy(drive->track[track][side].sector[sector].CHRN, pbPtr, 4); // copy CHRN
                        memcpy(drive->track[track][side].sector[sector].flags, (pbPtr + 0x04), 2); // copy ST1 & ST2
                        dword dwRealSize = 0x80 << *(pbPtr+0x03);
                        dwSectorSize = *(pbPtr + 0x6) + (*(pbPtr + 0x7) << 8); // sector size in bytes
                        sector_setSizes( &drive->track[track][side].sector[sector], dwRealSize, dwSectorSize);
                        sector_setData( &drive->track[track][side].sector[sector], pbDataPtr); // store pointer to sector data
                        pbDataPtr += dwSectorSize;
                        pbPtr += 8;
                     }
                     /*
                     if (!fread(pbTempPtr, dwTrackSize, 1, pfileObject)) { // read entire track data in one go
                        iRetCode = ERR_DSK_INVALID;
                        goto exit;
                     }
                     */
                     //memcpy(pbTempPtr, pbGPBuffer + 0x100, dwTrackSize);
                  }
               }
            }
            drive->altered = 0; // disk is as yet unmodified
         } else {
            iRetCode = ERR_DSK_INVALID; // file could not be identified as a valid DSK
         }
      }
/*    {
         char *pchTmpBuffer = new char[MAX_LINE_LEN];
         LoadString(hAppInstance, MSG_DSK_LOAD, chMsgBuffer, sizeof(chMsgBuffer));
         snprintf(pchTmpBuffer, _MAX_PATH-1, chMsgBuffer, chID, chID == 'A' ? CPC.drvA_file : CPC.drvB_file);
         add_message(pchTmpBuffer);
         delete [] pchTmpBuffer;
      } */
exit:;
/*
      fclose(pfileObject);
   } else {
      iRetCode = ERR_FILE_NOT_FOUND;
   }
*/
/*
   if (iRetCode != 0) { // on error, 'eject' disk from drive
      dsk_eject(drive);
   }
   */
   if(iRetCode) puts("invalid disk");
   if(iRetCode) dsk_eject(drive);
   return iRetCode;
}

