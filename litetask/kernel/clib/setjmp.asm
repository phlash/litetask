; SETJMP.ASM

; $Author:   Phlash  $
; $Date:   26 Oct 1993 20:01:52  $
; $Header:   C:/phil/litetask/kernel/clib/setjmp.asv   1.0   26 Oct 1993 20:01:52   Phlash  $

PUBLIC _setjmp
PUBLIC _longjmp

_TEXT SEGMENT PUBLIC WORD 'CODE'
ASSUME CS:_TEXT

; int far setjmp(int far *buf);
;
; buffer contents are: { BP, DI, SI, SP, SS, IP, CS, DS }
_setjmp PROC FAR
	MOV   AX,BP
	MOV   BP,SP
	MOV   DX,DS
	LDS   BX,[BP+04]
	MOV   [BX],AX       ; BP
	MOV   [BX+02],DI    ; DI
	MOV   [BX+04],SI    ; SI
	MOV   [BX+06],SP    ; SP
	MOV   CX,SS
	MOV   [BX+08],CX    ; SS
	MOV   CX,[BP+00]
	MOV   [BX+10],CX    ; IP
	MOV   CX,[BP+02]
	MOV   [BX+12],CX    ; CS
	MOV   [BX+14],DX    ; DS
	MOV   DS,DX
	MOV   BP,AX
	XOR   AX,AX
	RET
_setjmp ENDP

; void far longjmp(int far *buf, int ret);

_longjmp PROC FAR
	MOV   BP,SP
	MOV   AX,[BP+08]
	OR AX,AX
	JNZ   _skip
	INC   AX
_skip:
	LDS   BX,[BP+04]
	MOV   DI,[BX+02]  ; DI
	MOV   SI,[BX+04]  ; SI
	CLI
	MOV   SP,[BX+06]  ; SP
	MOV   CX,[BX+08]
	MOV   SS,CX       ; SS
	STI
	MOV   BP,SP
	MOV   CX,[BX+10]
	MOV   [BP+00],CX  ; IP (put on stack)
	MOV   CX,[BX+12]
	MOV   [BP+02],CX  ; CS (put on stack)
	MOV   BP,[BX]     ; BP
	MOV   DS,[BX+14]  ; DS
	RET               ; Returns to setjmp call
_longjmp ENDP

_TEXT ENDS

END
