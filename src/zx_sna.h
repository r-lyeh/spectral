static void *p = 0;
#define mread8()  (*src++)
#define mread16() (p = realloc(p, 2), 0[(byte*)p]=*src++, 1[(byte*)p]=*src++, *((unsigned short*)p))
#define mreadnum(n) (memcpy(p = realloc(p, (n)), src, (n)), src += (n), p)
//#define fread8()  (v = fgetc(fp), v)
//#define fread16() (v = fgetc(fp), v |= fgetc(fp) << 8, v)
//#define freadnum(n) (fread(p = realloc(p, (n)), 1, (n), fp), p)

int rom_load(const byte *src, int len) { // interface2 cartridge
    if(!( src && (len == 16384 || len == 32768 || len == 65536) )) 
        return 0;

    boot(len > 32768 ? 210 : len > 16384 ? 128 : 48, 0); // @fixme: needed?
    /*
    page128 &= ~32;
    port_0x7ffd(32|16);
    */
    ZXBorderColor = 0;
    PC(cpu) = 0;
    memcpy(rom, src, len);
    pins = z80_prefetch(&cpu, cpu.pc);
    return 1;
}

int scr_load(const byte *src, int len) { // screenshot
    if(len != 6912) return 0; // @fixme: 6912+64 for ulaplus+ screen$
#if 0
    //boot(48, 0);
#else
    //boot(48, 0);
    /*
    page128 &= ~32;
    port_0x7ffd(32|16);
    */
    ZXBorderColor = 0;
    PC(cpu) = 0;
#endif
    memcpy(rom, "\xF3\x00\x00\x76\x00\x00\x00", 7); // di, nop2, halt, nop3
    memcpy(VRAM, src, 6912);

    pins = z80_prefetch(&cpu, cpu.pc);
    return 1;
}

int sna_load(const byte *src, int size) {
    /*
        SNA128
        
        Offset   Size   Description
        ------------------------------------------------------------------------
        0        27     bytes  SNA header (see above)
        27       16Kb   bytes  RAM bank 5 \
        16411    16Kb   bytes  RAM bank 2  } - as standard 48Kb SNA file
        32795    16Kb   bytes  RAM bank n / (currently paged bank)
        49179    2      word   PC
        49181    1      byte   port 0x7ffd setting
        49182    1      byte   TR-DOS rom paged (1) or not (0)
        49183    16Kb   bytes  remaining RAM banks in ascending order
          ...
        ------------------------------------------------------------------------
        Total: 131103 or 147487 bytes
    */
    if( size != 49179 && size != 131103 && size != 147487 ) return 0;

//boot(size == 49179 ? 48 : 128, 0);
    page128 = 0; port_0x7ffd(32|16);

    I(cpu) = mread8();
    HL2(cpu) = mread16();
    DE2(cpu) = mread16();
    BC2(cpu) = mread16();
    AF2(cpu) = mread16();
    HL(cpu) = mread16();
    DE(cpu) = mread16();
    BC(cpu) = mread16();
    IY(cpu) = mread16();
    IX(cpu) = mread16();

    int iff2 = mread8();
    IFF1(cpu) =
    IFF2(cpu) = !!(iff2 & 0x04);

    R(cpu) = mread8();
    AF(cpu) = mread16();
    SP(cpu) = mread16();
    IM(cpu) = mread8() & 3; if(IM(cpu) == 3) IM(cpu) = 2;
    ZXBorderColor = mread8(); // & 7;

    memcpy(RAM_BANK(5), mreadnum(16384), 16384);
    memcpy(RAM_BANK(2), mreadnum(16384), 16384);
    memcpy(RAM_BANK(0), mreadnum(16384), 16384);

    // 128k
    if( size != 49179 ) {
        unsigned short pc = mread16();
        byte page = mread8();
        byte trdos128 = !!mread8();

        PC(cpu) = pc;
        page128 = 0; port_0x7ffd(page); page &= 7;

        if( page ) {
            memcpy(RAM_BANK(page), RAM_BANK(0), 16384);
        }
        for( int i = 0; i < 8; ++i ) {  // pentagon has up to 16 pages
            if( i == 2 || i == 5 || i == page ) continue;
            memcpy(RAM_BANK(i), mreadnum(16384), 16384);
        }

        if( trdos128 ) MEMr[0] = rom + 0x4000 * 2;

#if 0
        // amend some games that rely on AY, which cannot be not saved in .sna format
        // see: tai-pan 128k (title screen)
        if(1) ay_registers[7] = 0x00b8;
        if(1) ay_registers[15] = 0x00e5;
#endif
    }
    else {
        // Rui Ribeiro's fix as seen in CSS FAQ
        unsigned short sp = SP(cpu);
        unsigned short pc = READ16(sp);
        PC(cpu) = pc;

        // if( sp < 0x4000 || sp == 0xffff ) return 0;
        if(sp >= 0x4000) WRITE16(sp, 0);
        if(sp <= 0xfffd) SP(cpu) = sp+2;
    }

#if !NEWCORE
    EI(cpu) = IFF1(cpu);
#endif

#if 0 // def NEWCORE
    AF(cpu) = bswap16(AF(cpu));
    AF2(cpu) = bswap16(AF2(cpu));
#endif

    outport(0xFE, ZXBorderColor);

#if DEV
//    puts(regs("sna_load"));
#endif

    pins = z80_prefetch(&cpu, cpu.pc);
    return 1;
}

enum { Z80V1_RAW, Z80V1_RLE, Z80V2_RAW = 3, Z80V2_RLE };

void z80_unrle(int type, byte *dest, const byte* src, int len) {
    int limit;

    if(type == Z80V1_RAW || type == Z80V2_RAW) {
        memcpy(dest, src, type == Z80V1_RAW ? 0xc000 : 0x4000);
        return;
    }
    else if(type == Z80V1_RLE) {
        memset(dest,0,0xc000);
        limit = 0 + 0xc000;
    }
    else { // Z80V2_RLE
        memset(dest,0,0x4000);
        limit = 0 + len;
    }

    for(int pc=0, count=0;;) {
        byte c1=src[count++];

        if( c1 != 237 ) dest[pc++]=c1;   // if not ED...
        else {
            byte c2=src[count++];
            if( c2 != 237 ) dest[pc++]=c1,dest[pc++]=c2;  // if not ED ED...
            else {
                byte rep=src[count++];
                byte val=src[count++];
                memset(dest+pc, val, rep); pc += rep;
            }
        }

        if (type == Z80V1_RLE)    if(pc>=limit) return;
        if (type == Z80V2_RLE) if(count>=limit) return;
    }
}
int z80_guess(const byte *source_, int len) {
    byte buffer[86+10], ver = 0;
    memcpy(buffer, source_, 87);

    if( buffer[12]==255 ) buffer[12]=1; /*as told in CSS FAQ / .z80 section */

    unsigned pc = buffer[7]<<8|buffer[6];
    if(! pc ) {
        pc = buffer[33]<<8|buffer[32];
        if( !strchr("\x17\x36\x37", buffer[30]) )
            alert(va(".z80 unknown version: %d\n", buffer[30]));
        ver = buffer[30] == 0x17 ? 2 : 3;
    }

    if( ver < 2 ) {
        return 48;
    }

    // common extended values (v2+v3)
    /**/ if(buffer[34]== 7) return buffer[37]&0x80 ? 210 : 300; //OK!
    else if(buffer[34]== 8) return buffer[37]&0x80 ? 210 : 300; //OK!
    else if(buffer[34]== 9) return -128; // Pentagon 128k
    else if(buffer[34]==10) return alert(va(".z80 submodel not supported: %d", buffer[34])), 0; // Scorpion 256k
    else if(buffer[34]==11) return alert(va(".z80 submodel not supported: %d", buffer[34])), 0; // Didaktik-Kompakt
    else if(buffer[34]==12) return 200; //OK!
    else if(buffer[34]==13) return 210; //OK!
    else if(buffer[34]==14) return alert(va(".z80 submodel not supported: %d", buffer[34])), 0; // TC2048
    else if(buffer[34]==15) return alert(va(".z80 submodel not supported: %d", buffer[34])), 0; // TC2068
    else if(buffer[34]==16) return alert(va(".z80 submodel not supported: %d", buffer[34])), 0; // TS2068
    else {
    // v2 hw, v3 hw or Unknown hardware
    /**/ if(buffer[34]== 0 && ver>=2) return buffer[37]&0x80 ? 16 : 48; //OK!
    else if(buffer[34]== 1 && ver>=2) return buffer[37]&0x80 ? 16 : 48; //MISSING 48+IF1
    else if(buffer[34]== 2 && ver>=2) return buffer[37]&0x80 ? 16 : 48; //MISSING 48+SAMRAM
    else if(buffer[34]== 3 && ver>=2) return buffer[37]&0x80 ?200 :128; //OK!
    else if(buffer[34]== 4 && ver>=2) return buffer[37]&0x80 ?200 :128; //MISSING 128+IF1:
    else if(buffer[34]== 0 && ver!=2) return buffer[37]&0x80 ? 16 : 48; //OK!
    else if(buffer[34]== 1 && ver!=2) return buffer[37]&0x80 ? 16 : 48; //MISSING 16+IF1 & 48+IF1:
    else if(buffer[34]== 2 && ver!=2) return buffer[37]&0x80 ? 16 : 48; //MISSING 16+SAMRAM & 48+SAMRAM
    else if(buffer[34]== 3 && ver!=2) return buffer[37]&0x80 ? 16 : 48; //MISSING 16+MGT & 48+MGT:
    else if(buffer[34]== 4 && ver!=2) return buffer[37]&0x80 ?200 :128; //OK!
    else if(buffer[34]== 5 && ver!=2) return buffer[37]&0x80 ?200 :128; //MISSING +2+IF1 & 128+IF1 :
    else if(buffer[34]== 6 && ver!=2) return buffer[37]&0x80 ?200 :128; //MISSING 128 + MGT:
    else                              return 48;
    }
}
int z80_load(const byte *source_, int len) {
    int f, tam, sig;
    byte buffer[86+10], pag, ver = 0, ver_rle = 0;
    const byte *source = source_;
    const byte *end = source_ + len;

    memcpy(buffer, source, 87); // was, 86 before

    if( buffer[12]==255 ) buffer[12]=1; /*as told in CSS FAQ / .z80 section */

    // Check file version
    unsigned pc = buffer[7]<<8|buffer[6];
    if( !pc ) {
        pc = buffer[33]<<8|buffer[32];
        ver = buffer[30] == 23 ? 2 : 3;
    }

    // Config pages
    if( ver < 2 ) {
        // .z80 v1.45 or earlier
        source=source_; source+=30;

        // 48K only
        char *pages3 = malloc(0x4000*3);
        z80_unrle((buffer[12] & 0x20 ? Z80V1_RLE : Z80V1_RAW), pages3, source, 0);
        memcpy(RAM_BANK(5), pages3+0x4000*0, 0x4000);
        memcpy(RAM_BANK(2), pages3+0x4000*1, 0x4000);
        memcpy(RAM_BANK(0), pages3+0x4000*2, 0x4000);
        free(pages3);
    } else {

        // common extended values (v2+v3)
        if(ZX>=128) {
            //  if the word [30] is 23:
            for(int psg=0;psg<16;psg++) { port_0xfffd(psg); port_0xbffd(buffer[39+psg]); }
            port_0xfffd(buffer[38]);

            if(buffer[30] == 55) port_0x1ffd(buffer[86]);

            port_0x7ffd(buffer[35]);
        }

        sig = 30 + 2 + buffer[30]; // [30] should be 16bits

        for (f = 0; f < 16 ; f++) { //up 16 pages (ZS Scorpion)
            byte *target=NULL;

            source=source_; source+=sig;

            tam = *source++; 
            tam = tam + ((*source++) << 8);
            pag = *source++; 
            sig += 3 + tam;
if (tam==65535) sig-=49151;

            if(ZX<128)
            switch(pag) {
             case 1:  break; //Interface I/disciple/plus D rom
             case 11: break; //multiface rom

             //case 0:  target=ROM_BANK(0); break;
             case 8:  target=RAM_BANK(5); break;
             case 4:  target=RAM_BANK(2); break; // (2)?
             case 5:  target=RAM_BANK(0); break; // (3)?
             default: break;
            }
            else
            switch(pag) {
             case 1:  break; //Interface I/disciple/plus D rom
             case 11: break; //multiface rom

             //case 0:  target=ROM_BANK(1); break;
             //case 2:  target=ROM_BANK(0); break;
             default: if((pag>=3) && (pag<=10)) // [3..18] pages for scorpion
                                target=RAM_BANK(pag-3); break;
            }

            if(target!=NULL)
             z80_unrle(ver_rle = (tam == 0xffff ? Z80V2_RAW : Z80V2_RLE), target, source, tam);

            if(source_+sig>=end) break;
        }
    }

    PC(cpu) = pc;

    AF(cpu) = buffer[1]<<8|buffer[0];
    BC(cpu) = buffer[3]<<8|buffer[2];
    HL(cpu) = buffer[5]<<8|buffer[4];

    SP(cpu) = buffer[9]<<8|buffer[8];
    I(cpu) = buffer[10];
    R(cpu) = (buffer[11] & 0x7F) | ((buffer[12] & 1) << 7);
    ZXBorderColor = (buffer[12] >> 1); // & 7;

    DE(cpu) = buffer[14]<<8|buffer[13];
    BC2(cpu) = buffer[16]<<8|buffer[15];
    DE2(cpu) = buffer[18]<<8|buffer[17];
    HL2(cpu) = buffer[20]<<8|buffer[19];
    AF2(cpu) = buffer[22]<<8|buffer[21];
    IY(cpu) = buffer[24]<<8|buffer[23];
    IX(cpu) = buffer[26]<<8|buffer[25];

#if NEWCORE
    IFF1(cpu) = !!buffer[27];
#else
    EI(cpu) = !!buffer[27];
    IFF1(cpu) = 
#endif
    IFF2(cpu) = !!buffer[28];

    IM(cpu) = buffer[29] == 0xFF ? 1 : buffer[29] & 0x03;
    if(IM(cpu) == 3) IM(cpu) = 2;

    outport(0xFE, ZXBorderColor);

#if 1 // def NEWCORE
    AF(cpu) = bswap16(AF(cpu));
    AF2(cpu) = bswap16(AF2(cpu));
#endif

    printf("z80 v%d (rle:%d) (machine:%d)\n", ver, ver_rle, buffer[34]);

#if DEV
//    puts(regs("z80_load"));
#endif

    pins = z80_prefetch(&cpu, cpu.pc);
    return 1;
}

int pok_load(const byte *src, int len) {
    /*
    NInfinite lives
    M  8 41047   0  53
    Z  8 41046 195  91
    NSet number of lives
    Z  8 23343 256   0
    Y

    lbbb aaaaa vvv ooo
    Where l determines the content, bbb is the bank, aaaaa is the address to be poked with value vvv and ooo is the original 
    value of aaaaa. All values are decimal, and separated by one or more spaces, apart from between l and bbb; however, 
    the bank value is never larger than 8, so you will always see 2 spaces in front of the bank. The field bank field is
    built from;

    bit 0-2 : bank value
    bit 3 : ignore bank (1=yes, always set for 48K games)

    If the field [value] is in range 0-255, this is the value to be POKEd. If it is 256, a requester should pop up where
    the user can enter a value.
    */
    if( *src != 'N' ) return 0; // not a .pok file

    char trainer[256] = {0};
    char line[256] = {0};
    while( strchr("NMZY", *src) && sscanf(src, "%[^\r\n]", line) > 0 ) {
        src += strlen(line);
        while( strchr("\r\n", *src) ) ++src;

        if( line[0] == 'N' ) { if(trainer[0]) alert(trainer); strcpy(trainer, line+1); continue; }
        if( line[0] == 'Y' ) { if(trainer[0]) alert(trainer); return 1; }

        int bank, addr, val, defaults;
        if( 4 != sscanf(line+1, "%d %d %d %d", &bank, &addr, &val, &defaults) ) break;

        // int current = bank & 8 ? READ8(addr) : RAM_BANK(bank&7)[addr & 0x3FFF];
        // if( current != defaults ) { alert("poke mismatch"); continue; }

        extern Tigr *app;
        if( val == 256 ) val = (byte)atoi(prompt(trainer, "", va("%d",defaults))), printf("prompt: %d\n", val), trainer[0] = '\0';

        if( bank & 8 )
        WRITE8(addr, val);
        else
        RAM_BANK(bank&7)[addr & 0x3FFF] = val;
    }

    puts(".pok error");
    return 0;
}

int guess(const byte *ptr, int size) { // guess required model type for given data
    // ay first
    if( size > 0x08 &&(!memcmp(ptr, "ZXAYEMUL", 8) )) return 128;

    // dsk first
    if( size > 0x08 &&(!memcmp(ptr, "MV - CPC", 8) || !memcmp(ptr, "EXTENDED", 8)) ) return 300;

    // tapes first
    if( size > 0x08 && !memcmp(ptr, "ZXTape!\x1a", 8) ) return ZX;
    if( size > 0x02 && !memcmp(ptr, "\x13\x00", 2) ) return ZX;
    if( size > 0x17 && !memcmp(ptr, "Compressed Square Wave\x1a", 0x17) ) return ZX;

    // headerless fixed-size formats now, sorted by ascending file size.
    if( size == 6912 ) return 48;
    if( size == 16384 ) return 48;
    if( size == 32768 ) return 128;
    if( size == 65536 ) return 210;
    if( size == 49179 ) return 48;
    if( size == 131103 ) return 128;
    if( size == 147487 ) return 128;

    // at this point file is too large to be a snapshot. must be a disk instead (trd,scl,fdi,mgt,etc)
    if( size > 147487 ) return -128;

    // headerless variable-size formats now
    if( *ptr == 'N' ) return ZX;
    return size > 87 ? z80_guess(ptr, size) : ZX;
}
