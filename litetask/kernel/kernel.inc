; KERNEL.INC - Common macros and constants for LiteTask kernel assembly code

; $Author:   Phlash  $
; $Date:   24 Jun 1995 20:24:04  $
; $Revision:   1.0  $

.XLIST

; Size limits for stacks
MINSTACK EQU 1024
MAXSTACK EQU 32766
MINSTACKLEFT EQU 32

; Push all the CPU Regs except AX
PUSHREGS MACRO
   PUSH BX
   PUSH CX
   PUSH DX
   PUSH SI
   PUSH DI
   PUSH BP
   PUSH DS
   PUSH ES
ENDM

; Push all the CPU Regs including AX
PUSHAXREGS MACRO
   PUSH AX
   PUSHREGS
ENDM

; Restore the CPU regs
RESTOREREGS MACRO
   POP ES
   POP DS
   POP BP
   POP DI
   POP SI
   POP DX
   POP CX
   POP BX
   POP AX
ENDM

; Typical interrupt function wrapper
INTFUNCTION MACRO entry, func
EXTRN func:far
PUBLIC entry
entry PROC FAR
   PUSHAXREGS
   MOV AX,DGROUP
   MOV DS,AX
   CALL func
   RESTOREREGS
   IRET
entry ENDP
ENDM

; End
.LIST
