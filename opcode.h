/* Coding: ANSI */
#ifndef OPCODE_H
#define OPCODE_H

#include "tiger_type.h"

/*
 31-16 15-8 7-0
 opcode o1 o2
 stack-based machine
 0  NOP
 1  ADD
 2  SUB
 3  MUL
 4  DIV
 5  PSH
 
------
 PSH a'index in symtable
 PSH 2
 ADD
 MOV a'index in symtable
 JMP xx
 RET 
*/
namespace tiger{
    
}// namespace tiger


#endif
