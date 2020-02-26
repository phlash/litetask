; BIOSLIB.ASM - BIOS level text I/O functions for startup / emergencies

PUBLIC _biosCh
PUBLIC _biosStr
PUBLIC _biosKey

_TEXT SEGMENT PUBLIC WORD 'CODE'
      ASSUME CS:_TEXT

_biosCh  proc far
      push bp
      mov bp,sp
      mov ah,0Eh
      mov al,[bp+6]
      mov bh,0
      mov bl,7
      int 10h
      pop bp
      ret
_biosCh  endp

_biosStr proc far
      push bp
      mov bp,sp
      push si
      push ds
      lds si,[bp+6]
      mov ah,0Eh
      mov bh,0
      mov bl,7
bsNext:
      lodsb
      cmp al,0
      je short bsDone
      int 10h
      jmp short bsNext
bsDone:
      pop ds
      pop si
      pop bp
      ret
_biosStr endp

_biosKey proc far
      push bp
      mov bp,sp
      mov ax,[bp+6]
      mov bx,[bp+8]
      cmp ah,1h
      jz short bkTest
      cmp ah,11h
      jz short bkTest
      int 16h
      pop bp
      ret
bkTest:
      int 16h
      jnz short bkDone
      xor ax,ax
bkDone:
      pop bp
      ret
_biosKey endp

_TEXT ENDS

END
