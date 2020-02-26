; INTGLUE.ASM - Hardware interrupt 'glue' for LiteTask

; $Author:   Phlash  $
; $Date:   24 Jun 1995 20:23:34  $
; $Revision:   1.0  $

INCLUDE KERNEL.INC

IRQ0OFFSET     EQU      8
IRQ8OFFSET     EQU      70h-8

EXTRN _panic:far
EXTRN _schedule:far
EXTRN _setVector:far

PUBLIC _setIRQTrap
PUBLIC _clearIRQTrap
PUBLIC _chainIRQ

DGROUP GROUP _DATA

_DATA SEGMENT PUBLIC WORD 'DATA'

; The interrupt stack pointer table
intStckTable dd 16 dup(0)

; The user function callout table
intCallTable dd 16 dup(0)

; The saved interrupt vector table
oldVectTable dd 16 dup(0)

; The overrun protection flags & message
intInProgress db 16 dup(-1)
intOverrun    db 'Interrupt overrun', 0

_DATA ENDS

_TEXT SEGMENT PUBLIC WORD 'CODE'
   ASSUME CS:_TEXT,DS:DGROUP

; Interrupt entry points, each stores the IRQ no. in AX and jumps to HWINT

INTENTRY MACRO arg
IRQ&arg:
      PUSH AX
      MOV AX,arg
      JMP SHORT HWINT
ENDM

      INTENTRY 0
      INTENTRY 1
      INTENTRY 2
      INTENTRY 3
      INTENTRY 4
      INTENTRY 5
      INTENTRY 6
      INTENTRY 7
      INTENTRY 8
      INTENTRY 9
      INTENTRY 10
      INTENTRY 11
      INTENTRY 12
      INTENTRY 13
      INTENTRY 14
      INTENTRY 15

HWINT:
      PUSHREGS                    ; Save remaining CPU registers

      MOV DX,SS                   ; Save context in DX:CX (odd I know :)
      MOV CX,SP

      STI                         ; Turn those ol' ints back on..

      MOV BX,DGROUP               ; Restore DS for C code
      MOV DS,BX

      MOV BX,AX                   ; Test for IRQ overrun
      INC BYTE PTR [BX+intInProgress]
      JZ SHORT IRQOK
      PUSH BX
      PUSH DS                     ; Call kernel panic()
      MOV AX,OFFSET DGROUP:intOverrun
      PUSH AX
      CALL _panic
      ADD SP,04
      POP BX
      JMP SHORT restore

IRQOK:
      SHL BX,1                    ; Map IRQ number to table index
      SHL BX,1
                                  ; Switch to local interrupt stack
      MOV SS,WORD PTR [BX+intStckTable+02]
      MOV SP,WORD PTR [BX+intStckTable]

      PUSH DX                     ; Save old context for later
      PUSH CX

      PUSH AX                     ; Passing IRQ number as an argument..
      CALL [BX+intCallTable]      ; Call C handler for interrupt
      OR AX,AX                    ; Test return value:
      JZ SHORT noSched            ;                    0 => continue

      CALL _schedule              ; Call using previously saved context DX:CX
      POP BX                      ; Restore IRQ number to BX
      ADD SP,04
      MOV SS,DX                   ; Switch to new stack
      MOV SP,AX
      JMP SHORT restore

noSched:
      POP BX                      ; Restore IRQ number to BX
      POP CX                      ; Switch back to original stack
      POP DX
      MOV SS,DX
      MOV SP,CX

restore:
      DEC BYTE PTR [BX+intInProgress] ; Clear overrun protection flag
      RESTOREREGS                 ; Restore required context
      IRET

; extern int far setIRQTrap(int irq, void far *trap, void far *intStack);

_setIRQTrap PROC FAR
      PUSH BP
      MOV BP,SP
      MOV AX,[BP+06]
      
CHECKIRQ MACRO arg
      MOV CX,OFFSET _TEXT:IRQ&arg
      CMP AX,arg
      JZ SHORT SetIRQ
ENDM
      CHECKIRQ 0
      CHECKIRQ 1
      CHECKIRQ 2
      CHECKIRQ 3
      CHECKIRQ 4
      CHECKIRQ 5
      CHECKIRQ 6
      CHECKIRQ 7
      CHECKIRQ 8
      CHECKIRQ 9
      CHECKIRQ 10
      CHECKIRQ 11
      CHECKIRQ 12
      CHECKIRQ 13
      CHECKIRQ 14
      CHECKIRQ 15

      MOV AX,-1                   ; Oops, not a valid IRQ
      MOV DX,AX
      POP BP
      RET

SetIRQ:
      MOV BX,AX                   ; Store user function & stack top in tables
      SHL BX,1
      SHL BX,1
      MOV DX,[BP+08]
      MOV WORD PTR [BX+intCallTable],DX
      MOV DX,[BP+10]
      MOV WORD PTR [BX+intCallTable+02],DX
      MOV DX,[BP+12]
      MOV WORD PTR [BX+intStckTable],DX
      MOV DX,[BP+14]
      MOV WORD PTR [BX+intStckTable+02],DX
      PUSH BX                     ; We need this index later...
      PUSH CS                     ; Set the new interrupt vector
      PUSH CX
      CMP AX,8
      JL SHORT SetLow
      ADD AX,IRQ8OFFSET
      JMP SHORT SetINT
SetLow:
      ADD AX,IRQ0OFFSET
SetINT:
      PUSH AX
      CALL _setVector
      ADD SP,06
      POP BX                      ; Restore index and save original vector
      MOV WORD PTR [BX+oldVectTable],AX
      MOV WORD PTR [BX+oldVectTable+02],DX
      XOR AX,AX
      POP BP
      RET
_setIRQTrap ENDP

; extern int far clearIRQTrap(int irq);

_clearIRQTrap PROC FAR
      PUSH BP
      MOV BP,SP
      MOV AX,[BP+06]
      TEST AX,0FFF0h              ; Check that only bits 0,1,2,3 are set
      JZ SHORT ClearIRQ
      MOV AX,-1
      POP BP
      RET

ClearIRQ:                         ; Put the previous vector address back
      MOV BX,AX
      SHL BX,1
      SHL BX,1
      MOV CX,WORD PTR [BX+oldVectTable+02]
      PUSH CX
      MOV CX,WORD PTR [BX+oldVectTable]
      PUSH CX
      CMP AX,8
      JL SHORT ClearLow
      ADD AX,IRQ8OFFSET
      JMP SHORT ClearINT
ClearLow:
      ADD AX,IRQ0OFFSET
ClearINT:
      PUSH AX
      CALL _setVector
      ADD SP,06
      XOR AX,AX
      POP BP
      RET
_clearIRQTrap ENDP

; extern int far chainIRQ(int irq);

_chainIRQ   PROC FAR
      PUSH BP
      MOV BP,SP
      MOV BX,[BP+06]
      TEST BX,0FFF0h              ; Check that only bits 0,1,2,3 are set
      JZ SHORT ChainIRQ
      MOV AX,-1
      POP BP
      RET

ChainIRQ:
      SHL BX,1
      SHL BX,1
      PUSHF
      CALL [BX+oldVectTable]
      XOR AX,AX
      POP BP
      RET
_chainIRQ ENDP

_TEXT ENDS

END
