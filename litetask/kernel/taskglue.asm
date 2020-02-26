; TASKGLUE.ASM: LiteTask Kernel Task Switcher Glue routines

; $Author:   Phlash  $
; $Date:   24 Jun 1995  8:37:56  $
; $Revision:   1.7  $

INCLUDE KERNEL.INC

EXTRN _taskExit:far
EXTRN _schedule:far

PUBLIC _scheduleEntry
PUBLIC _beginScheduler
PUBLIC TASKEXIT

DGROUP GROUP _DATA

_DATA SEGMENT PUBLIC PARA 'DATA'
_DATA ENDS


_TEXT SEGMENT PUBLIC PARA 'CODE'
   ASSUME DS:DGROUP
   ASSUME CS:_TEXT

; void far scheduleEntry(void);
;
; This is the C callable entry point to the scheduler

_scheduleEntry PROC FAR
   PUSH BP          ; Must preserve BP...
   PUSHF            ; Generate an interrupt stack frame for IRET
   PUSH CS
   PUSHF            ; Turn off trap flag
   MOV BP,SP
   AND [BP],0FEFFh
   POPF
   CALL schedEnter  ; 'Push IP'
   POP BP
   RET

schedEnter:
   PUSHAXREGS       ; Save CPU context

   MOV DX,SS
   MOV AX,SP        ; save current stack pointer in DX:AX

; 'newcontext=schedule(int irq, void far *oldcontext);'
   PUSH DX
   PUSH AX
   MOV AX,-1
   PUSH AX
   CALL _schedule   ; (returns new stack pointer in DX:AX)
   ADD SP,06

; Swap back to new context as specified in function return value
schedExit:
   MOV SS,DX       ; switch to new (or the same) stack
   MOV SP,AX

   RESTOREREGS     ; Restore context from stack
   IRET

_scheduleEntry ENDP

; void far beginScheduler(void far *context);
;
; Direct function call interface to start scheduler, NOTE: This should
; only be used by the kernel to start/restart the scheduler.
;
_beginScheduler PROC FAR
   PUSH BP
   MOV BP,SP

; Get initial task context from function argument and 'jump' to that context
   MOV AX,[BP+06]
   MOV DX,[BP+08]
   JMP short schedExit

_beginScheduler ENDP

; TASKEXIT - Function to place the task return value on the task
; stack and call taskExit(int exitStatus)
;
TASKEXIT PROC FAR
   PUSH AX
   CALL _taskExit        ; Should NEVER return!!

TASKEXIT ENDP

_TEXT ENDS

END

