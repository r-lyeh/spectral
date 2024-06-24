// ay player: based on AY2SNA (license below)
// - rlyeh, public domain.

// AY to SNA converter.
// (c)2001 Bulba S.V.
// You can use this source as you want without any reference to author.
// Originally compiled in Delphi 5.
// Main procedures are originally used in Micro Speccy and AY-files player.
// Sergey Bulba, vorobey@mail.khstu.ru, http://bulba.at.kz/
//
// - How player of version 3 must play one of songs of AY File
//   a) Fill #0000-#00FF range with #C9 value
//   b) Fill #0100-#3FFF range with #FF value
//   c) Fill #4000-#FFFF range with #00 value
//   d) Place to #0038 address #FB value
//   e) if INIT equal to ZERO then place to first CALL instruction address of
//      first AY file block instead of INIT (see next f) and g) steps)
//   f) if INTERRUPT equal to ZERO then place at ZERO address next player:
//
//       DI
//       CALL    INIT
//       LOOP:   IM      2
//       EI
//       HALT
//       JR      LOOP
//
//   g) if INTERRUPT not equal to ZERO then place at ZERO address next player:
//
//       DI
//       CALL    INIT
//       LOOP:   IM      1
//       EI
//       HALT
//       CALL INTERRUPT
//       JR      LOOP
//
//   h) Load all blocks for this song
//   i) Load all common lower registers with LoReg value (including AF register)
//   j) Load all common higher registers with HiReg value
//   k) Load into I register 3 (this player version)
//   l) load to SP stack value from points data of this song
//   m) Load to PC ZERO value
//   n) Disable Z80 interrupts and set IM0 mode
//   o) Emulate resetting of AY chip
//   p) Start Z80 emulation
//
//   As you can see, blocks of AY files can to rewrite standard player routine
//   with own one. So, if you can not adapt some of tunes to standard player,
//   you can write own player and place it at INIT address or even at 0x0001
//   address (block of AY-file can be loaded at any address except 0x0000).


struct ay_track_tag {
    const byte *namestr,*data;
    const byte *data_stacketc,*data_memblocks;
    int fadestart,fadelen;
};

struct aydata_tag {
    const byte *filedata;
    int filelen;
    struct ay_track_tag *tracks;

    int filever,playerver;
    const byte *authorstr,*miscstr;
    int num_tracks;
    int first_track;
} aydata;

void play_track(unsigned track) {
    #define WORD16(x) ( 0[x] << 8 | 1[x] )

    int init = WORD16(aydata.tracks[track].data_stacketc+2);
    int interrupt = WORD16(aydata.tracks[track].data_stacketc+4);
    int ay_1st_block = WORD16(aydata.tracks[track].data_memblocks);

    // m) Load to PC ZERO value
    // n) Disable Z80 interrupts and set IM0 mode
    // o) Emulate resetting of AY chip
    // p) Start Z80 emulation
    reset(128);
    port_0x00fe(0); // black border
    /*
    page128 &= ~32;
    port_0x7ffd(32|16);
    */

    // a) Fill #0000-#00FF range with #C9 value
    // b) Fill #0100-#3FFF range with #FF value
    // c) Fill #4000-#FFFF range with #00 value
    // d) Place to #0038 address #FB value
    memset(ADDR8(0x0000),0xFF,0x4000);
    memset(ADDR8(0x0000),0xC9,0x0100);
    memset(ADDR8(0x4000),0x00,0x4000);
    memset(ADDR8(0x8000),0x00,0x4000);
    memset(ADDR8(0xC000),0x00,0x4000);
    *ADDR8(0x38) = 0xFB; /* EI */

    // e) if INIT equal to ZERO then place to first CALL instruction address of
    //    first AY file block instead of INIT (see next f) and g) steps)
    init = init ? init : ay_1st_block;

    // f) if INTERRUPT equal to ZERO then place at ZERO address intz player
    // g) if INTERRUPT not equal to ZERO then place at ZERO address intnz player
    if( interrupt ) {
        unsigned char intnz[] = {
            0xf3,           // di
            0xcd,0x00,0x00, // call init
            0xed,0x56,      // loop: im 1
            0xfb,           // ei
            0x76,           // halt
            0xcd,0x00,0x00, // call interrupt
            0x18,0xf7       // jr loop
        };

        // patch call init
        intnz[2] = init % 256;
        intnz[3] = init / 256;

        // patch call interrupt
        intnz[ 9] = interrupt % 256;
        intnz[10] = interrupt / 256;

        memcpy(ADDR8(0),intnz,sizeof(intnz));
    }
    else {
        unsigned char intz[] = {
            0xf3,           // di
            0xcd,0x00,0x00, // call init
            0xed,0x5e,      // loop: im 2
            0xfb,           // ei
            0x76,           // halt
            0x18,0xfa       // jr loop
        };

        // patch call init
        intz[2] = init % 256;
        intz[3] = init / 256;

        memcpy(ADDR8(0),intz,sizeof(intz));
    }

    // h) Load all blocks for this song
    // put the memory blocks in place
    track %= aydata.num_tracks;
    {
        int addr;
        for( const byte *ptr = aydata.tracks[track].data_memblocks; (addr = WORD16(ptr)) != 0; ptr += 6 ) {
            int len = WORD16(ptr+2);
            int ofs = WORD16(ptr+4); if( ofs >= 0x8000) ofs -= 0x10000;

            // range check
            if( ptr-4-aydata.filedata+ofs >= 0 && ptr-4-aydata.filedata+ofs < aydata.filelen) {
                // fix any broken length
                if( ptr+4+ofs+len >= aydata.filedata+aydata.filelen )
                    len=aydata.filedata+aydata.filelen-(ptr+4+ofs);
                if( addr+len > 0x10000 )
                    len=0x10000-addr;

                memcpy(ADDR8(addr),ptr+4+ofs,len);
            }
        }
    }

    // i) Load all common lower registers with LoReg value (including AF register)
    // j) Load all common higher registers with HiReg value
    
    WZ(cpu) = 0;
    IX(cpu) = IY(cpu) =
    BC(cpu) = DE(cpu) = HL(cpu) = AF(cpu) =
    BC2(cpu) = DE2(cpu) = HL2(cpu) = AF2(cpu) =
    *(aydata.tracks[track].data+8) * 256 + *(aydata.tracks[track].data+9);

    // k) Load into I register 3 (this player version)

    I(cpu) = 0; // 3;

    // l) load to SP stack value from points data of this song

    SP(cpu) = WORD16(aydata.tracks[track].data_stacketc+0);

    #undef WORD16
}

int load_ay(const byte *data, int len) {
    #define READ16W(x)   (x)=256*(*ptr++); (x)|=*ptr++
    #define READ16PTR(x) READ16W(tmp); if(tmp>=0x8000) tmp=-0x10000+tmp; if(ptr-data-2+tmp>=len || ptr-data-2+tmp<0)  return(0); (x)=ptr-2+tmp

    if( len < 8 || memcmp(data,"ZXAYEMUL",8) )
        return 0;

    int tmp;

    aydata.filedata = data;
    aydata.filelen = len;

    const byte *ptr=data+8;
    aydata.filever=*ptr++;
    aydata.playerver=*ptr++; // if(aydata.playerver!=3) return alert(va("ay version %d not supported",aydata.playerver)), (0);
    ptr+=2; // skip custom player stuff
    READ16PTR(aydata.authorstr); //puts(aydata.authorstr);
    READ16PTR(aydata.miscstr); //puts(aydata.miscstr);
    aydata.num_tracks=1+*ptr++;
    aydata.first_track=*ptr++;

    // skip to track info
    const byte *track_info; READ16PTR(track_info);
    ptr = track_info;

    // alloc
    aydata.tracks = realloc(aydata.tracks, aydata.num_tracks*sizeof(struct ay_track_tag));
    if( !aydata.tracks ) return 0;

    // read tracks
    for( int i = 0; i < aydata.num_tracks; ++i ) {
        READ16PTR(aydata.tracks[i].namestr); //puts(aydata.tracks[i].namestr);
        READ16PTR(aydata.tracks[i].data);
    }

    // decode tracks
    for( int i = 0; i < aydata.num_tracks; ++i ) {
        if( aydata.tracks[i].data-data+10>len-4 ) {
            return 0;
        }

        ptr = aydata.tracks[i].data+10;
        READ16PTR(aydata.tracks[i].data_stacketc);
        READ16PTR(aydata.tracks[i].data_memblocks);

        ptr = aydata.tracks[i].data+4;
        READ16W(aydata.tracks[i].fadestart);
        READ16W(aydata.tracks[i].fadelen);
    }

    // no more parsing here
    play_track(aydata.first_track);
    return 1;

    #undef READ16W
    #undef READ16PTR
}
