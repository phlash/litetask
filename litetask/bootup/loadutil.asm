; LOADUTIL.ASM - Utility functions for Liteload.com

PUBLIC _biosChar
PUBLIC _biosRead
PUBLIC _biosKey

_TEXT SEGMENT PUBLIC WORD 'CODE'
   ASSUME CS:_TEXT

; int biosChar(char c);

_biosChar PROC NEAR
   PUSH BP
   MOV BP,SP
   MOV AH,0Eh
   MOV AL,[BP+04]
   MOV BX,7
   INT 10h
   POP BP
   RET
_biosChar ENDP

; int biosKey(BYTE function);

_biosKey PROC NEAR
   PUSH BP
   MOV BP,SP
   MOV AH,[BP+04]
   CMP AH,1
   JZ  SHORT getStatus
   INT 16h
   POP BP
   RET

getStatus:
   INT 16h
   JNZ SHORT keyWaiting
   MOV AX,-1
keyWaiting:
   POP BP
   RET
_biosKey ENDP

; int biosRead(WORD drive, DWORD logSector, BYTE nSectors,
;              BPB far *bpb, char far *buffer);

_biosRead PROC NEAR
   PUSH BP
   MOV BP,SP
   PUSH ES
   LES BX,[BP+12]
   MOV CX,ES
   OR  CX,BX
   JZ  SHORT noBPB
   MOV AX,[BP+06]
   MOV DX,[BP+08]
   CALL logToPhys
   JMP SHORT callBIOS
noBPB:
   MOV DH,[BP+08]
   MOV CX,[BP+06]
callBIOS:
   MOV AH,02h
   MOV AL,[BP+10]
   LES BX,[BP+16]
   MOV DL,[BP+04]
   INT 13h
   JC  SHORT oops
   XOR AX,AX
oops:
   POP ES
   POP BP
   RET
_biosRead ENDP

; Convert logical sector number (in DX:AX) into Cyl/Head/Sector for BIOS
; using BPB data pointed to by ES:BX.
logToPhys PROC NEAR
; Save original values
   PUSH DX
   PUSH AX

; First the cylinder number (stored in CH and top bits of CL)
   PUSH AX
   MOV AX,ES:[BX+18h]
   MUL WORD PTR ES:[BX+1Ah]
   MOV CX,AX
   POP AX
   DIV CX
   MOV CL,06
   SHL AH,CL
   XCHG AH,AL
   MOV CX,AX

; Next the head number (stored in DH) & sector number (OR'ed into CL)
   POP AX
   POP DX
   DIV WORD PTR ES:[BX+18h]
   INC DL
   OR  CL,DL
   XOR DX,DX
   DIV WORD PTR ES:[BX+1Ah]
   MOV DH,DL
   RET
logToPhys ENDP

_TEXT ENDS

END
