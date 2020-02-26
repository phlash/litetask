;This file contains useful graphics functions

CYCLE_SIZE             equ 256
SCREEN_SEGMENT         equ 0a000h
SCREEN_WIDTH_IN_BYTES  equ 320
INPUT_STATUS_1         equ 03dah
DAC_READ_INDEX         equ 03c7h
DAC_WRITE_INDEX        equ 03c8h
DAC_DATA               equ 03c9h


   PUBLIC _setVGAMode
   PUBLIC _BiosSetDACregisters
   PUBLIC _cyclePalette
   PUBLIC _plotPixel
   PUBLIC _ColorTable
   PUBLIC _setDACregisters
   PUBLIC _DrawHorizon

   .model  small
   .data
;Storage for all 256 DAC locations, organized as one three byte RGB triplet
;color.  Actually three 6 bit values, upper two bits of each aren't significant

_ColorTable    label   byte

    db    0,0,0

X=0
    REPT  64
    db    X,X/2,0
X=X+1
    ENDM

X=63
    REPT  64
    db    X,X/2,0
X=X-1
    ENDM
X=0
    REPT  64
    db    X,0,X/2
X=X+1
    ENDM

X=63
    REPT  64
    db    X,0,X/2
X=X-1
    ENDM

;X=0
;    REPT  42
;    db    0,X,63-(X*20)/42
;X=X+1
;    ENDM

;X=41
;    REPT  42
;    db    0,63-(X*20)/42,X
;X=X-1
;    ENDM

    .code

_setVGAMode proc near
;Select VGA's standard 256 color mode, mode 13h
   push  bp
   mov   bp,sp
   mov   ax,[bp+4]
   int   10h
   pop   bp
   ret
_setVGAMode endp

_BiosSetDACregisters proc near
;Now use the BIOS to set the DAC registers to initial value
   push  bp
   mov   bp,sp
   mov   ax,ds
   mov   es,ax
   mov   dx,[bp+4]
   mov   ax,1012h
   sub   bx,bx
   mov   cx,100h
   int   10h
   pop   bp
   ret
_BiosSetDACregisters endp

_cyclePalette proc near
;Rotate colors 1 to 255, location 0 is always left unchanged so that
;background and border don't change
   push  bp
   mov   bp,sp
CycleLoop:
   mov   ax,[bp+8]
WaitLoop:
   dec   ax
   jnz   WaitLoop
   mov   bx,[bp+4]
   push  word ptr [bx+(1*3)]
   push  word ptr [bx+(1*3)+2]
   add   bx,(1*3)
   mov   di,bx
   add   bx,(1*3)
   mov   si,bx
   mov   ax,ds
   mov   es,ax
   mov   cx,254*3/2
   rep   movsw
   pop   bx
   pop   ax
   stosw
   mov   es:[di],bl
   mov   dx,INPUT_STATUS_1
WaitNotVSync2:
   in    al,dx
   and   al,08h
   jnz   WaitNotVSync2
WaitVSync2:
   in    al,dx
   and   al,08h
   jz    WaitVSync2
;Now that we have vertical sync pulse change palette
   mov   cx,[bp+10]
   mov   si,[bp+4]
   sub   ah,ah
DACLoadLoop:
   mov   dx,DAC_WRITE_INDEX
   mov   al,ah
   cli
   out   dx,al
   mov   dx,DAC_DATA
   lodsb
   out   dx,al
   lodsb
   out   dx,al
   lodsb
   out   dx,al
   sti
   inc   ah
   loop  DACLoadLoop
;See if there is a number_of_cycles parameter.  If 0 check for keypress
   mov   ax,[bp+6]
   dec   ax
   js    KeyCheck
   jz    Leave_routine
   mov   [bp+6],ax
   jmp   CycleLoop
;See if a key has been pressed
KeyCheck:
   mov   ah,0bh
   int   21h
   and   al,al
   jz    CycleLoop
;Clear the keypress: Don't really want to do this.
;   mov   ah,1
;   int   21h
Leave_routine:
   pop   bp
   ret
_cyclePalette endp

_plotPixel proc near
;Plot the pixel pointed to by the first two parameters in the color of the
;third parameter.
   push  bp
   mov   bp,sp                       ;Preserve the stack
   mov   ax,SCREEN_SEGMENT           ;Set ES to point to the screen
   mov   es,ax
   mov   bx,[bp+4]                   ;Get the x co-ord
   mov   ax,[bp+6]                   ;Get y co-ord
   mov   dx,SCREEN_WIDTH_IN_BYTES
   mul   dx
   mov   di,ax                       ;point ES:DI to start of line y
   add   di,bx                       ;point to column x
   mov   ax,[bp+8]                   ;Set color 0 to 255
   stosb                             ;Draw the pixel
   pop   bp
   ret
_plotPixel endp

_setDACregisters proc near
;This accesses the hardware directly to change the palette
;Takes only one parameter which is the address of the new palette
   push  bp
   mov   bp,sp
   mov   dx,INPUT_STATUS_1
WaitforNotVSync2:
   in    al,dx
   and   al,08h
   jnz   WaitforNotVSync2
WaitforVSync2:
   in    al,dx
   and   al,08h
   jz    WaitforVSync2
;Now that we have vertical sync pulse change palette
   mov   cx,256
   mov   si,[bp+4]
   sub   ah,ah
nowDACLoadLoop:
   mov   dx,DAC_WRITE_INDEX
   mov   al,ah
   cli
   out   dx,al
   mov   dx,DAC_DATA
   lodsb
   out   dx,al
   lodsb
   out   dx,al
   lodsb
   out   dx,al
   sti
   inc   ah
   loop  nowDACLoadLoop
   pop   bp
   ret
_setDACregisters endp

_DrawHorizon proc near
;draws a horizontal line of length specified by the fourth parameter
   push  bp
   mov   bp,sp                       ;Preserve the stack
   mov   ax,SCREEN_SEGMENT           ;Set ES to point to the screen
   mov   es,ax
   mov   bx,[bp+4]                   ;Get the x co-ord
   mov   ax,[bp+6]                   ;Get y co-ord
   mov   dx,SCREEN_WIDTH_IN_BYTES
   mul   dx
   mov   di,ax                       ;point ES:DI to start of line y
   add   di,bx                       ;point to column x
   mov   cx,[bp+10]                  ;get line length
LineLoop:
   add   ax,cx                       ;draw line backwards
   dec   ax                          ;remember to draw start point not one next to it
   mov   ax,[bp+8]                   ;Set color 0 to 255
   stosb                             ;Draw the pixel
   loop  LineLoop
   pop   bp
   ret
_DrawHorizon endp

   end

