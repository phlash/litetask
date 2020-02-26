; INT15.ASM - Routines for interfacing to the BIOS multi-tasking support via
; interrupt 15h

PUBLIC _installInt15
PUBLIC _removeInt15
PUBLIC _checkInt15

EXTRN _getTaskHandle:far
EXTRN _suspendTask:far
EXTRN _resumeTask:far
EXTRN _setVector:far

DGROUP GROUP _DATA

_DATA SEGMENT PUBLIC WORD 'DATA'
_DATA ENDS

_TEXT SEGMENT PUBLIC WORD 'CODE'
   ASSUME CS:_TEXT,DS:DGROUP

installed  dw 0
oldvector  dd 0
waitDevs   db 128 dup(0)
waitTasks  dd 128 dup(0)

int15handler PROC FAR
; Check interrupt function code
   PUSHF
   CMP AH,90h
   JE SHORT deviceWait
   CMP AH,91h
   JE SHORT devicePost
   
toBIOS:
   CALL CS:[oldVector]         ; Nothing to do with me Guv!
   RETF 2
   
; device Wait operation, check for a device which generates devicePost's
deviceWait:
   CMP AL,80h                  ; Only device types < 80h..
   JNC SHORT toBIOS            ; others back to BIOS

   PUSH AX                     ; Save register set
   PUSH BX
   PUSH CX
   PUSH DX
   PUSH SI
   PUSH DI
   PUSH DS
   PUSH ES
   PUSH BP

   MOV  BL,AL                  ; Store wait device in BX
   XOR  BH,BH
   PUSH BX                     ; Save on stack for later

   INC CS:[waitDevs+BX]        ; Set wait device flag

   MOV  AX,DGROUP              ; Set data segment
   MOV  DS,AX

   CALL _getTaskHandle         ; Store current task handle
   POP BX                      ; Restore BX from stack..
   PUSH BX                     ; and save it again
   SHL BX,1                    ; Times 4 for handle table
   SHL BX,1
   MOV WORD PTR CS:[waitTasks+BX],AX
   MOV WORD PTR CS:[waitTasks+BX+02],DX

   CALL _suspendTask           ; Suspend this thread (also re-enables ints)

   POP BX                      ; Restore wait device to BX
   XOR AL,AL
   MOV CS:[waitDevs+BX],AL     ; Clear wait device flag

   POP BP                      ; Restore register set
   POP ES
   POP DS
   POP DI
   POP SI
   POP DX
   POP CX
   POP BX
   POP AX
   
   POPF                        ; Restore flags
   STC                         ; Set carry flag
   RETF 2                      ; Bye!

; Check for post device compatible with waiting device
devicePost:
   CMP AL,80h                  ; Only device types < 80h..
   JNC SHORT toBIOS            ; others back to BIOS

   PUSH BX                     ; Preserve BX
   MOV BL,AL                   ; Put post device in BX
   XOR BH,BH
   CMP CS:[waitDevs+BX],0      ; Are we waiting on this device?
   POP BX                      ; Restore BX
   JZ  SHORT toBIOS            ; No, so hand over to BIOS
   
   PUSH AX                     ; Save register set
   PUSH BX
   PUSH CX
   PUSH DX
   PUSH SI
   PUSH DI
   PUSH DS
   PUSH ES
   PUSH BP

   MOV BL,AL                   ; Put post device in BX (again!)
   XOR BH,BH
   SHL BX,1                    ; Times 4 for handle table
   SHL BX,1

   MOV  AX,DGROUP              ; Set data segment
   MOV  DS,AX

   XOR AX,AX
   PUSH AX                     ; resume suspended thread (exit code 0)
   PUSH WORD PTR CS:[waitTasks+BX+02]
   PUSH WORD PTR CS:[waitTasks+BX]
   CALL _resumeTask
   ADD SP,6

   POP BP                      ; Restore register set
   POP ES
   POP DS
   POP DI
   POP SI
   POP DX
   POP CX
   POP BX
   POP AX

   POPF                        ; Restore flags
   XOR AH,AH                   ; Clear AH
   STI                         ; Re-enable interrupts
   RETF 2                      ; Done!


int15handler ENDP

_installInt15 PROC FAR
   MOV AX,_TEXT
   PUSH AX
   MOV AX,offset int15handler
   PUSH AX
   MOV AX,15h
   PUSH AX
   CALL _setVector
   ADD SP,6

   MOV WORD PTR CS:[oldvector],AX
   MOV WORD PTR CS:[oldvector+02],DX
   INC CS:[installed]
   RET
_installInt15 ENDP

_removeInt15 PROC FAR
   PUSH WORD PTR CS:[oldvector+02]
   PUSH WORD PTR CS:[oldvector]
   MOV AX,15h
   PUSH AX
   CALL _setVector
   ADD SP,6
   XOR AX,AX
   MOV CS:[installed],AX
   RET
_removeInt15 ENDP

_checkInt15 PROC FAR
   MOV AX,CS:[installed]
   RET
_checkInt15 ENDP

_TEXT ENDS

END
