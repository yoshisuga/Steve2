//
//  main.c
//  6502
//
//  Created by Tamas Rudnai on 7/14/19.
//  Copyright © 2019 Tamas Rudnai. All rights reserved.
//
// This file is part of Steve ][ -- The Apple ][ Emulator.
//
// Steve ][ is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Steve ][ is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Steve ][.  If not, see <https://www.gnu.org/licenses/>.
//


#ifndef __6502_INSTR_LOAD_STORE_H__
#define __6502_INSTR_LOAD_STORE_H__

#include <ctype.h>
#include "../../dev/mem/mmio.h"

/**
 LDA  Load Accumulator with Memory
 
 M -> A                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     LDA #oper     A9    2     2
 zeropage      LDA oper      A5    2     3
 zeropage,X    LDA oper,X    B5    2     4
 absolute      LDA oper      AD    3     4
 absolute,X    LDA oper,X    BD    3     4*
 absolute,Y    LDA oper,Y    B9    3     4*
 (indirect,X)  LDA (oper,X)  A1    2     6
 (indirect),Y  LDA (oper),Y  B1    2     5*
**/
INLINE void LDA( uint8_t src ) {
    dbgPrintf("LDA(%02X) ", src);
    disPrintf(disassembly.inst, "LDA");
    set_flags_NZ(m6502.A = src);
}

/**
 LDX  Load Index X with Memory
 
 M -> X                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     LDX #oper     A2    2     2
 zeropage      LDX oper      A6    2     3
 zeropage,Y    LDX oper,Y    B6    2     4
 absolute      LDX oper      AE    3     4
 absolute,Y    LDX oper,Y    BE    3     4*
**/
INLINE void LDX( uint8_t src ) {
    dbgPrintf("LDX(%02X) ", src);
    disPrintf(disassembly.inst, "LDX");
    set_flags_NZ(m6502.X = src);
}

/**
 LDY  Load Index Y with Memory
 
 M -> Y                           N Z C I D V
                                  + + - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 immidiate     LDY #oper     A0    2     2
 zeropage      LDY oper      A4    2     3
 zeropage,X    LDY oper,X    B4    2     4
 absolute      LDY oper      AC    3     4
 absolute,X    LDY oper,X    BC    3     4*
**/
INLINE void LDY( uint8_t src ) {
    dbgPrintf("LDY(%02X) ", src);
    disPrintf(disassembly.inst, "LDY");
    set_flags_NZ(m6502.Y = src);
}


char * charConv =
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_ !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~?"
;


/**
 STR   Store Data in Destination
 
 (not a real instruction, only a helper function)
**/
INLINE void STR( uint16_t addr, uint8_t src ) {
    dbgPrintf("STR [%04X], %02X ", addr, src );
    memwrite(addr, src);
}

/**
 STA  Store Accumulator in Memory
 
 A -> M                           N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      STA oper      85    2     3
 zeropage,X    STA oper,X    95    2     4
 absolute      STA oper      8D    3     4
 absolute,X    STA oper,X    9D    3     5
 absolute,Y    STA oper,Y    99    3     5
 (indirect,X)  STA (oper,X)  81    2     6
 (indirect),Y  STA (oper),Y  91    2     6
**/
INLINE void STA( uint16_t addr ) {
    dbgPrintf("STA ");
    disPrintf(disassembly.inst, "STA");
    STR(addr, m6502.A);
}

/**
 STX  Store Index X in Memory
 
 X -> M                           N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      STX oper      86    2     3
 zeropage,Y    STX oper,Y    96    2     4
 absolute      STX oper      8E    3     4
 **/
INLINE void STX( uint16_t addr ) {
    dbgPrintf("STX ");
    disPrintf(disassembly.inst, "STX");
    STR(addr, m6502.X);
}

/**
 STY  Sore Index Y in Memory
 
 Y -> M                           N Z C I D V
                                  - - - - - -
 
 addressing    assembler    opc  bytes  cyles
 --------------------------------------------
 zeropage      STY oper      84    2     3
 zeropage,X    STY oper,X    94    2     4
 absolute      STY oper      8C    3     4
 **/
INLINE void STY( uint16_t addr ) {
    dbgPrintf("STY ");
    disPrintf(disassembly.inst, "STY");
    STR(addr, m6502.Y);
}

#endif // __6502_INSTR_LOAD_STORE_H__

