; FILE:   MOUSE.ASM
; $Revision:   1.2  $
; $Author:   Philip Ashby  $
; $Date:   28 Sep 1990 10:21:56  $

			NAME MOUSE

; * An interface function set between High level languages (eg: 'C')
; * and the Microsoft mouse driver.

; Notes
; ~~~~~
; * (1) APPLICATION INTERFACE:

; * 'C' prototypes:
; *     int far cdecl callmouse(int far * ax, int far * bx,
; *                              int far * cx, void far * dx);
; *
; * Parameters: ax, bx, cx, dx - are pointers to integers which correspond
; *             directly to the machine registers used in INT 33h call
; *             to mouse driver (see any MS-DOS Guide for details).
; *
; * Return value: is the value of the AX register on return from INT 33h,
; *               or, 0xFFFF (-1) if an error has occured (invalid paramters).
; *
; * NB: the final paramter ('dx') is declared 'void far *' because it can
; *     be used to pass the address of a data pointer, or function pointer,
; *     as well as a pointer to an integer (see next section).

; *  Registers affected: AX, BX, CX, DX, ES


; *  *****
; *     void far cdecl exclude_zone(int x1, int y1, int x2, int y2);
; *
; * Parameters: x1, y1, x2, y2- are integers which define the exclusion zone
; *             for the mouse cursor.
; *             To cancel this exclusion zone, use 'callmouse()' with *ax = 1.
; *
; * Return value: none
; *
; * Registers affected: AX, CX, DX


; * (2) VARIANCES FROM INT 33h SPECIFICATION:

; * When the INT 33h specification uses 'ES:DX', then this code assumes that
; * paramter 4 ('dx') points to suitable data pointer, or function pointer.

; * The function 1Fh (31) 'Disable mouse driver' specifies that the previous
; * INT 33h handler address is returned in ES:BX, This code however, stores
; * the address in the variable pointed to by parameter 4 ('dx'), NOT paramter
; * 2 ('bx').

; ****************************************************************************

; Public definitions

	PUBLIC _callmouse, _event_trap, __QCEnsScrSwap
	PUBLIC _exclude_zone, __gettextposition

	EXTRN   _event_handler:far, $_gettextposition:far

; Data Segment..
; ~~~~~~~~~~~~

_DATA SEGMENT WORD PUBLIC 'DATA'

extend	 DB 0                 ;Flag byte for extended indicator.
old_off  DW 0                 ;Address to return to after trapping graphics call.
old_seg  DW 0

_DATA ENDS

; Code Segments..
; ~~~~~~~~~~~~

mouse_code SEGMENT WORD PUBLIC 'CODE'
	ASSUME CS:mouse_code, DS:_DATA, ES:nothing

__gettextposition PROC FAR  ;Proceedure to fit between application / graphics
	PUSH BP             ;Required because $_gettextposition does not PUSH BP
	MOV BP, SP
	CALL $_gettextposition
	MOV SP, BP
	POP BP
	RET
__gettextposition ENDP

set_extend PROC NEAR          ;Proceedure for setting extend flag to AL
	MOV extend, AL
	RET
set_extend ENDP

read_extend PROC NEAR         ;Proccedure to read extend flag into AL
	MOV AL, extend
	RET
read_extend ENDP

_callmouse PROC FAR           ;Proceedure to interface to mouse driver
	PUSH BP               ;Standard 'C' function startup code
	MOV BP,SP

; Initialise data
	XOR AX,AX
	CALL set_extend       ;Clear extend flag
	PUSH DS               ;Save DS register
	PUSH ES               ;Save program state
	PUSH SI
	PUSH DI

; Check for extended 4th parameter (pointer to other types)
	MOV AX, [BP+08h]      ;Load segment address into DS
	MOV DS, AX
	MOV SI, [BP+06h]      ;Load offset address into SI
	MOV AX, [SI]          ;Load funtion number into AX
	CMP AX, 24h           ;Check for function out of range..
	JBE SHORT param_OK    ;OK.. or
	JMP param_error       ;OOPS !
param_OK:
	CMP AX, 09h           ;Set graphics cursor shape
	JE SHORT extend_in
	CMP AX, 0Ch           ;Set user_defined event handler
	JE SHORT extend_in
	CMP AX, 14h           ;Swap user_defined event handlers
	JE SHORT extend_out  ;Set extend_out flag as well as extend_in
	CMP AX, 16h           ;Save mouse driver state
	JE SHORT extend_in
	CMP AX, 17h           ;Restore mouse driver state
	JE SHORT extend_in
	CMP AX, 18h           ;Set alternate event handler (ALT/SHIFT/CTRL+..)
	JE SHORT extend_in
	CMP AX, 19h           ;Get address of alternate event handler
	JE SHORT extend_out   ;Set extend_out flag as well as extend_in
	CMP AX, 1Fh           ;Disable mouse driver (see Notes)
	JNE SHORT call_driver ;Skip if not this..
	MOV AL, 2             ;Special flag value for later
	CALL set_extend
	JMP SHORT call_driver ;Skip to normal call

extend_out:
	MOV AL, 01h
	CALL set_extend       ;Indicate extended return info

extend_in:
	MOV AX, [BP+14h]      ;Load segment address into DS
	MOV DS, AX
	MOV SI, [BP+12h]      ;Load offset address into SI
	MOV AX, [SI+02h]      ;Load segment data into ES
	MOV ES, AX            ;For extended call data

call_driver:
	MOV AX, [BP+14h]      ;Load segment address into DS
	MOV DS, AX
	MOV SI, [BP+12h]      ;Load offset address into SI
	MOV DX, [SI]          ;Load data pointed to into DX
	MOV AX, [BP+10h]      ;Load segment address into DS
	MOV DS, AX
	MOV SI, [BP+0Eh]      ;Load offset address into SI
	MOV CX, [SI]          ;Load data pointed to into CX
	MOV AX, [BP+0Ch]      ;Load segment address into DS
	MOV DS, AX
	MOV SI, [BP+0Ah]      ;Load offset address into SI
	MOV BX, [SI]          ;Load data pointed to into BX
	MOV AX, [BP+08h]      ;Load segment address into DS
	MOV DS, AX
	MOV SI, [BP+06h]      ;Load offset address into SI
	MOV AX, [SI]          ;Load data pointed to into AX
	INT 33h               ;Call mouse driver..

; Restore data from call..
	PUSH AX               ;Save AX for later
	MOV AX, [BP+14h]      ;Load segment address into DS
	MOV DS, AX
	MOV SI, [BP+12h]      ;Load offset address into SI
	MOV [SI], DX          ;Load DX into data pointed to
	MOV AX, [BP+10h]      ;Load segment address into DS
	MOV DS, AX
	MOV SI, [BP+0Eh]      ;Load offset address into SI
	MOV [SI], CX          ;Load CX into data pointed to
	MOV AX, [BP+0Ch]      ;Load segment address into DS
	MOV DS, AX
	MOV SI, [BP+0Ah]      ;Load offset address into SI
	MOV [SI], BX          ;Load BX into data pointed to
	MOV AX, [BP+08h]      ;Load segment address into DS
	MOV DS, AX
	MOV SI, [BP+06h]      ;Load offset address into SI
	POP AX                ;Restore saved value of AX
	MOV [SI], AX          ;Load AX into data pointed to
	MOV CX, AX            ;temporary store for AX..

;Check for extended data returned..
	CALL read_extend
	CMP AL ,0
	MOV AX, CX            ;Default return value is AX, from call
	JE SHORT no_extend
	CMP AL, 2             ;Check for Function 1Fh..
	JNE SHORT normal_extend
	MOV AX, [BP+14h]      ;Load segment address into DS
	MOV DS, AX
	MOV SI, [BP+12h]      ;Load offset address into SI
	MOV [SI], BX          ;Load BX into DX data pointer

normal_extend:
	MOV AX, [BP+14h]      ;Load segment address into DS
	MOV DS, AX
	MOV SI, [BP+12h]      ;Load offset address into SI
	MOV AX, ES            ;Load ES into extended segment pointer
	MOV [SI+02h], AX
	MOV AX, CX            ;use default return value from call

no_extend:
	POP DI                ;Restore program state
	POP SI
	POP ES
	POP DS                ;restore old Data segment
	MOV SP, BP            ;Standard 'C' exit code
	POP BP
	RET

param_error:
	MOV AX, 0FFFFh        ;Indicate unsuccessful call.
	JMP SHORT no_extend

_callmouse ENDP

_exclude_zone PROC FAR        ;A proceeedure to set the exclusion zone
	PUSH BP               ;Standard 'C' function startup code
	MOV BP,SP

; Initialise data
	PUSH DS               ;Save DS register
	PUSH SI               ;Save program state
	PUSH DI

	MOV AX, 10h           ;'Set exclusion zone'
	MOV CX, [BP+06h]      ;Load registers with co-ords
	MOV DX, [BP+08h]
	MOV SI, [BP+0Ah]
	MOV DI, [BP+0Ch]
	INT 33h               ;Call mouse driver..

	POP DI                ;Restore program state
	POP SI
	POP DS                ;restore old Data segment
	MOV SP, BP            ;Standard 'C' exit code
	POP BP
	RET

_exclude_zone ENDP

_event_trap PROC FAR          ;Proceedure to call 'C' interrupt type
	PUSHF                 ;function from mouse driver call back.
	CALL _event_handler
	RET
_event_trap ENDP

__QCEnsScrSwap PROC FAR       ;Proceedure to replace graphics.lib version
	PUSH ES
	PUSH DX
	PUSH AX
; Now swap address on stack with old_addr..
; NB THIS ASSUMES THAT BP has been set by graphics function on entry !!
	LES DX, [BP+02h]      ;Grab return address
	MOV AX, ES
	MOV old_off,DX
	MOV old_seg,AX
	MOV [BP+04h], mouse_code ;Replace with my trap function
	MOV [BP+02h], offset enable_mouse
;Turn off pointer
	MOV AX, 2		 ;Turn pointer off
	INT 33h
	POP AX                   ; Restore and return
	POP DX
	POP ES
	RET
__QCEnsScrSwap ENDP

enable_mouse PROC FAR        ;Proceedure to re_enable mouse and return to
                             ;trapped program return address
	PUSH AX              ;Dummy return address for me to fill in !!
	PUSH AX

	PUSH BP
	MOV BP, SP
	PUSH ES
	PUSH DX
	PUSH AX
	LES DX,DWORD PTR old_off
	MOV [BP+02],DX        ;Put old return address back on stack
	MOV [BP+04],ES
	MOV AX, 01            ;Enable mouse
	INT 33h               ;Call driver
	POP AX                ;Restore registers
	POP DX
	POP ES
	MOV SP, BP
	POP BP
	RET                ;Return to trapped address
enable_mouse ENDP

mouse_code ENDS

END
