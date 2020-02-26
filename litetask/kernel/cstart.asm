; CSTART.ASM - C startup code for ASHBYSOFT Scheduler

; $Author:   Phlash  $
; $Date:   24 Jun 1995 19:35:20  $
; $Revision:   1.2  $

; ***************************************************

INCLUDE KERNEL.INC

; Public symbols
PUBLIC __acrtused   ; Trick to pull in this startup code
PUBLIC _psp         ; Program access to PSP segment
PUBLIC _stackEnd    ; End of stack, start of heap

; main() program entry point
EXTRN _main:far

; Declare data groupings

DGROUP GROUP CONST,_BSS,_DATA,STACK

; Entry code segment

_TEXT SEGMENT PUBLIC PARA 'CODE'
   ASSUME CS:_TEXT,DS:DGROUP,SS:STACK

; Trick to pull in this startup code (A Typical Microsoft bodge!!)
__acrtused:

; DOS entry point,DS=ES='PSP',CS=_TEXT,SS=STACK,SP=_stackEnd

c_start PROC FAR
   MOV AX,DGROUP    ; Make data segment addressable
   MOV DS,AX
   
   MOV [_psp],ES    ; Save PSP segment

                    ; Create ASCIIZ string from command tail
   MOV BL,ES:[80h]  ; offset 80h in PSP contains string size...
   XOR BH,BH        ; so place a zero at the end of the string
   ADD BX,81h
   MOV Byte ptr ES:[BX],0

   PUSH ES          ; Push a 'char far *' to the command tail for main()
   MOV AX,81h
   PUSH AX
   CALL _main       ; Transfer control to C code
   ADD SP,04

   MOV AH,4Ch       ; Return exit code to DOS
   INT 21h
c_start ENDP

_TEXT ENDS

; Constants
;
CONST SEGMENT PUBLIC PARA 'CONST'
CONST ENDS

; Uninitialized data
;
_BSS SEGMENT PUBLIC PARA 'BSS'
_BSS ENDS

; Program initialized data
;
_DATA SEGMENT PUBLIC PARA 'DATA'

_psp dw 0

_DATA ENDS

; Program stack segment
;
STACK SEGMENT STACK PARA 'STACK'
   db MINSTACK dup(?)
_stackEnd label far
STACK ENDS

END c_start
