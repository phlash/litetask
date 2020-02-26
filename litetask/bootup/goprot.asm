; GoProt.asm - Protected mode initialisation routines for 386(tm) DX
;
; NB: This code must be assembled using MASM 5.10 / TASM V2.5 or above

; Using a 386 I believe
.386p

_TEXT SEGMENT PUBLIC WORD 'CODE' USE16
   ASSUME CS:_TEXT

; extern unsigned long loadGdt(unsigned short limit, void far *gdtAddr);
public _loadGdt
_loadGdt proc near
   push bp
   mov bp,sp
   xor eax,eax
   mov ax,[bp+8]
   shl eax,4
   xor ebx,ebx
   mov bx,[bp+6]
   add eax,ebx
   mov [bp+6],eax
   lgdt fword ptr [bp+4]
   sgdt fword ptr [bp+4]
   mov ax,[bp+6]
   mov dx,[bp+8]
   pop bp
   ret
_loadGdt endp

; extern unsigned long loadIdt(unsigned short limit, void far *idtAddr);
public _loadIdt
_loadIdt proc near
   push bp
   mov bp,sp
   xor eax,eax
   mov ax,[bp+8]
   shl eax,4
   xor ebx,ebx
   mov bx,[bp+6]
   add eax,ebx
   mov [bp+6],eax
   lidt fword ptr [bp+4]
   sidt fword ptr [bp+4]
   mov ax,[bp+6]
   mov dx,[bp+8]
   pop bp
   ret
_loadIdt endp

; extern void goProt(ushort cs, ulong eip, ushort ds, ushort ss, ulong esp);
public _goProt
_goProt proc near
   push bp
   mov bp,sp

; First store start address into JMP instruction below
   mov ax,[bp+4]
   mov cs:jseg,ax
   mov ax,[bp+6]
   mov cs:jadd,ax
   mov ax,[bp+8]
   mov cs:jadd+2,ax

; Grab ds & stack values into registers
   mov bx,[bp+10]
   mov cx,[bp+12]
   mov edx,[bp+14]

; Enter protected mode, reset segment regs
   cli
ifdef DEBUG
   mov di,0B800h
   mov es,di
   mov di,154
   mov es:[di],byte ptr '1'
endif
   smsw ax
   or ax,1
   lmsw ax
   jmp short next
next:
   mov ds,bx
   mov es,bx
   mov fs,bx
   mov gs,bx
ifdef DEBUG
   mov edi,0B8000h
   mov [edi+156], byte ptr '2'
endif

; Set up the stack
   mov ss,cx
   mov esp,edx
ifdef DEBUG
   mov [edi+158], byte ptr '3'
endif

; Jump to specified start address (also loads CS with correct selector)
jump DB 066h,0EAh
jadd DW 0,0
jseg DW 0

; Just in case..
halt:
   jmp short halt

_goProt endp

_TEXT ENDS

END
