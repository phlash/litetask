; MISCGLUE.ASM - LiteTask Kernel Miscellaneous Assembly Glue routines

; $Author:   Phlash  $
; $Date:   24 Jun 1995  8:31:40  $
; $Revision:   1.7  $

EXTRN TASKEXIT:far

PUBLIC _newContext
PUBLIC _enableInts
PUBLIC _disableInts
PUBLIC _lockInts
PUBLIC _unlockInts
PUBLIC _setVector
PUBLIC _outp
PUBLIC _outpw
PUBLIC _inp
PUBLIC _inpw
PUBLIC _int86

DGROUP GROUP _DATA

_DATA SEGMENT PUBLIC PARA 'DATA'
_DATA ENDS

_TEXT SEGMENT PUBLIC PARA 'CODE'
   ASSUME DS:DGROUP,SS:DGROUP
   ASSUME CS:_TEXT

; void far * far newContext(void far *stacktop, void far *taskAddress);
;
; Sets up a new task context prior to running the task for the first time
; NOTE: this function is only intended to be used by the kernel to create
; a new stack. From user code, use 'newTask(...)'.
;
_newContext PROC FAR
   PUSH BP
   MOV BP,SP
   PUSH SI
   PUSH DI

; Read stack top address and task address
   MOV AX,[BP+06]   ; stack address
   MOV DX,[BP+08]
   MOV SI,[BP+10]   ; task address
   MOV DI,[BP+12]

; Change to task's stack temporarily (save flags for later)
   PUSHF
   MOV CX,SS        ; save 'C' stack in CX:BX
   MOV BX,SP
   MOV SS,DX        ; switch to task stack (DX:AX)
   MOV SP,AX

; Set flags for new task
   STI
   CLD

; Now create dummy task CPU context on specified stack
   ; 1) return address is TASKEXIT (far return to cope with large model..)
   PUSH CS
   MOV AX,offset TASKEXIT
   PUSH AX

   ; 2) flag register and far task entry point address for IRET to 'restore'
   PUSHF
   PUSH DI
   PUSH SI

   ; 3) register set for CPU context, all zero except DS
   MOV AX,0
   PUSH AX              ; AX
   PUSH AX              ; BX
   PUSH AX              ; CX
   PUSH AX              ; DX
   PUSH AX              ; SI
   PUSH AX              ; DI
   PUSH AX              ; BP
   PUSH DS              ; Use current value of DS (DGROUP)
   PUSH AX              ; ES

; Record current SS:SP to return to calling function
   MOV DX,SS
   MOV AX,SP

; revert to 'C' stack, restore original flags
   MOV SS,CX
   MOV SP,BX
   POPF

; normal C exit code
   POP DI
   POP SI
   POP BP
   RET

_newContext ENDP

; void far enableInts(void); 
;
; Enables interrupts
;
_enableInts PROC FAR
   STI
   RET
_enableInts ENDP

; void far disableInts(void); 
;
; Disables interrupts
;
_disableInts PROC FAR
   CLI
   RET
_disableInts ENDP

; short far lockInts(void); 
;
; Grab current flag register into AX, disable interrupts and return AX
;
_lockInts PROC FAR
   PUSHF
   POP AX
   CLI
   RET
_lockInts ENDP

; void unlockInts(short flag);
;
; Restore flags register from argument (BEWARE!!)
;
_unlockInts PROC FAR
   PUSH BP
   MOV BP,SP
   MOV AX,[BP+06]
   PUSH AX
   POPF
   POP BP
   RET
_unlockInts ENDP

; void far * far setVector(int intNumber, void far *address);
;
; Set the specified PC interupt vector to address, return old vector.
;
_setVector PROC FAR
   PUSH BP
   MOV BP,SP
   PUSH ES

; Disable interrupts
   PUSHF
   CLI

; Point ES:BX at interrupt table entry
   XOR AX,AX
   MOV ES,AX
   MOV BX,[BP+06]
   SHL BX,1
   SHL BX,1

; Save original vector in DX:AX (to return to caller)
   MOV AX,ES:[BX+00]
   MOV DX,ES:[BX+02]

; Now install new vector
   MOV CX,[BP+08]
   MOV ES:[BX+00],CX
   MOV CX,[BP+10]
   MOV ES:[BX+02],CX

; Restore interrupt state, and return
   POPF
   POP ES
   POP BP
   RET

_setVector ENDP

; void far outp(int port, unsigned char byte);
;
; Put a byte out of a port

_outp PROC FAR

   PUSH BP
   MOV BP,SP
   MOV DX,[BP+06]
   MOV AL,[BP+08]
   OUT DX,AL
   POP BP
   RET

_outp ENDP

; void far outpw(int port, unsigned short word);
;
; Put a word out of a port (AT's only)

_outpw PROC FAR

   PUSH BP
   MOV BP,SP
   MOV DX,[BP+06]
   MOV AX,[BP+08]
   OUT DX,AX
   POP BP
   RET

_outpw ENDP

; unsigned char far inp(int port);
;
; Get a byte from a port

_inp PROC FAR

   PUSH BP
   MOV BP,SP
   MOV DX,[BP+06]
   IN  AL,DX
   XOR AH,AH
   POP BP
   RET

_inp ENDP

; unsigned short far inpw(int port);
;
; Get a word from a port (AT's only)

_inpw PROC FAR

   PUSH BP
   MOV BP,SP
   MOV DX,[BP+06]
   IN  AX,DX
   POP BP
   RET

_inpw ENDP

; int far int86(short intr, union REGS *regs);
;
; Generates any 8086 interrupt using specified register set. Note the special
; handling for INT 25h/26h (MS-DOS) which do not remove the flags from the
; stack on return (yuk!).

LocalFrame STRUC                   ; Stack frame of local variables
_bp   dw ?
_int  db 6 dup(?)
addr  dd ?
LocalFrame ENDS

_int86 PROC FAR
      PUSH  BP                     ; Save C context and access parameters
      MOV   BP,SP
      SUB   SP,10                  ; Create local stack space
      
      PUSH  DS                     ; Save important registers
      PUSH  SI
      PUSH  DI

      MOV   BYTE PTR [BP-06],0CDh  ; Build an 8086 INT xx instruction
      MOV   AX,[BP+06]             ; on the stack (in local space)
      MOV   [BP-05],AL
      CMP   AL,25h                 ; Check for INT 25h / 26h
      JZ    _int86_1
      CMP   AL,26h
      JZ    _int86_1
      MOV   BYTE PTR [BP-04],0CBh  ; Normal RETF instruction
      JMP   _int86_2
_int86_1:
      MOV   BYTE PTR [BP-02],0CBh  ; Fudge for INT 25h / 26h
      MOV   BYTE PTR [BP-03],44h   ; Add two INC SP instructions
      MOV   BYTE PTR [BP-04],44h
_int86_2:     
      MOV   [BP-08],SS             ; Save the address of the..
      LEA   AX,[BP-06]             ; INT xx instruction for an
      MOV   [BP-10],AX             ; indirect call (see below)
      
      LDS   DI,[BP+08]             ; Get address of 'union REGS' parameter
      PUSH  [DI+14]                ; Save regs.x.di on stack
      PUSH  [DI+12]                ; Save regs.x.si on stack
      MOV   AX,[DI]
      MOV   BX,[DI+02]
      MOV   CX,[DI+04]
      MOV   DX,[DI+06]
      MOV   SI,[DI+10]
      MOV   ES,SI
      MOV   SI,[DI+08]
      MOV   DS,SI
      POP   SI                     ; Set SI & DI from stack
      POP   DI

      PUSH  BP                     ; Save BP across the interrupt
      CALL  [BP-02-addr]           ; Call the INT xx instruction
      POP   BP                     ; Restore BP

      PUSH  DS                     ; Save these for later
      PUSH  SI
      PUSH  DI
      PUSHF

      CLD                          ; Restore direction flag
      JB    _int86_3               ; Was carry flag set?
      XOR   SI,SI                  ; NO, return 0
      JMP   _int86_4
_int86_3:
      MOV   SI,0001                ; YES, return 1
_int86_4:
      LDS   DI,[BP+08]             ; Get address of 'union REGS' parameter,
      MOV   [DI],AX                ; Store AX early so that I can set the
      MOV   AX,SI                  ; function return value and re-use SI..
      POP   [DI+16]                ; Store CPU flags from stack
      POP   [DI+14]                ; Store saved DI,SI & DS from stack
      POP   [DI+12]
      POP   [DI+08]
      MOV   SI,ES
      MOV   [DI+10],SI
      MOV   [DI+06],DX
      MOV   [DI+04],CX
      MOV   [DI+02],BX

      POP   DI                     ; Restore important registers
      POP   SI
      POP   DS
      MOV   SP,BP                  ; Remove local stack space
      POP   BP                     ; Restore C context and return.
      RET
_int86 ENDP

_TEXT ENDS

END
