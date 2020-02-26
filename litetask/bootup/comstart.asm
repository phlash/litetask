; COMSTART.ASM - C Startup code for .COM programs

; NOTE WELL: ALWAYS link this file first!!

PUBLIC __acrtused
PUBLIC __chkstk

__acrtused equ 1

EXTRN _main:near

DGROUP GROUP _DATA,CONST,_BSS

_TEXT SEGMENT PUBLIC WORD 'CODE'
   ASSUME CS:_TEXT

ORG 100h

; Loader entry point.
; CS=This code segment, DS=?, ES=?, SS:SP=useable stack (1k)
start PROC FAR
   MOV SI,DS               ; Save DS. This is either the boot sector segment
                           ; or the PSP segment if loaded from DOS.

   PUSH SS                 ; Save current stack frame
   POP ES
   MOV BP,SP
                           ; Calculate my DGROUP from CS and end marker (Yuk!)
   MOV AX,offset _TEXT:startDGroup
   ADD AX,0Fh              ; Add a correction for rounding error
   MOV CL,4                ; Divide by 16 to get a segment value..
   SHR AX,CL
   MOV BX,CS               ; Add the load address (CS)..
   ADD AX,BX
   MOV DS,AX               ; et voila!
   ASSUME DS:DGROUP

   MOV SS,AX               ; Set up a 32k-byte stack with SS = DS
   MOV SP,8000h
   PUSH ES                 ; Save previous stack frame
   PUSH BP

   PUSH SI                 ; Save entry value of DS on new stack
   MOV AX,SI               ; Let's see if we have been loaded by DOS....
   SUB AX,BX               ; compare the Stack and code segments
   JNZ SHORT notDOS        ; unequal = loaded by boot sector.
   MOV AX,-1
   PUSH AX                 ; DOS loaded us, so push -1 (to indicate DOS).. 
   CALL _main
   ADD SP,4
   MOV AH,4Ch              ; DOS termination (exit code in AL)
   INT 21h
notDOS:
   XOR DH,DH               ; Clear high byte (DL still holds drive ID)
   PUSH DX                 ; push drive ID byte..
   CALL _main              ; run Loader...
   ADD SP,4
   POP BP                  ; Restore previous stack frame
   POP SS
   MOV SP,BP
   POP DS                  ; Restore DS of primary loader
   RET                     ; Return to primary loader (provides reboot code)
start ENDP

__chkstk PROC NEAR
   POP CX
   MOV BX,SP
   SUB BX,AX
   MOV SP,BX
   JMP CX
__chkstk ENDP

_TEXT ENDS

;NOTE: PARAgraph alignment required for first segment in DGROUP..

_DATA SEGMENT PUBLIC PARA 'DATA'
startDGroup label byte
_DATA ENDS

CONST SEGMENT PUBLIC WORD 'CONST'
CONST ENDS

_BSS SEGMENT PUBLIC WORD 'BSS'
_BSS ENDS

END start
