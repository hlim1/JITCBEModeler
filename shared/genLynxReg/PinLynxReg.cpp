#include "PinLynxReg.h"

const REG LynxReg2RegArr[] = {
#ifdef TARGET_IA32
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_SEG_CS,
	REG_SEG_DS,
	REG_SEG_ES,
	REG_SEG_FS,
	REG_SEG_GS,
	REG_SEG_SS,
	REG_MXCSR,
	REG_ST0,
	REG_ST1,
	REG_ST2,
	REG_ST3,
	REG_ST4,
	REG_ST5,
	REG_ST6,
	REG_ST7,
	REG_FPSW,
	REG_FPCW,
	REG_FPTAG_FULL,
	REG_FPIP_OFF,
	REG_FPIP_SEL,
	REG_FPDP_OFF,
	REG_FPDP_SEL,
	REG_FPOPCODE,
	REG_DR0,
	REG_DR1,
	REG_DR2,
	REG_DR3,
	REG_DR4,
	REG_DR5,
	REG_DR6,
	REG_DR7,
	REG_CR0,
	REG_CR1,
	REG_CR2,
	REG_CR3,
	REG_CR4,
	REG_GFLAGS,
	REG_YMM0,
	REG_YMM1,
	REG_YMM2,
	REG_YMM3,
	REG_YMM4,
	REG_YMM5,
	REG_YMM6,
	REG_YMM7,
	REG_EIP,
	REG_EAX,
	REG_EBX,
	REG_ECX,
	REG_EDX,
	REG_EBP,
	REG_ESI,
	REG_EDI,
	REG_ESP,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_IP,
	REG_AX,
	REG_BX,
	REG_CX,
	REG_DX,
	REG_BP,
	REG_SI,
	REG_DI,
	REG_SP,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_AH,
	REG_BH,
	REG_CH,
	REG_DH,
	REG_AL,
	REG_BL,
	REG_CL,
	REG_DL,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_XMM0,
	REG_XMM1,
	REG_XMM2,
	REG_XMM3,
	REG_XMM4,
	REG_XMM5,
	REG_XMM6,
	REG_XMM7,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_MM0,
	REG_MM1,
	REG_MM2,
	REG_MM3,
	REG_MM4,
	REG_MM5,
	REG_MM6,
	REG_MM7,
	REG_INVALID_,
	REG_INVALID_,
#endif
#ifdef TARGET_IA32E
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_RIP,
	REG_RAX,
	REG_RBX,
	REG_RCX,
	REG_RDX,
	REG_RBP,
	REG_RSI,
	REG_RDI,
	REG_RSP,
	REG_R8,
	REG_R9,
	REG_R10,
	REG_R11,
	REG_R12,
	REG_R13,
	REG_R14,
	REG_R15,
	REG_SEG_CS,
	REG_SEG_DS,
	REG_SEG_ES,
	REG_SEG_FS,
	REG_SEG_GS,
	REG_SEG_SS,
	REG_MXCSR,
	REG_ST0,
	REG_ST1,
	REG_ST2,
	REG_ST3,
	REG_ST4,
	REG_ST5,
	REG_ST6,
	REG_ST7,
	REG_FPSW,
	REG_FPCW,
	REG_FPTAG_FULL,
	REG_FPIP_OFF,
	REG_FPIP_SEL,
	REG_FPDP_OFF,
	REG_FPDP_SEL,
	REG_FPOPCODE,
	REG_DR0,
	REG_DR1,
	REG_DR2,
	REG_DR3,
	REG_DR4,
	REG_DR5,
	REG_DR6,
	REG_DR7,
	REG_CR0,
	REG_CR1,
	REG_CR2,
	REG_CR3,
	REG_CR4,
	REG_GFLAGS,
	REG_YMM0,
	REG_YMM1,
	REG_YMM2,
	REG_YMM3,
	REG_YMM4,
	REG_YMM5,
	REG_YMM6,
	REG_YMM7,
	REG_EIP,
	REG_EAX,
	REG_EBX,
	REG_ECX,
	REG_EDX,
	REG_EBP,
	REG_ESI,
	REG_EDI,
	REG_ESP,
	REG_R8D,
	REG_R9D,
	REG_R10D,
	REG_R11D,
	REG_R12D,
	REG_R13D,
	REG_R14D,
	REG_R15D,
	REG_IP,
	REG_AX,
	REG_BX,
	REG_CX,
	REG_DX,
	REG_BP,
	REG_SI,
	REG_DI,
	REG_SP,
	REG_R8W,
	REG_R9W,
	REG_R10W,
	REG_R11W,
	REG_R12W,
	REG_R13W,
	REG_R14W,
	REG_R15W,
	REG_AH,
	REG_BH,
	REG_CH,
	REG_DH,
	REG_AL,
	REG_BL,
	REG_CL,
	REG_DL,
	REG_BPL,
	REG_SIL,
	REG_DIL,
	REG_SPL,
	REG_R8B,
	REG_R9B,
	REG_R10B,
	REG_R11B,
	REG_R12B,
	REG_R13B,
	REG_R14B,
	REG_R15B,
	REG_YMM8,
	REG_YMM9,
	REG_YMM10,
	REG_YMM11,
	REG_YMM12,
	REG_YMM13,
	REG_YMM14,
	REG_YMM15,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_XMM0,
	REG_XMM1,
	REG_XMM2,
	REG_XMM3,
	REG_XMM4,
	REG_XMM5,
	REG_XMM6,
	REG_XMM7,
	REG_XMM8,
	REG_XMM9,
	REG_XMM10,
	REG_XMM11,
	REG_XMM12,
	REG_XMM13,
	REG_XMM14,
	REG_XMM15,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_MM0,
	REG_MM1,
	REG_MM2,
	REG_MM3,
	REG_MM4,
	REG_MM5,
	REG_MM6,
	REG_MM7,
	REG_INVALID_,
	REG_INVALID_,
#endif
#ifdef TARGET_MIC
	REG_ZMM0,
	REG_ZMM1,
	REG_ZMM2,
	REG_ZMM3,
	REG_ZMM4,
	REG_ZMM5,
	REG_ZMM6,
	REG_ZMM7,
	REG_ZMM8,
	REG_ZMM9,
	REG_ZMM10,
	REG_ZMM11,
	REG_ZMM12,
	REG_ZMM13,
	REG_ZMM14,
	REG_ZMM15,
	REG_ZMM16,
	REG_ZMM17,
	REG_ZMM18,
	REG_ZMM19,
	REG_ZMM20,
	REG_ZMM21,
	REG_ZMM22,
	REG_ZMM23,
	REG_ZMM24,
	REG_ZMM25,
	REG_ZMM26,
	REG_ZMM27,
	REG_ZMM28,
	REG_ZMM29,
	REG_ZMM30,
	REG_ZMM31,
	REG_K0,
	REG_K1,
	REG_K2,
	REG_K3,
	REG_K4,
	REG_K5,
	REG_K6,
	REG_K7,
	REG_RIP,
	REG_RAX,
	REG_RBX,
	REG_RCX,
	REG_RDX,
	REG_RBP,
	REG_RSI,
	REG_RDI,
	REG_RSP,
	REG_R8,
	REG_R9,
	REG_R10,
	REG_R11,
	REG_R12,
	REG_R13,
	REG_R14,
	REG_R15,
	REG_SEG_CS,
	REG_SEG_DS,
	REG_SEG_ES,
	REG_SEG_FS,
	REG_SEG_GS,
	REG_SEG_SS,
	REG_MXCSR,
	REG_ST0,
	REG_ST1,
	REG_ST2,
	REG_ST3,
	REG_ST4,
	REG_ST5,
	REG_ST6,
	REG_ST7,
	REG_FPSW,
	REG_FPCW,
	REG_FPTAG_FULL,
	REG_FPIP_OFF,
	REG_FPIP_SEL,
	REG_FPDP_OFF,
	REG_FPDP_SEL,
	REG_FPOPCODE,
	REG_DR0,
	REG_DR1,
	REG_DR2,
	REG_DR3,
	REG_DR4,
	REG_DR5,
	REG_DR6,
	REG_DR7,
	REG_CR0,
	REG_CR1,
	REG_CR2,
	REG_CR3,
	REG_CR4,
	REG_GFLAGS,
	REG_YMM0,
	REG_YMM1,
	REG_YMM2,
	REG_YMM3,
	REG_YMM4,
	REG_YMM5,
	REG_YMM6,
	REG_YMM7,
	REG_EIP,
	REG_EAX,
	REG_EBX,
	REG_ECX,
	REG_EDX,
	REG_EBP,
	REG_ESI,
	REG_EDI,
	REG_ESP,
	REG_R8D,
	REG_R9D,
	REG_R10D,
	REG_R11D,
	REG_R12D,
	REG_R13D,
	REG_R14D,
	REG_R15D,
	REG_IP,
	REG_AX,
	REG_BX,
	REG_CX,
	REG_DX,
	REG_BP,
	REG_SI,
	REG_DI,
	REG_SP,
	REG_R8W,
	REG_R9W,
	REG_R10W,
	REG_R11W,
	REG_R12W,
	REG_R13W,
	REG_R14W,
	REG_R15W,
	REG_AH,
	REG_BH,
	REG_CH,
	REG_DH,
	REG_AL,
	REG_BL,
	REG_CL,
	REG_DL,
	REG_BPL,
	REG_SIL,
	REG_DIL,
	REG_SPL,
	REG_R8B,
	REG_R9B,
	REG_R10B,
	REG_R11B,
	REG_R12B,
	REG_R13B,
	REG_R14B,
	REG_R15B,
	REG_YMM8,
	REG_YMM9,
	REG_YMM10,
	REG_YMM11,
	REG_YMM12,
	REG_YMM13,
	REG_YMM14,
	REG_YMM15,
	REG_YMM16,
	REG_YMM17,
	REG_YMM18,
	REG_YMM19,
	REG_YMM20,
	REG_YMM21,
	REG_YMM22,
	REG_YMM23,
	REG_YMM24,
	REG_YMM25,
	REG_YMM26,
	REG_YMM27,
	REG_YMM28,
	REG_YMM29,
	REG_YMM30,
	REG_YMM31,
	REG_XMM0,
	REG_XMM1,
	REG_XMM2,
	REG_XMM3,
	REG_XMM4,
	REG_XMM5,
	REG_XMM6,
	REG_XMM7,
	REG_XMM8,
	REG_XMM9,
	REG_XMM10,
	REG_XMM11,
	REG_XMM12,
	REG_XMM13,
	REG_XMM14,
	REG_XMM15,
	REG_XMM16,
	REG_XMM17,
	REG_XMM18,
	REG_XMM19,
	REG_XMM20,
	REG_XMM21,
	REG_XMM22,
	REG_XMM23,
	REG_XMM24,
	REG_XMM25,
	REG_XMM26,
	REG_XMM27,
	REG_XMM28,
	REG_XMM29,
	REG_XMM30,
	REG_XMM31,
	REG_MM0,
	REG_MM1,
	REG_MM2,
	REG_MM3,
	REG_MM4,
	REG_MM5,
	REG_MM6,
	REG_MM7,
	REG_INVALID_,
	REG_INVALID_,
#endif
#ifdef TARGET_NONE
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_INVALID_,
	REG_FPU_STACK,
	REG_INVALID_,
#endif
};
REG LynxReg2Reg(LynxReg lReg) {
	return LynxReg2RegArr[lReg];
}
LynxReg Reg2LynxReg(REG reg) {
	switch(reg) {
#ifdef TARGET_IA32
		case REG_SEG_CS:
			return LYNX_SEG_CS;
		case REG_SEG_DS:
			return LYNX_SEG_DS;
		case REG_SEG_ES:
			return LYNX_SEG_ES;
		case REG_SEG_FS:
			return LYNX_SEG_FS;
		case REG_SEG_GS:
			return LYNX_SEG_GS;
		case REG_SEG_SS:
			return LYNX_SEG_SS;
		case REG_MXCSR:
			return LYNX_MXCSR;
		case REG_ST0:
			return LYNX_ST0;
		case REG_ST1:
			return LYNX_ST1;
		case REG_ST2:
			return LYNX_ST2;
		case REG_ST3:
			return LYNX_ST3;
		case REG_ST4:
			return LYNX_ST4;
		case REG_ST5:
			return LYNX_ST5;
		case REG_ST6:
			return LYNX_ST6;
		case REG_ST7:
			return LYNX_ST7;
		case REG_FPSW:
			return LYNX_FPSW;
		case REG_FPCW:
			return LYNX_FPCW;
		case REG_FPTAG_FULL:
			return LYNX_FPTAG_FULL;
		case REG_FPIP_OFF:
			return LYNX_FPIP_OFF;
		case REG_FPIP_SEL:
			return LYNX_FPIP_SEL;
		case REG_FPDP_OFF:
			return LYNX_FPDP_OFF;
		case REG_FPDP_SEL:
			return LYNX_FPDP_SEL;
		case REG_FPOPCODE:
			return LYNX_FPOPCODE;
		case REG_DR0:
			return LYNX_DR0;
		case REG_DR1:
			return LYNX_DR1;
		case REG_DR2:
			return LYNX_DR2;
		case REG_DR3:
			return LYNX_DR3;
		case REG_DR4:
			return LYNX_DR4;
		case REG_DR5:
			return LYNX_DR5;
		case REG_DR6:
			return LYNX_DR6;
		case REG_DR7:
			return LYNX_DR7;
		case REG_CR0:
			return LYNX_CR0;
		case REG_CR1:
			return LYNX_CR1;
		case REG_CR2:
			return LYNX_CR2;
		case REG_CR3:
			return LYNX_CR3;
		case REG_CR4:
			return LYNX_CR4;
		case REG_GFLAGS:
			return LYNX_GFLAGS;
		case REG_YMM0:
			return LYNX_YMM0;
		case REG_YMM1:
			return LYNX_YMM1;
		case REG_YMM2:
			return LYNX_YMM2;
		case REG_YMM3:
			return LYNX_YMM3;
		case REG_YMM4:
			return LYNX_YMM4;
		case REG_YMM5:
			return LYNX_YMM5;
		case REG_YMM6:
			return LYNX_YMM6;
		case REG_YMM7:
			return LYNX_YMM7;
		case REG_EIP:
			return LYNX_EIP;
		case REG_EAX:
			return LYNX_EAX;
		case REG_EBX:
			return LYNX_EBX;
		case REG_ECX:
			return LYNX_ECX;
		case REG_EDX:
			return LYNX_EDX;
		case REG_EBP:
			return LYNX_EBP;
		case REG_ESI:
			return LYNX_ESI;
		case REG_EDI:
			return LYNX_EDI;
		case REG_ESP:
			return LYNX_ESP;
		case REG_IP:
			return LYNX_IP;
		case REG_AX:
			return LYNX_AX;
		case REG_BX:
			return LYNX_BX;
		case REG_CX:
			return LYNX_CX;
		case REG_DX:
			return LYNX_DX;
		case REG_BP:
			return LYNX_BP;
		case REG_SI:
			return LYNX_SI;
		case REG_DI:
			return LYNX_DI;
		case REG_SP:
			return LYNX_SP;
		case REG_AH:
			return LYNX_AH;
		case REG_BH:
			return LYNX_BH;
		case REG_CH:
			return LYNX_CH;
		case REG_DH:
			return LYNX_DH;
		case REG_AL:
			return LYNX_AL;
		case REG_BL:
			return LYNX_BL;
		case REG_CL:
			return LYNX_CL;
		case REG_DL:
			return LYNX_DL;
		case REG_XMM0:
			return LYNX_XMM0;
		case REG_XMM1:
			return LYNX_XMM1;
		case REG_XMM2:
			return LYNX_XMM2;
		case REG_XMM3:
			return LYNX_XMM3;
		case REG_XMM4:
			return LYNX_XMM4;
		case REG_XMM5:
			return LYNX_XMM5;
		case REG_XMM6:
			return LYNX_XMM6;
		case REG_XMM7:
			return LYNX_XMM7;
		case REG_MM0:
			return LYNX_MM0;
		case REG_MM1:
			return LYNX_MM1;
		case REG_MM2:
			return LYNX_MM2;
		case REG_MM3:
			return LYNX_MM3;
		case REG_MM4:
			return LYNX_MM4;
		case REG_MM5:
			return LYNX_MM5;
		case REG_MM6:
			return LYNX_MM6;
		case REG_MM7:
			return LYNX_MM7;
		case REG_INVALID_:
			return LYNX_INVALID;
#endif
#ifdef TARGET_IA32E
		case REG_RIP:
			return LYNX_RIP;
		case REG_RAX:
			return LYNX_RAX;
		case REG_RBX:
			return LYNX_RBX;
		case REG_RCX:
			return LYNX_RCX;
		case REG_RDX:
			return LYNX_RDX;
		case REG_RBP:
			return LYNX_RBP;
		case REG_RSI:
			return LYNX_RSI;
		case REG_RDI:
			return LYNX_RDI;
		case REG_RSP:
			return LYNX_RSP;
		case REG_R8:
			return LYNX_R8;
		case REG_R9:
			return LYNX_R9;
		case REG_R10:
			return LYNX_R10;
		case REG_R11:
			return LYNX_R11;
		case REG_R12:
			return LYNX_R12;
		case REG_R13:
			return LYNX_R13;
		case REG_R14:
			return LYNX_R14;
		case REG_R15:
			return LYNX_R15;
		case REG_SEG_CS:
			return LYNX_SEG_CS;
		case REG_SEG_DS:
			return LYNX_SEG_DS;
		case REG_SEG_ES:
			return LYNX_SEG_ES;
		case REG_SEG_FS:
			return LYNX_SEG_FS;
		case REG_SEG_GS:
			return LYNX_SEG_GS;
		case REG_SEG_SS:
			return LYNX_SEG_SS;
		case REG_MXCSR:
			return LYNX_MXCSR;
		case REG_ST0:
			return LYNX_ST0;
		case REG_ST1:
			return LYNX_ST1;
		case REG_ST2:
			return LYNX_ST2;
		case REG_ST3:
			return LYNX_ST3;
		case REG_ST4:
			return LYNX_ST4;
		case REG_ST5:
			return LYNX_ST5;
		case REG_ST6:
			return LYNX_ST6;
		case REG_ST7:
			return LYNX_ST7;
		case REG_FPSW:
			return LYNX_FPSW;
		case REG_FPCW:
			return LYNX_FPCW;
		case REG_FPTAG_FULL:
			return LYNX_FPTAG_FULL;
		case REG_FPIP_OFF:
			return LYNX_FPIP_OFF;
		case REG_FPIP_SEL:
			return LYNX_FPIP_SEL;
		case REG_FPDP_OFF:
			return LYNX_FPDP_OFF;
		case REG_FPDP_SEL:
			return LYNX_FPDP_SEL;
		case REG_FPOPCODE:
			return LYNX_FPOPCODE;
		case REG_DR0:
			return LYNX_DR0;
		case REG_DR1:
			return LYNX_DR1;
		case REG_DR2:
			return LYNX_DR2;
		case REG_DR3:
			return LYNX_DR3;
		case REG_DR4:
			return LYNX_DR4;
		case REG_DR5:
			return LYNX_DR5;
		case REG_DR6:
			return LYNX_DR6;
		case REG_DR7:
			return LYNX_DR7;
		case REG_CR0:
			return LYNX_CR0;
		case REG_CR1:
			return LYNX_CR1;
		case REG_CR2:
			return LYNX_CR2;
		case REG_CR3:
			return LYNX_CR3;
		case REG_CR4:
			return LYNX_CR4;
		case REG_GFLAGS:
			return LYNX_GFLAGS;
		case REG_YMM0:
			return LYNX_YMM0;
		case REG_YMM1:
			return LYNX_YMM1;
		case REG_YMM2:
			return LYNX_YMM2;
		case REG_YMM3:
			return LYNX_YMM3;
		case REG_YMM4:
			return LYNX_YMM4;
		case REG_YMM5:
			return LYNX_YMM5;
		case REG_YMM6:
			return LYNX_YMM6;
		case REG_YMM7:
			return LYNX_YMM7;
		case REG_EIP:
			return LYNX_EIP;
		case REG_EAX:
			return LYNX_EAX;
		case REG_EBX:
			return LYNX_EBX;
		case REG_ECX:
			return LYNX_ECX;
		case REG_EDX:
			return LYNX_EDX;
		case REG_EBP:
			return LYNX_EBP;
		case REG_ESI:
			return LYNX_ESI;
		case REG_EDI:
			return LYNX_EDI;
		case REG_ESP:
			return LYNX_ESP;
		case REG_R8D:
			return LYNX_R8D;
		case REG_R9D:
			return LYNX_R9D;
		case REG_R10D:
			return LYNX_R10D;
		case REG_R11D:
			return LYNX_R11D;
		case REG_R12D:
			return LYNX_R12D;
		case REG_R13D:
			return LYNX_R13D;
		case REG_R14D:
			return LYNX_R14D;
		case REG_R15D:
			return LYNX_R15D;
		case REG_IP:
			return LYNX_IP;
		case REG_AX:
			return LYNX_AX;
		case REG_BX:
			return LYNX_BX;
		case REG_CX:
			return LYNX_CX;
		case REG_DX:
			return LYNX_DX;
		case REG_BP:
			return LYNX_BP;
		case REG_SI:
			return LYNX_SI;
		case REG_DI:
			return LYNX_DI;
		case REG_SP:
			return LYNX_SP;
		case REG_R8W:
			return LYNX_R8W;
		case REG_R9W:
			return LYNX_R9W;
		case REG_R10W:
			return LYNX_R10W;
		case REG_R11W:
			return LYNX_R11W;
		case REG_R12W:
			return LYNX_R12W;
		case REG_R13W:
			return LYNX_R13W;
		case REG_R14W:
			return LYNX_R14W;
		case REG_R15W:
			return LYNX_R15W;
		case REG_AH:
			return LYNX_AH;
		case REG_BH:
			return LYNX_BH;
		case REG_CH:
			return LYNX_CH;
		case REG_DH:
			return LYNX_DH;
		case REG_AL:
			return LYNX_AL;
		case REG_BL:
			return LYNX_BL;
		case REG_CL:
			return LYNX_CL;
		case REG_DL:
			return LYNX_DL;
		case REG_BPL:
			return LYNX_BPL;
		case REG_SIL:
			return LYNX_SIL;
		case REG_DIL:
			return LYNX_DIL;
		case REG_SPL:
			return LYNX_SPL;
		case REG_R8B:
			return LYNX_R8B;
		case REG_R9B:
			return LYNX_R9B;
		case REG_R10B:
			return LYNX_R10B;
		case REG_R11B:
			return LYNX_R11B;
		case REG_R12B:
			return LYNX_R12B;
		case REG_R13B:
			return LYNX_R13B;
		case REG_R14B:
			return LYNX_R14B;
		case REG_R15B:
			return LYNX_R15B;
		case REG_YMM8:
			return LYNX_YMM8;
		case REG_YMM9:
			return LYNX_YMM9;
		case REG_YMM10:
			return LYNX_YMM10;
		case REG_YMM11:
			return LYNX_YMM11;
		case REG_YMM12:
			return LYNX_YMM12;
		case REG_YMM13:
			return LYNX_YMM13;
		case REG_YMM14:
			return LYNX_YMM14;
		case REG_YMM15:
			return LYNX_YMM15;
		case REG_XMM0:
			return LYNX_XMM0;
		case REG_XMM1:
			return LYNX_XMM1;
		case REG_XMM2:
			return LYNX_XMM2;
		case REG_XMM3:
			return LYNX_XMM3;
		case REG_XMM4:
			return LYNX_XMM4;
		case REG_XMM5:
			return LYNX_XMM5;
		case REG_XMM6:
			return LYNX_XMM6;
		case REG_XMM7:
			return LYNX_XMM7;
		case REG_XMM8:
			return LYNX_XMM8;
		case REG_XMM9:
			return LYNX_XMM9;
		case REG_XMM10:
			return LYNX_XMM10;
		case REG_XMM11:
			return LYNX_XMM11;
		case REG_XMM12:
			return LYNX_XMM12;
		case REG_XMM13:
			return LYNX_XMM13;
		case REG_XMM14:
			return LYNX_XMM14;
		case REG_XMM15:
			return LYNX_XMM15;
		case REG_MM0:
			return LYNX_MM0;
		case REG_MM1:
			return LYNX_MM1;
		case REG_MM2:
			return LYNX_MM2;
		case REG_MM3:
			return LYNX_MM3;
		case REG_MM4:
			return LYNX_MM4;
		case REG_MM5:
			return LYNX_MM5;
		case REG_MM6:
			return LYNX_MM6;
		case REG_MM7:
			return LYNX_MM7;
		case REG_INVALID_:
			return LYNX_INVALID;
#endif
#ifdef TARGET_MIC
		case REG_ZMM0:
			return LYNX_ZMM0;
		case REG_ZMM1:
			return LYNX_ZMM1;
		case REG_ZMM2:
			return LYNX_ZMM2;
		case REG_ZMM3:
			return LYNX_ZMM3;
		case REG_ZMM4:
			return LYNX_ZMM4;
		case REG_ZMM5:
			return LYNX_ZMM5;
		case REG_ZMM6:
			return LYNX_ZMM6;
		case REG_ZMM7:
			return LYNX_ZMM7;
		case REG_ZMM8:
			return LYNX_ZMM8;
		case REG_ZMM9:
			return LYNX_ZMM9;
		case REG_ZMM10:
			return LYNX_ZMM10;
		case REG_ZMM11:
			return LYNX_ZMM11;
		case REG_ZMM12:
			return LYNX_ZMM12;
		case REG_ZMM13:
			return LYNX_ZMM13;
		case REG_ZMM14:
			return LYNX_ZMM14;
		case REG_ZMM15:
			return LYNX_ZMM15;
		case REG_ZMM16:
			return LYNX_ZMM16;
		case REG_ZMM17:
			return LYNX_ZMM17;
		case REG_ZMM18:
			return LYNX_ZMM18;
		case REG_ZMM19:
			return LYNX_ZMM19;
		case REG_ZMM20:
			return LYNX_ZMM20;
		case REG_ZMM21:
			return LYNX_ZMM21;
		case REG_ZMM22:
			return LYNX_ZMM22;
		case REG_ZMM23:
			return LYNX_ZMM23;
		case REG_ZMM24:
			return LYNX_ZMM24;
		case REG_ZMM25:
			return LYNX_ZMM25;
		case REG_ZMM26:
			return LYNX_ZMM26;
		case REG_ZMM27:
			return LYNX_ZMM27;
		case REG_ZMM28:
			return LYNX_ZMM28;
		case REG_ZMM29:
			return LYNX_ZMM29;
		case REG_ZMM30:
			return LYNX_ZMM30;
		case REG_ZMM31:
			return LYNX_ZMM31;
		case REG_K0:
			return LYNX_K0;
		case REG_K1:
			return LYNX_K1;
		case REG_K2:
			return LYNX_K2;
		case REG_K3:
			return LYNX_K3;
		case REG_K4:
			return LYNX_K4;
		case REG_K5:
			return LYNX_K5;
		case REG_K6:
			return LYNX_K6;
		case REG_K7:
			return LYNX_K7;
		case REG_RIP:
			return LYNX_RIP;
		case REG_RAX:
			return LYNX_RAX;
		case REG_RBX:
			return LYNX_RBX;
		case REG_RCX:
			return LYNX_RCX;
		case REG_RDX:
			return LYNX_RDX;
		case REG_RBP:
			return LYNX_RBP;
		case REG_RSI:
			return LYNX_RSI;
		case REG_RDI:
			return LYNX_RDI;
		case REG_RSP:
			return LYNX_RSP;
		case REG_R8:
			return LYNX_R8;
		case REG_R9:
			return LYNX_R9;
		case REG_R10:
			return LYNX_R10;
		case REG_R11:
			return LYNX_R11;
		case REG_R12:
			return LYNX_R12;
		case REG_R13:
			return LYNX_R13;
		case REG_R14:
			return LYNX_R14;
		case REG_R15:
			return LYNX_R15;
		case REG_SEG_CS:
			return LYNX_SEG_CS;
		case REG_SEG_DS:
			return LYNX_SEG_DS;
		case REG_SEG_ES:
			return LYNX_SEG_ES;
		case REG_SEG_FS:
			return LYNX_SEG_FS;
		case REG_SEG_GS:
			return LYNX_SEG_GS;
		case REG_SEG_SS:
			return LYNX_SEG_SS;
		case REG_MXCSR:
			return LYNX_MXCSR;
		case REG_ST0:
			return LYNX_ST0;
		case REG_ST1:
			return LYNX_ST1;
		case REG_ST2:
			return LYNX_ST2;
		case REG_ST3:
			return LYNX_ST3;
		case REG_ST4:
			return LYNX_ST4;
		case REG_ST5:
			return LYNX_ST5;
		case REG_ST6:
			return LYNX_ST6;
		case REG_ST7:
			return LYNX_ST7;
		case REG_FPSW:
			return LYNX_FPSW;
		case REG_FPCW:
			return LYNX_FPCW;
		case REG_FPTAG_FULL:
			return LYNX_FPTAG_FULL;
		case REG_FPIP_OFF:
			return LYNX_FPIP_OFF;
		case REG_FPIP_SEL:
			return LYNX_FPIP_SEL;
		case REG_FPDP_OFF:
			return LYNX_FPDP_OFF;
		case REG_FPDP_SEL:
			return LYNX_FPDP_SEL;
		case REG_FPOPCODE:
			return LYNX_FPOPCODE;
		case REG_DR0:
			return LYNX_DR0;
		case REG_DR1:
			return LYNX_DR1;
		case REG_DR2:
			return LYNX_DR2;
		case REG_DR3:
			return LYNX_DR3;
		case REG_DR4:
			return LYNX_DR4;
		case REG_DR5:
			return LYNX_DR5;
		case REG_DR6:
			return LYNX_DR6;
		case REG_DR7:
			return LYNX_DR7;
		case REG_CR0:
			return LYNX_CR0;
		case REG_CR1:
			return LYNX_CR1;
		case REG_CR2:
			return LYNX_CR2;
		case REG_CR3:
			return LYNX_CR3;
		case REG_CR4:
			return LYNX_CR4;
		case REG_GFLAGS:
			return LYNX_GFLAGS;
		case REG_YMM0:
			return LYNX_YMM0;
		case REG_YMM1:
			return LYNX_YMM1;
		case REG_YMM2:
			return LYNX_YMM2;
		case REG_YMM3:
			return LYNX_YMM3;
		case REG_YMM4:
			return LYNX_YMM4;
		case REG_YMM5:
			return LYNX_YMM5;
		case REG_YMM6:
			return LYNX_YMM6;
		case REG_YMM7:
			return LYNX_YMM7;
		case REG_EIP:
			return LYNX_EIP;
		case REG_EAX:
			return LYNX_EAX;
		case REG_EBX:
			return LYNX_EBX;
		case REG_ECX:
			return LYNX_ECX;
		case REG_EDX:
			return LYNX_EDX;
		case REG_EBP:
			return LYNX_EBP;
		case REG_ESI:
			return LYNX_ESI;
		case REG_EDI:
			return LYNX_EDI;
		case REG_ESP:
			return LYNX_ESP;
		case REG_R8D:
			return LYNX_R8D;
		case REG_R9D:
			return LYNX_R9D;
		case REG_R10D:
			return LYNX_R10D;
		case REG_R11D:
			return LYNX_R11D;
		case REG_R12D:
			return LYNX_R12D;
		case REG_R13D:
			return LYNX_R13D;
		case REG_R14D:
			return LYNX_R14D;
		case REG_R15D:
			return LYNX_R15D;
		case REG_IP:
			return LYNX_IP;
		case REG_AX:
			return LYNX_AX;
		case REG_BX:
			return LYNX_BX;
		case REG_CX:
			return LYNX_CX;
		case REG_DX:
			return LYNX_DX;
		case REG_BP:
			return LYNX_BP;
		case REG_SI:
			return LYNX_SI;
		case REG_DI:
			return LYNX_DI;
		case REG_SP:
			return LYNX_SP;
		case REG_R8W:
			return LYNX_R8W;
		case REG_R9W:
			return LYNX_R9W;
		case REG_R10W:
			return LYNX_R10W;
		case REG_R11W:
			return LYNX_R11W;
		case REG_R12W:
			return LYNX_R12W;
		case REG_R13W:
			return LYNX_R13W;
		case REG_R14W:
			return LYNX_R14W;
		case REG_R15W:
			return LYNX_R15W;
		case REG_AH:
			return LYNX_AH;
		case REG_BH:
			return LYNX_BH;
		case REG_CH:
			return LYNX_CH;
		case REG_DH:
			return LYNX_DH;
		case REG_AL:
			return LYNX_AL;
		case REG_BL:
			return LYNX_BL;
		case REG_CL:
			return LYNX_CL;
		case REG_DL:
			return LYNX_DL;
		case REG_BPL:
			return LYNX_BPL;
		case REG_SIL:
			return LYNX_SIL;
		case REG_DIL:
			return LYNX_DIL;
		case REG_SPL:
			return LYNX_SPL;
		case REG_R8B:
			return LYNX_R8B;
		case REG_R9B:
			return LYNX_R9B;
		case REG_R10B:
			return LYNX_R10B;
		case REG_R11B:
			return LYNX_R11B;
		case REG_R12B:
			return LYNX_R12B;
		case REG_R13B:
			return LYNX_R13B;
		case REG_R14B:
			return LYNX_R14B;
		case REG_R15B:
			return LYNX_R15B;
		case REG_YMM8:
			return LYNX_YMM8;
		case REG_YMM9:
			return LYNX_YMM9;
		case REG_YMM10:
			return LYNX_YMM10;
		case REG_YMM11:
			return LYNX_YMM11;
		case REG_YMM12:
			return LYNX_YMM12;
		case REG_YMM13:
			return LYNX_YMM13;
		case REG_YMM14:
			return LYNX_YMM14;
		case REG_YMM15:
			return LYNX_YMM15;
		case REG_YMM16:
			return LYNX_YMM16;
		case REG_YMM17:
			return LYNX_YMM17;
		case REG_YMM18:
			return LYNX_YMM18;
		case REG_YMM19:
			return LYNX_YMM19;
		case REG_YMM20:
			return LYNX_YMM20;
		case REG_YMM21:
			return LYNX_YMM21;
		case REG_YMM22:
			return LYNX_YMM22;
		case REG_YMM23:
			return LYNX_YMM23;
		case REG_YMM24:
			return LYNX_YMM24;
		case REG_YMM25:
			return LYNX_YMM25;
		case REG_YMM26:
			return LYNX_YMM26;
		case REG_YMM27:
			return LYNX_YMM27;
		case REG_YMM28:
			return LYNX_YMM28;
		case REG_YMM29:
			return LYNX_YMM29;
		case REG_YMM30:
			return LYNX_YMM30;
		case REG_YMM31:
			return LYNX_YMM31;
		case REG_XMM0:
			return LYNX_XMM0;
		case REG_XMM1:
			return LYNX_XMM1;
		case REG_XMM2:
			return LYNX_XMM2;
		case REG_XMM3:
			return LYNX_XMM3;
		case REG_XMM4:
			return LYNX_XMM4;
		case REG_XMM5:
			return LYNX_XMM5;
		case REG_XMM6:
			return LYNX_XMM6;
		case REG_XMM7:
			return LYNX_XMM7;
		case REG_XMM8:
			return LYNX_XMM8;
		case REG_XMM9:
			return LYNX_XMM9;
		case REG_XMM10:
			return LYNX_XMM10;
		case REG_XMM11:
			return LYNX_XMM11;
		case REG_XMM12:
			return LYNX_XMM12;
		case REG_XMM13:
			return LYNX_XMM13;
		case REG_XMM14:
			return LYNX_XMM14;
		case REG_XMM15:
			return LYNX_XMM15;
		case REG_XMM16:
			return LYNX_XMM16;
		case REG_XMM17:
			return LYNX_XMM17;
		case REG_XMM18:
			return LYNX_XMM18;
		case REG_XMM19:
			return LYNX_XMM19;
		case REG_XMM20:
			return LYNX_XMM20;
		case REG_XMM21:
			return LYNX_XMM21;
		case REG_XMM22:
			return LYNX_XMM22;
		case REG_XMM23:
			return LYNX_XMM23;
		case REG_XMM24:
			return LYNX_XMM24;
		case REG_XMM25:
			return LYNX_XMM25;
		case REG_XMM26:
			return LYNX_XMM26;
		case REG_XMM27:
			return LYNX_XMM27;
		case REG_XMM28:
			return LYNX_XMM28;
		case REG_XMM29:
			return LYNX_XMM29;
		case REG_XMM30:
			return LYNX_XMM30;
		case REG_XMM31:
			return LYNX_XMM31;
		case REG_MM0:
			return LYNX_MM0;
		case REG_MM1:
			return LYNX_MM1;
		case REG_MM2:
			return LYNX_MM2;
		case REG_MM3:
			return LYNX_MM3;
		case REG_MM4:
			return LYNX_MM4;
		case REG_MM5:
			return LYNX_MM5;
		case REG_MM6:
			return LYNX_MM6;
		case REG_MM7:
			return LYNX_MM7;
		case REG_INVALID_:
			return LYNX_INVALID;
#endif
#ifdef TARGET_NONE
		case REG_FPU_STACK:
			return LYNX_FPU_STACK;
#endif
		default:
			PIN_MutexLock(&errorLock);
			fprintf(errorFile, "Unknown Reg: %d\n", reg);
			PIN_MutexUnlock(&errorLock);
	}
	return LYNX_INVALID;
}
