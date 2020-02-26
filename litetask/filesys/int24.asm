; INT24.ASM - DOS Critical error handler for FAT file system

; This routine always fails the application call

PUBLIC _int24Trap

_TEXT SEGMENT PUBLIC WORD 'CODE'

_int24Trap proc far
   mov al,3
   iret
_int24Trap endp

_TEXT ENDS

end
