// warning: the format spec is yet subject to change. wip.

// ## snapshot spec v0: big endian (readable when hexdumped)
// [optional .png screenshot, variable size]
// header [16] + version [16]
// @todo: uncompressed SCR page, then get this page excluded in 16ram pages block
// z80 regs (in alphabetical order) [16 each]
//  AF AF2 BC BC2 DE DE2 HL HL2 IFF1 IFF2 IM IR IX IY PC SP WZ
// ports (in ascending order, ideally)
//  addr [16], data [16]
//  [...]
//  null terminator [16]
// 04 rom pages in ascending order
//  size [16], blob [N] (@todo: z80rle if size < 16k; not paged in if size == 0, thus blob is skipped)
// 16 ram pages in ascending order
//  size [16], blob [N] (@todo: z80rle if size < 16k; not paged in if size == 0, thus blob is skipped)
// num attached medias (tap, tzx, dsk, ...) [16]
//  pages [16], bytes in last page [16], seek percent [16]
//  [...]
// attached blobs for medias
//  blob
//  [...]
// @todo: checksum [16] (checksum of all the previous blocks found. initial hash = snap version that your emu supports)

// @todo: save/load to mem
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
        putnn( ( temp = (value), temp = bswap16(temp), &temp ), 2 )

    put16(STATE_HEADER);
    put16(STATE_VERSION);

    // model and submodel flags
    put16(ZX);
    put16( (!!(rom_patches&TURBO_PATCH)) << 15 | (!!ZX_PENTAGON) << 1 | issue2 );

    put16(AF(cpu));
    put16(AF2(cpu));
    put16(BC(cpu));
    put16(BC2(cpu));
    put16(DE(cpu));
    put16(DE2(cpu));
    put16(HL(cpu));
    put16(HL2(cpu));
    put16(IFF1(cpu));
    put16(IFF2(cpu));
    put16(IM(cpu));
    put16(IR(cpu));
    put16(IX(cpu));
    put16(IY(cpu));
    put16(PC(cpu));
    put16(SP(cpu));
    put16(WZ(cpu));

                    put16(0x00fe), put16(ZXBorderColor); // any ZX
    if( ZX >= 210 ) put16(0x1ffd), put16(page2a); // +2A, +3
    if( ZX >= 128 ) put16(0x7ffd), put16(page128); // any 128
    if( ZX >= 128 ) for( int i = 0; i < 16; ++i ) { // any 128
                        int reg = ( ay_current_reg + i + 1 ) & 15;
                        put16(0xfffd), put16(reg);
                        put16(0xbffd), put16(ay_registers[reg]);
                    }

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

    if( errors ) alert("export errors");
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

    get16(ZX);
    boot(ZX,0);

    // @fixme: we should put boolean flags here like: has_snow, has_contended, has_beta128, has_timings, etc
    uint16_t submodel;
    get16(submodel);
    rom_patches |= !!(submodel&0x8000) * TURBO_PATCH;
    ZX_PENTAGON = !!(submodel & 2);
    issue2 = !!(submodel & 1);

    get16(AF(cpu));
    get16(AF2(cpu));
    get16(BC(cpu));
    get16(BC2(cpu));
    get16(DE(cpu));
    get16(DE2(cpu));
    get16(HL(cpu));
    get16(HL2(cpu));
    get16(IFF1(cpu));
    get16(IFF2(cpu));
    get16(IM(cpu));
    get16(IR(cpu));
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
            tape_seekf(media[i].pos); // @fixme: dsk side/sector case. needed?
    }

    puts(regs("import_state"));

    if( errors ) alert(".sav import errors");
    return errors ? 0 : count;
}
