; EXESTART.ASM - .EXE startup hook

PUBLIC _exeStart

_TEXT SEGMENT PUBLIC WORD 'CODE'
   ASSUME CS:_TEXT

; int exeStart(exeHeader_t far *exeHeader, unsigned short loadSeg);

_exeStart PROC NEAR
   PUSH BP
   MOV BP,SP
   LES BX,[BP+04]            ; ES:BX -> exeHeader
   MOV DX,[BP+08]            ; DX = start segment of .EXE
   MOV AX,ES:[BX+0Eh]        ; AX = offset to SS
   MOV CX,ES:[BX+10h]        ; CX = initial SP
   ADD AX,DX                 ; AX = real value for SS

   CLI                       ; Swap to .EXE's stack
   MOV SS,AX
   MOV SP,CX
   STI

   MOV AX,ES:[BX+16h]        ; AX = offset to CS
   MOV CX,ES:[BX+14h]        ; CX = initial IP
   ADD AX,DX                 ; AX = real value for CS

   SUB DX,10h                ; DS = start segment - 10h (sort of PSP'ish)
   MOV DS,DX                 ; Set ES & DS to "PSP"
   MOV ES,DX
   PUSH AX                   ; Jump to .EXE startup via 'far return'
   PUSH CX
   RETF

_exeStart ENDP

_TEXT ENDS

END
