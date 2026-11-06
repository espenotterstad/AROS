#ifndef ASM_RISCV_CPU_H
#define ASM_RISCV_CPU_H

/*
    Copyright (c) 2023-2026, The AROS Development Team. All rights reserved.
    $Id$

    Desc: assembler-level specific definitions for riscv CPUs,
          usable from both RV32 and RV64 code.
    Lang: english
*/

#include <aros/config.h>    /* __AROSPLATFORM_RISCV_MMODE__ */

#ifdef __cplusplus
extern "C" {
#endif

/* Memory barriers and hints */
static inline void fence(void)    { asm volatile("fence"      : : : "memory"); }
static inline void fence_r(void)  { asm volatile("fence r,rw" : : : "memory"); }
static inline void fence_w(void)  { asm volatile("fence w,rw" : : : "memory"); }
/* Zifencei is optional in the RVA22 profile - name it explicitly */
static inline void fence_i(void)
{
    asm volatile(".option push\n"
                 ".option arch, +zifencei\n"
                 "fence.i\n"
                 ".option pop" : : : "memory");
}
static inline void wfi(void)      { asm volatile("wfi"); }

/*
 * sstatus fields. The FS and VS dirty-tracking fields drive lazy FPU and
 * vector context switching: the scheduler only saves the unit's state
 * when the field reads back as DIRTY, and traps on first use when it is
 * set to OFF. VS reads as a hardwired zero when the vector extension is
 * not implemented, which is also how its presence is detected at runtime.
 */
#define SSTATUS_SIE         0x00000002UL
#define SSTATUS_SPIE        0x00000020UL
#define SSTATUS_UBE         0x00000040UL
#define SSTATUS_SPP         0x00000100UL
#define SSTATUS_VS          0x00000600UL /* Vector unit state (V ext)   */
#define SSTATUS_VS_OFF      0x00000000UL
#define SSTATUS_VS_INITIAL  0x00000200UL
#define SSTATUS_VS_CLEAN    0x00000400UL
#define SSTATUS_VS_DIRTY    0x00000600UL
#define SSTATUS_FS          0x00006000UL /* FPU state                   */
#define SSTATUS_FS_OFF      0x00000000UL
#define SSTATUS_FS_INITIAL  0x00002000UL
#define SSTATUS_FS_CLEAN    0x00004000UL
#define SSTATUS_FS_DIRTY    0x00006000UL
#define SSTATUS_SUM         0x00040000UL
#define SSTATUS_MXR         0x00080000UL

/* sie/sip interrupt-enable/pending bits */
#define SIE_SSIE            0x00000002UL /* Supervisor software (IPI) */
#define SIE_STIE            0x00000020UL /* Supervisor timer          */
#define SIE_SEIE            0x00000200UL /* Supervisor external       */

/* scause interrupt codes (with the top bit set) */
#define SCAUSE_IRQ_SSI      1
#define SCAUSE_IRQ_STI      5
#define SCAUSE_IRQ_SEI      9

/* mstatus fields; FS and VS sit at the same bits as in sstatus */
#define MSTATUS_MIE         0x00000008UL
#define MSTATUS_MPIE        0x00000080UL
#define MSTATUS_MPP         0x00001800UL /* 3 = M-mode                  */

/* mie/mip interrupt-enable/pending bits */
#define MIE_MSIE            0x00000008UL /* Machine software (IPI)    */
#define MIE_MTIE            0x00000080UL /* Machine timer             */
#define MIE_MEIE            0x00000800UL /* Machine external          */

/* mcause interrupt codes (with the top bit set) */
#define MCAUSE_IRQ_MSI      3
#define MCAUSE_IRQ_MTI      7
#define MCAUSE_IRQ_MEI      11

/*
 * The CSRs and bits of the privilege mode the kernel runs in, chosen at
 * build time: configure defines __AROSPLATFORM_RISCV_MMODE__ for
 * machine-mode platforms, supervisor mode (under SBI firmware) is the
 * default. Shared code uses these names; the S and M sets above stay
 * for code that is specific to one mode.
 */
#if defined(__AROSPLATFORM_RISCV_MMODE__)
#define CSR_STATUS          mstatus
#define CSR_IE              mie
#define CSR_IP              mip
#define CSR_EPC             mepc
#define CSR_CAUSE           mcause
#define CSR_TVAL            mtval
#define CSR_TVEC            mtvec
#define CSR_SCRATCH         mscratch
#define STATUS_IE           MSTATUS_MIE
#define STATUS_PIE          MSTATUS_MPIE
#define STATUS_PP_KERNEL    MSTATUS_MPP
#define IE_SOFT             MIE_MSIE
#define IE_TIMER            MIE_MTIE
#define IE_EXT              MIE_MEIE
#define CAUSE_IRQ_SOFT      MCAUSE_IRQ_MSI
#define CAUSE_IRQ_TIMER     MCAUSE_IRQ_MTI
#define CAUSE_IRQ_EXT       MCAUSE_IRQ_MEI
#else
#define CSR_STATUS          sstatus
#define CSR_IE              sie
#define CSR_IP              sip
#define CSR_EPC             sepc
#define CSR_CAUSE           scause
#define CSR_TVAL            stval
#define CSR_TVEC            stvec
#define CSR_SCRATCH         sscratch
#define STATUS_IE           SSTATUS_SIE
#define STATUS_PIE          SSTATUS_SPIE
#define STATUS_PP_KERNEL    SSTATUS_SPP
#define IE_SOFT             SIE_SSIE
#define IE_TIMER            SIE_STIE
#define IE_EXT              SIE_SEIE
#define CAUSE_IRQ_SOFT      SCAUSE_IRQ_SSI
#define CAUSE_IRQ_TIMER     SCAUSE_IRQ_STI
#define CAUSE_IRQ_EXT       SCAUSE_IRQ_SEI
#endif

/* FPU and vector state fields, the same in both modes */
#define STATUS_VS           SSTATUS_VS
#define STATUS_VS_OFF       SSTATUS_VS_OFF
#define STATUS_VS_INITIAL   SSTATUS_VS_INITIAL
#define STATUS_VS_CLEAN     SSTATUS_VS_CLEAN
#define STATUS_VS_DIRTY     SSTATUS_VS_DIRTY
#define STATUS_FS           SSTATUS_FS
#define STATUS_FS_OFF       SSTATUS_FS_OFF
#define STATUS_FS_INITIAL   SSTATUS_FS_INITIAL
#define STATUS_FS_CLEAN     SSTATUS_FS_CLEAN
#define STATUS_FS_DIRTY     SSTATUS_FS_DIRTY

/*
 * Vector extension CSR numbers. Numeric so they assemble without V in
 * the build's -march; accessing them traps unless sstatus.VS is enabled.
 */
#define CSR_VSTART          0x008
#define CSR_VCSR            0x00F
#define CSR_VL              0xC20
#define CSR_VTYPE           0xC21
#define CSR_VLENB           0xC22

/*
 * CSR accessors. 'csr' must be a compile time CSR name or number, or a
 * macro that expands to one (CSR_STATUS). csr_name() gives it as a
 * string for asm text of its own.
 */
#define __csr_str(csr)      #csr
#define csr_name(csr)       __csr_str(csr)

#define csr_read(csr)                                           \
({                                                              \
    unsigned long __v;                                          \
    asm volatile("csrr %0, " csr_name(csr)                      \
                 : "=r"(__v) : : "memory");                     \
    __v;                                                        \
})

#define csr_write(csr, val)                                     \
do {                                                            \
    unsigned long __v = (unsigned long)(val);                   \
    asm volatile("csrw " csr_name(csr) ", %0"                   \
                 : : "rK"(__v) : "memory");                     \
} while (0)

#define csr_swap(csr, val)                                      \
({                                                              \
    unsigned long __v = (unsigned long)(val);                   \
    asm volatile("csrrw %0, " csr_name(csr) ", %1"              \
                 : "=r"(__v) : "rK"(__v) : "memory");           \
    __v;                                                        \
})

#define csr_set(csr, mask)                                      \
do {                                                            \
    unsigned long __v = (unsigned long)(mask);                  \
    asm volatile("csrs " csr_name(csr) ", %0"                   \
                 : : "rK"(__v) : "memory");                     \
} while (0)

#define csr_clear(csr, mask)                                    \
do {                                                            \
    unsigned long __v = (unsigned long)(mask);                  \
    asm volatile("csrc " csr_name(csr) ", %0"                   \
                 : : "rK"(__v) : "memory");                     \
} while (0)

/* Set bits given as a 5-bit constant with csrsi, needing no register */
#define csr_set_imm(csr, imm)                                   \
    asm volatile("csrsi " csr_name(csr) ", %0" : : "K"(imm) : "memory")

#ifdef __cplusplus
}
#endif

#endif /* ASM_RISCV_CPU_H */
