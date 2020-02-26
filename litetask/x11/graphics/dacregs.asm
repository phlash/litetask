;Routines for setting an individual DAC register, or the complete set.

INPUT_STATUS_1         equ 03dah
DAC_WRITE_INDEX        equ 03c8h
DAC_DATA               equ 03c9h

PUBLIC _setDACregister
PUBLIC _setDACregisters

EXTRN _lockInts:far
EXTRN _unlockInts:far

_TEXT SEGMENT PUBLIC WORD 'CODE'
ASSUME CS:_TEXT

;This function syncronises with the vertical refresh of the display

WaitVsync proc near
   mov   dx,INPUT_STATUS_1
WaitforNotVSync:
   in    al,dx
   and   al,08h
   jnz   WaitforNotVSync
WaitforVSync:
   in    al,dx
   and   al,08h
   jz    WaitforVSync
   ret
WaitVsync endp

;This sets the given DAC register to the given colour
;Takes two parameter, the register offset and the RGB value (in a long)
_setDACregister proc far
   push  bp
   mov   bp,sp
;Wait for vertical refresh (otherwise we cause snow on the display)
   call  WaitVsync
   call  _lockInts      ;don't want interruptions now, we ain't got time..
   mov   cx,ax          ;keep return value for later
   mov   dx,DAC_WRITE_INDEX
   mov   ax,[bp+6]
   out   dx,al
   mov   ax,[bp+8]      ;bx:ax = RGB value
   mov   bx,[bp+10]
   mov   dx,DAC_DATA
   out   dx,al
   mov   al,ah
   out   dx,al
   mov   al,bl
   out   dx,al
   push  cx
   call  _unlockInts    ;interrupts allowed again..
   add   sp,2
   pop   bp
   ret
_setDACregister endp

;This accesses the hardware directly to change the palette
;Takes three parameter, the address of the new palette, the start register
;and the number of registers to change
_setDACregisters proc far
   push  bp
   mov   bp,sp
   push  ds
   push  si
   push  di
;Wait for vertical refresh (otherwise we cause snow on the display)
   call  WaitVsync
;Store parameters into local registers for speed
   lds   si,[bp+6]
   mov   bh,[bp+10]
   mov   cx,[bp+12]
DACLoadLoop:
   call  _lockInts            ;Don't stop me now (that's a song isn't it?)
   mov   di,ax                ;Preserve interrupt state in DI
   mov   dx,DAC_WRITE_INDEX   ;Select the register to change
   mov   al,bh
   out   dx,al
   mov   dx,DAC_DATA          ;Now change it (hur! hur!)
   lodsb
   out   dx,al
   lodsb
   out   dx,al
   lodsb
   out   dx,al
   lodsb          ;Strip redundant byte from array of four byte color values
   push  di                   ;Restore interrupt state
   call  _unlockInts          ;Allow interrupts while we loop..
   add   sp,2
   inc   bh
   loop  DACLoadLoop          ;Do specified quantity of registers
   pop   di
   pop   si
   pop   ds
   pop   bp
   ret
_setDACregisters endp

_TEXT ENDS

END
