PUBLIC _int15trap
PUBLIC _int15inst
PUBLIC _int15dele

EXTRN _scancode:far

_TEXT SEGMENT PUBLIC WORD 'CODE'
   ASSUME CS:_TEXT

oldTrap  dd 0

_int15trap PROC FAR
   PUSHF
   CMP AH,4Fh
   JZ short next
   POPF
   JMP CS:[oldTrap]

next:
   XOR AH,AH
   PUSH AX
   CALL _scancode
   POP AX
   POPF
   STC
   RETF 2
_int15trap ENDP

_int15inst PROC FAR
   MOV AX,0
   MOV ES,AX
   CLI
   MOV AX,ES:[54h]
   MOV WORD PTR CS:[oldTrap],AX
   MOV AX,ES:[56h]
   MOV WORD PTR CS:[oldTrap+2],AX
   MOV AX,offset _int15trap
   MOV WORD PTR ES:[54h],AX
   MOV AX,CS
   MOV WORD PTR ES:[56h],AX
   STI
   RET
_int15inst ENDP

_int15dele PROC FAR
   MOV AX,0
   MOV ES,AX
   CLI
   MOV AX,WORD PTR CS:[oldTrap]
   MOV ES:[54h],AX
   MOV AX,WORD PTR CS:[oldTrap+2]
   MOV ES:[56h],AX
   STI
   RET
_int15dele ENDP

_TEXT ENDS

END
