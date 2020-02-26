; LiteTest.asm - Test secondary loader program, displays a message, waits
; for a key press, then reboots the PC...

_TEXT SEGMENT PUBLIC WORD 'CODE'
   ASSUME CS:_TEXT,DS:_TEXT

ORG 100h                                    ; It's a .COM file

; display large text message
entry:
   PUSH CS
   POP DS
   MOV SI,offset msg
   CALL message

; now wait for a key press
   XOR AX,AX
   INT 16h

; Reboot using INT 19h
   MOV SI, offset boot
   CALL message
   INT 19h

; message - display an ASCIIZ string on the screen (from DS:SI)
message:
   LODSB
   CMP AL,0
   JZ SHORT message1
   MOV AH,0Eh
   XOR BX,BX
   MOV BL,7
   INT 10h
   JMP SHORT message
message1:
   RET


; Use up a few sectors of the disk (just for fun!)
space DB 16384 dup(?)

msg DB 'LiteTest - a test program for the primary loader BOOTUP.BIN, which',0dh,0ah
    DB 'would normally load the Real secondary loader, and this in turn',0dh,0ah
    DB 'searches for a kernel file, loads it (via the FAT file system)',0dh,0ah
    DB 'and starts the (AshbySoft *) LiteTask multi-tasking kernel',0dh,0ah
    DB 0dh,0ah,'Press any key to reboot this PC: ',0

boot DB 'Rebooting..',0

_TEXT ENDS

END entry
