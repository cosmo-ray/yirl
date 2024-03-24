  OPCODE(BIT_ab, 0x2c) //break / interrupt
  OPCODE(ORA, 0x01) //or with accumulator
  OPCODE(ASL, 0x02) //arithmetic shift left
  OPCODE(PHP, 0x08) //push processor status (SR)
  OPCODE(BPL, 0x10) //branch on plus (negative clear)
  OPCODE(CLC, 0x18) //clear carry
  OPCODE(JSR, 0x20)
  OPCODE(AND_xindir_zp, 0x21) //and (with accumulator)
  OPCODE(ROL, 0x22) //rotate left
  OPCODE(PLP, 0x28) //pull processor status (SR)
  OPCODE(AND_im, 0x29) //and (with accumulator)
  OPCODE(BMI, 0x30) //branch on minus (negative set)
  OPCODE(SEC, 0x38) //set carry
  OPCODE(EOR_X_ZP_I, 0x41) //exclusive or (with accumulator)
  OPCODE(EOR_ZP, 0x45)
  OPCODE(EOR_IM, 0x49)
  OPCODE(EOR_AB, 0x4D)
  OPCODE(EOR_Y_AB, 0x59)
  OPCODE(EOR_X_AB, 0x5D)
  OPCODE(LSR_A, 0x4A) //logical shift right
  OPCODE(PHA, 0x48) //push accumulator
  OPCODE(JMP_ab, 0x4c) //jump //return from interrupt
  OPCODE(BVC, 0x50) //branch on overflow clear
  OPCODE(RTS, 0x60) //return from subroutine
  OPCODE(CLI, 0x58) //clear interrupt disable
  OPCODE(ADC_xindir_zp, 0x61) //add with carry
  OPCODE(ROR, 0x62) //rotate right
  OPCODE(PLA, 0x68) //pull accumulator
  OPCODE(ADC_im, 0x69) //add with carry
  OPCODE(BVS, 0x70) //branch on overflow set
  OPCODE(SEI, 0x78) //set interrupt disable
  OPCODE(STY, 0x80) //store Y
  OPCODE(STA, 0x81) //store accumulator
  OPCODE(STX, 0x82) //store X
  OPCODE(DEY, 0x88) //decrement Y
  OPCODE(TXA, 0x8A) //transfer X to accumulator
  OPCODE(STA_addr, 0x8D) //load accumulator
  OPCODE(STX_ab, 0x8E) //store X
  OPCODE(BCC, 0x90) //branch on carry clear
  OPCODE(TXS, 0x9A) //transfer X to stack pointer
  OPCODE(TYA, 0x9B) //transfer Y to accumulator
  OPCODE(STA_xaddr, 0x9D) //load accumulator
  OPCODE(LDY_im, 0xA0) //load Y
  OPCODE(LDA, 0xA1) //load accumulator
  OPCODE(LDX_im, 0xA2) //load X
  OPCODE(TAY, 0xA8) //transfer accumulator to Y
  OPCODE(LDA_im, 0xA9) //load accumulator
  OPCODE(LDA_addr, 0xad) //load accumulator
  OPCODE(LDA_addx, 0xbd) //load accumulator
  OPCODE(TAX, 0xAA) //transfer accumulator to X
  OPCODE(BCS, 0xB0) //branch on carry set
  OPCODE(CLV, 0xB8) //clear overflow
  OPCODE(TSX, 0xBA) //transfer stack pointer to X
  OPCODE(CPY, 0xC0) //compare with Y
  OPCODE(CMP, 0xC1) //compare (with accumulator)
  OPCODE(INY, 0xC8) //increment Y
  OPCODE(CMP_imediate, 0xC9) //branch on equal (zero set)
  OPCODE(DEX, 0xCA) //decrement X
  OPCODE(BNE, 0xD0) //branch on not equal (zero clear)
  OPCODE(CLD, 0xD8) //clear decimal
  OPCODE(CPX, 0xE0) //compare with X
  OPCODE(SBC, 0xE1) //subtract with carry
  OPCODE(INX, 0xE8) //increment X
  OPCODE(NOP, 0xEA) //no operation
  OPCODE(CPX_var, 0xEC) //compare with X
  OPCODE(BEQ, 0xF0) //branch on equal (zero set)
  OPCODE(SED, 0xF8) //set decimal

// unofficial

/* OPCODE(ALR), */
/* OPCODE(ANC), */
/* OPCODE(ARR), */
/* OPCODE(AXS), */
/* OPCODE(LAX), */
/* OPCODE(SAX), */

/* OPCODE(DCP), */
/* OPCODE(ISB), */
/* OPCODE(RLA), */
/* OPCODE(RRA) */
