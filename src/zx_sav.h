// ## snapshot spec v1: big endian (readable when hexdumped)
// [optional .png screenshot, variable size]
// header [16] + version [16]
// z80 regs (in alphabetical order) [16 each]
//  AF AF2 BC BC2 DE DE2 HL HL2 IFF12 IM IR IX IY PC SP WZ
// num ports (in ascending order) [16]
//  addr [16], data [16]
// num rom sequential pages (in ascending order) [4]
//  size [16], blob [N] (@todo: z80rle if size < 16k; not paged in if size == 0, thus blob is skipped)
// num ram sequential pages (in ascending order) [16]
//  size [16], blob [N] (@todo: z80rle if size < 16k; not paged in if size == 0, thus blob is skipped)
// num attached medias (tap, tzx, dsk, ...) [16]
//  pages [16], size [16], seek [16]
// blob[0]
// blob[1]
// ..
// blob[16]

// @todo: save/load to mem
// @todo: checksum at eof
// @todo: png header,
//        then make screenshot() to write a snapshots too,
//        then allow .pngs to be dropped,
//        and load them
//        Length:32  ChunkType:32  ChunkData:32  CRC:32 (computed over the chunk type and chunk data)
//        z - not critical
//        x - not standardised
//        s - unrecognised chunk
//        v - may be safely copied regardless of the extent of modifications to the file

const uint16_t STATE_HEADER = 'Xp';
const uint16_t STATE_VERSION = '0\x1a';

int export_state(FILE *fp) {
    if( !fp ) return 0;

    puts(regs("export_state"));

    uint32_t count = 0, errors = 0, temp;

    #define putnn(ptr,len) \
        ( errors += fwrite( (count += (len), (ptr)), (len), 1, fp ) != 1 )

    #define put16(value) \
        putnn( ( temp = bswap16(value), &temp ), 2 )

    put16(STATE_HEADER);
    put16(STATE_VERSION);

    put16(ZX|!!(rom_patches&TURBO_PATCH)); // this flag shouldnt be here

    put16(AF(cpu));
    put16(AF2(cpu));
    put16(BC(cpu));
    put16(BC2(cpu));
    put16(DE(cpu));
    put16(DE2(cpu));
    put16(HL(cpu));
    put16(HL2(cpu));       uint16_t cpu_iff12 = (IFF1(cpu) << 8) | (IFF2(cpu) & 255);
    put16(cpu_iff12);
    put16(IM(cpu));        uint16_t cpu_ir = (I(cpu) << 8) | (R(cpu) & 255);
    put16(cpu_ir);
    put16(IX(cpu));
    put16(IY(cpu));
    put16(PC(cpu));
    put16(SP(cpu));
    put16(WZ(cpu));

    if( ZX >= 210 ) put16(0x1ffd), put16(page2a); // +2A, +3
    if( ZX >= 128 ) put16(0x7ffd), put16(page128); // any 128
    if( ZX >= 128 ) for( int i = 0; i < 16; ++i ) { // any 128
                        int reg = ( ay_current_reg + i + 1 ) & 15;
                        put16(0xfffd), put16(reg);
                        put16(0xbffd), put16(ay_registers[reg]);
                    }
                    put16(0x00fe), put16(ZXBorderColor); // any ZX

    if( ZX_ULAPLUS && ulaplus_enabled ) {
        put16(0xBF3B), put16( 64 ); // mode group
        put16(0xFF3B), put16(  1 ); // turn palette mode on
        for( int i = 0; i < 64; ++i ) { // dump palette
            put16(0xBF3B), put16( i ),
            put16(0xFF3B), put16( ulaplus_registers[i] );
        }
    }

    put16(0x0000); // terminator

    for( int i = 0; i < 4; ++i ) {
        put16(0x4000);
        putnn(ROM_BANK(i), 0x4000);
    }

    for( int i = 0; i < 16; ++i ) {
        put16(0x4000);
        putnn(RAM_BANK(i), 0x4000);
    }

    put16(medias);
    for( int i = 0; i < medias; ++i ) {
        printf("saving media [%d] @%f %u\n", i, media[i].pos, media[i].len);
        uint16_t pages = media[i].len / 65536;
        uint16_t bytes = media[i].len % 65536;
        uint16_t seekp = (uint16_t)(media[i].pos * 65335.0);
        put16(pages);
        put16(bytes);
        put16(seekp);
    }
    for( int i = 0; i < medias; ++i ) {
        putnn(media[i].bin, media[i].len);
    }

    if( errors ) warning("export errors");
    return errors ? 0 : count;
}

int import_state(FILE *fp) {
    if( !fp ) return 0;

    uint16_t count = 0, errors = 0, temp;

    #define getnn(ptr,len) \
        ( errors += fread( (count += (len), (ptr)), (len), 1, fp ) != 1 )

    #define get16(value) \
        (getnn(&temp, 2), value = bswap16(temp))

    #define check16(value) \
        if( (get16(temp), temp) != value ) return 0

    check16(STATE_HEADER);
    check16(STATE_VERSION);

    get16(ZX); int turbo = ZX&1;
    boot(ZX&0xFFFE,0);
    rom_patches |= turbo * TURBO_PATCH;

    get16(AF(cpu));
    get16(AF2(cpu));
    get16(BC(cpu));
    get16(BC2(cpu));
    get16(DE(cpu));
    get16(DE2(cpu));
    get16(HL(cpu));
    get16(HL2(cpu));    uint16_t cpu_iff12;
    get16(cpu_iff12);   IFF1(cpu)=cpu_iff12>>8; IFF2(cpu)=cpu_iff12&255;
    get16(IM(cpu));     uint16_t cpu_ir;
    get16(cpu_ir);      I(cpu)=cpu_ir>>8; R(cpu)=cpu_ir&255;
    get16(IX(cpu));
    get16(IY(cpu));
    get16(PC(cpu));
    get16(SP(cpu));
    get16(WZ(cpu));

    for(;;) {
        uint16_t port, data;
        get16(port); if(!port) break; get16(data);

        /**/ if( port == 0x00fe ) outport(port, ZXBorderColor = data );
        else if( port == 0x7ffd ) outport(port, page128 = data );
        else if( port == 0x1ffd ) outport(port, page2a = data );
        else                      outport(port, data);
    }

    for( int i = 0; i < 4; ++i ) {
        uint16_t banklen;
        get16(banklen);

        getnn(ROM_BANK(i), banklen);
    }

    // ZX_ALTROMS = ZX > 200 ? 0 : !!memcmp(ROM_BANK(0), ZX < 128 ? rom48 : rom128, 0x4000);

    for( int i = 0; i < 16; ++i ) {
        uint16_t banklen;
        get16(banklen);

        getnn(RAM_BANK(i), banklen);
    }

    get16(medias);
    for( int i = 0; i < medias; ++i ) {
        uint16_t pages; get16(pages);
        uint16_t bytes; get16(bytes);
        uint16_t seekp; get16(seekp);

        media[i].len = pages * 65536 + bytes;
        media[i].bin = realloc(media[i].bin, media[i].len);
        media[i].pos = seekp / 65535.0;

        printf("loading media [%d] @%f %u\n", i, media[i].pos, media[i].len);
    }
    for( int i = 0; i < medias; ++i ) {
        getnn(media[i].bin, media[i].len);
        if(loadbin_(media[i].bin, media[i].len, 0))
            mic_seekf(media[i].pos); // @fixme: dsk side/sector case
    }

    puts(regs("import_state"));

    if( errors ) warning(".sav import errors");
    return errors ? 0 : count;
}
