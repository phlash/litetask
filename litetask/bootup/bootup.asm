; BOOTUP.ASM - Disk bootstrap routine

; Segment address of load point, and relocation segment
LOADPNT    equ 7C0h
RELOCATE   equ 9000h

_TEXT SEGMENT PUBLIC PARA 'CODE'
   ASSUME CS:_TEXT,DS:_TEXT

ORG 0h

entry:
   JMP SHORT start
   NOP

; OEM Identification
OEM_id              db 'LTsk 0.1'

; Define locations of BPB variables (these will be merged in by boot_sec.exe)
BytesPerSector      dw ?
SectorsPerCluster   db ?
ReservedSectors     dw ?
nFATS               db ?
RootDirSize         dw ?
TotalSectors        dw ?
MediaByte           db ?
SectorsPerFAT       dw ?
SectorsPerTrack     dw ?
nHeads              dw ?
HiddenSectorsLo     dw ?
HiddenSectorsHi     dw ?

; Set BPB 'volume label' to default kernel file name
ORG 2Bh
KernelName          db 'LITETASKEXE'

; Skip rest of BPB (DOS 4.0+ bits)
ORG 3Eh

; Bootstrap code for real
start:
; Set up a stack (1024 bytes below the relocation point)
   CLI
   MOV AX,RELOCATE - 40h
   MOV SS,AX
   MOV SP,400h
   STI

; Set up segments, DS=load point (here), ES=relocation address 
   MOV AX,LOADPNT
   MOV DS,AX
   MOV AX,RELOCATE
   MOV ES,AX

; Save drive ID byte
   MOV drv,DL

; Relocate this boot sector
   XOR AX,AX
   MOV SI,AX
   MOV DI,AX
   MOV CX,100h
   REPZ MOVSW

; Jump to relocation address
   PUSH ES
   MOV AX,offset relocated
   PUSH AX
   RETF

; Re-set DS register
relocated:
   PUSH CS
   POP DS

; Now display a message on screen
   MOV SI,offset msg
   CALL message

loadit:
; Reset floppy disk drive
   XOR AX,AX
   MOV DL,drv
   INT 13h

; Calculate root dir location ((SectorsPerFAT * nFATS) + HiddenSectors + 1)
   MOV AX,SectorsPerFAT
   SUB DX,DX
   MUL nFATS
   ADD AX,HiddenSectorsLo
   ADC DX,HiddenSectorsHi
   INC AX
   ADC DX,0                   ; DX:AX = Root dir sector number
   MOV SectorLo,AX            ; Save it for later
   MOV SectorHi,DX
   CALL logToPhys             ; Convert to physical values

; Load first sector of root directory at original load point
   MOV AX,LOADPNT
   MOV ES,AX
   MOV AX,0201h
   XOR BX,BX
   MOV DL,drv
   INT 13h
   JC  SHORT nodisk

; Check first entry is valid (ie: it is the file 'loadername')
   MOV SI,offset loadername
   XOR DI,DI
   MOV CX,11
   REPZ CMPSB
   JNZ SHORT noloader

; Calculate start of files area (RootDir + (RootDirSize * 32 / 512))
   MOV AX,RootDirSize
   MOV CX,32
   MUL CX
   MOV CX,512
   DIV CX
   ADD SectorLo,AX
   ADC SectorHi,DX

; Compute number of sectors to load (from directory entry info), store in CX
   MOV AX,ES:[1Ch]     ; File size in bytes (assumes < 64k)
   ADD AX,511          ; Rounding offset
   XOR DX,DX
   DIV CX              ; Still contains 512 from above
   MOV CX,AX           ; Store in CX

; Load required number of sectors directly above this sector
   MOV AX,RELOCATE + 10h
   MOV ES,AX
   MOV BX,100h         ; It's a .COM file :-)
loadsect:   
   PUSH CX             ; Preserve sector count
   MOV AX,SectorLo     ; Convert sector number to physical values
   MOV DX,SectorHi
   CALL logToPhys
   MOV AX,0201h        ; Read one sector at a time
   MOV DL,drv
   INT 13h
   JC SHORT nodisk
   INC SectorLo        ; Next disk sector
   ADC SectorHi,0
   ADD BX,200h         ; Next memory sector
   POP CX
   LOOP loadsect

; Jump to secondary load startup (offset 100h since it's a .COM file)
   PUSH CS             ; Save a return address for error returns
   MOV AX,offset die1
   PUSH AX
   PUSH ES             ; Jump to loader via stack address
   MOV BX,100h
   PUSH BX
   RETF

; =============== Error Traps ================
nodisk:
   MOV SI,offset msg2
   JMP SHORT die

noloader:
   MOV SI,offset msg3
   JMP SHORT die

die:
; Put up messages...
   CALL message
die1:
   MOV SI,offset msgd
   CALL message
; Now, wait for a key, then issue INT 19h (reload)
   XOR AX,AX
   INT 16h
   MOV SI,offset msgb
   CALL message
   INT 19h

; ============== Sub-routines ===============

; logToPhys - calculate physical address of a logical sector (in DX:AX)
; Store appropriately for BIOS Calls

logToPhys:
; Save original values
   PUSH DX
   PUSH AX

; First the cylinder number (stored in CH and top bits of CL)
   PUSH AX
   MOV AX,SectorsPerTrack
   MUL nHeads
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
   DIV SectorsPerTrack
   INC DL
   OR  CL,DL
   XOR DX,DX
   DIV nHeads
   MOV DH,DL
   RET


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

; ===================== Data Area =======================

; Some text messages
msg  DB '(AshbySoft *) Boot loader V 0.1',0Dh,0Ah,0

msg2 DB 0Dh,0Ah,'Error reading disk drive.',0
msg3 DB 0Dh,0Ah,'Not a LiteTask bootable disk.',0
msgd DB ' Press a key to retry: ',0
msgb DB 'Rebooting.',0

; Place holder for drive ID byte
drv  DB ?

; Place holders for a sector number
SectorLo   dw ?
SectorHi   dw ?

ORG 1F0h
; The secondary loader name (at a fixed location for boot_sec.exe to munge!)
loadername DB 'LITELOADCOM'

; All bootsectors MUST have a valid signature at the end!
ORG 1FEh
BootSig  db 55h,0AAh

_TEXT ENDS

END entry
