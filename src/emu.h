//-----------------------------------------------------------------------------
// 3rd party emu libs

int play(int sample, unsigned count); // this is from sys headers actually

#define Read1793(...)  (play('read',  1), Read1793(__VA_ARGS__))
#define SeekFDI(...)   (play('seek',  1), SeekFDI(__VA_ARGS__))
#define Write1793(...) (play('moto', 10), Write1793(__VA_ARGS__))

#define fdc_read()     (play('read',  1), fdc_read())
#define fdc_seek()     (play('seek',  1), fdc_seek())
#define fdc_motor(on)  (play('moto', on ? ~0u : 0), fdc_motor(on))

#define CHIPS_IMPL
#define CHIPS_UTIL_IMPL
#include "emu_spk.h"
#include "emu_ay.h"
#include "emu_ayumi.h"
#include "emu_fdc.h"
#include "emu_wd1793.h"

#if! REDCODE

#ifndef OLDCORE
#define NEWCORE 1
#endif

#if NEWCORE
#include "emu_z80.h"
#define EI IFF1
#else
#include "emu_z80old.h"
#define EI(cpu)    (cpu).ei
#endif

#include "emu_z80dasm.h"

#define AF(cpu)    (cpu).af
#define BC(cpu)    (cpu).bc
#define DE(cpu)    (cpu).de
#define HL(cpu)    (cpu).hl
#define AF2(cpu)   (cpu).af2
#define BC2(cpu)   (cpu).bc2
#define DE2(cpu)   (cpu).de2
#define HL2(cpu)   (cpu).hl2
#define IX(cpu)    (cpu).ix
#define IY(cpu)    (cpu).iy
#define PC(cpu)    (cpu).pc
#define SP(cpu)    (cpu).sp
#define I(cpu)     (cpu).i
#define R(cpu)     (cpu).r
#define IR(cpu)    (cpu).ir
#define IM(cpu)    (cpu).im
#define IFF1(cpu)  (cpu).iff1
#define IFF2(cpu)  (cpu).iff2
#define WZ(cpu)    (cpu).wz

#define z80_interrupt(x,on) do { \
    if(on) pins |= Z80_INT; \
    else pins &= ~Z80_INT; \
} while(0)

#else

#pragma push_macro("IN")
#pragma push_macro("OUT")
#undef IN
#undef OUT
#include "emu_z80red.h"
#pragma pop_macro("OUT")
#pragma pop_macro("IN")

#undef  AF
#define AF(cpu)    Z80_AF(cpu)
#undef  BC
#define BC(cpu)    Z80_BC(cpu)
#undef  DE
#define DE(cpu)    Z80_DE(cpu)
#undef  HL
#define HL(cpu)    Z80_HL(cpu)
#undef  AF2
#define AF2(cpu)   Z80_AF_(cpu)
#undef  BC2
#define BC2(cpu)   Z80_BC_(cpu)
#undef  DE2
#define DE2(cpu)   Z80_DE_(cpu)
#undef  HL2
#define HL2(cpu)   Z80_HL_(cpu)
#undef  IX
#define IX(cpu)    Z80_IX(cpu)
#undef  IY
#define IY(cpu)    Z80_IY(cpu)
#undef  PC
#define PC(cpu)    Z80_PC(cpu)
#undef  SP
#define SP(cpu)    Z80_SP(cpu)
#undef  I
#define I(cpu)     Z80_I(cpu)
#undef  R
#define R(cpu)     Z80_R(cpu)
#undef  IM
#define IM(cpu)    Z80_IM(cpu)
#undef  IFF1
#define IFF1(cpu)  Z80_IFF1(cpu)
#undef  IFF2
#define IFF2(cpu)  Z80_IFF2(cpu)

#define Z80_I(cpu) (cpu).i
#define Z80_R(cpu) (cpu).r // warning: missing r7 here
#define Z80_Q(cpu) (cpu).q
#define Z80_IM(cpu) (cpu).im
#define Z80_IFF1(cpu) (cpu).iff1
#define Z80_IFF2(cpu) (cpu).iff2

#define z80_t Z80
#define z80_reset(x) (z80_instant_reset(x), 0)
#define z80_tick(x,pins) (z80_run(x,1),0) // z80_execute()

void z80_interrupt(z80_t *z, int on) {
    // z80_int(z, on);
    z80_int(z, 1); z->int_line = false;
}

uint8_t read_int(void * context, uint16_t address) {
    return z80_int((z80_t*)context, 0), 0xff;
}

void z80_init(z80_t *z) {
    memset(z, 0, sizeof(z80_t));
    z80_power(z, 1);
    z->context = z;
    z->options = Z80_MODEL_ZILOG_NMOS;
    z->nop = NULL;
    z->fetch =
    z->fetch_opcode =
    z->read = read_byte;
    z->write = write_byte;
    z->in = read_io;
    z->out = write_io;
    z->inta = read_int;
/*
    z->halt         = NULL;
    z->nmia         = NULL;
    z->int_fetch    = NULL;
    z->ld_i_a       = NULL;
    z->ld_r_a       = NULL;
    z->reti         = NULL;
    z->retn         = NULL;
    z->hook         = NULL;
    z->illegal      = NULL;
*/
}

void zx_run_frame(Machine *self) {
    int *cycles = self->cycles;

    /* CPU cycles before the INT signal */
    cycles += z80_execute(&cpu, CYCLES_AT_INT - cycles);

    /* CPU cycles during the INT signal */
    z80_int(&cpu, Z_TRUE);
    cycles += z80_run(&cpu, (CYCLES_AT_INT + CYCLES_PER_INT) - cycles);
    z80_int(&cpu, Z_FALSE);

    /* CPU cycles after the INT signal */
    cycles += z80_execute(&cpu, CYCLES_PER_FRAME - cycles);

    cycles -= CYCLES_PER_FRAME;
}

#endif
