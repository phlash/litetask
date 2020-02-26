; FASTCIRC.ASM
;
ISVGA equ 0

.model small
.code

GenerateOctantParms  struc
        dw     ?                  ; pushed bp
        dw     ?                  ; return address
PixList dw     ?
MinorAdjust dd ?
Threshold dd   ?
MajorSquared dd ?
MinorSquared dd ?

GenerateOctantParms  ends

PixelCount equ -2
MajorAdjust equ -6
MajorSquaredTimes2 equ -10
MinorSquaredTimes2 equ -14

   public _GenerateEOctant
_GenerateEOctant   proc  near
   push bp
   mov bp,sp
   add sp,MinorSquaredTimes2
   push si
   push di

   mov word ptr [PixelCount+bp],0
   mov ax,word ptr [MajorSquared+bp]
   shl ax,1
   mov word ptr [MajorSquaredTimes2+bp],ax
   mov ax,word ptr [MajorSquared+bp+2]
   rcl ax,1
   mov word ptr [MajorSquaredTimes2+bp+2],ax

   mov ax,word ptr [MinorSquared+bp]
   shl ax,1
   mov word ptr [MinorSquaredTimes2+bp],ax
   mov ax,word ptr [MinorSquared+bp+2]
   rcl ax,1
   mov word ptr [MinorSquaredTimes2+bp+2],ax

   mov di,[PixList+bp]

   sub cx,cx
   mov si,cx
   mov bx,word ptr [Threshold+bp]
   mov dx,word ptr [Threshold+bp+2]

GenLoop:
   add bx,cx
   adc dx,si
   add bx,word ptr [MinorSquared+bp]
   adc dx,word ptr [MinorSquared+bp+2]

   mov byte ptr [di],0
   js MoveMajor

   mov ax,word ptr [MajorSquaredTimes2+bp]
   sub word ptr [MinorAdjust+bp],ax
   mov ax,word ptr [MajorSquaredTimes2+bp+2]
   sbb word ptr [MinorAdjust+bp+2],ax
   sub bx,word ptr [MinorAdjust+bp]
   sbb dx,word ptr [MinorAdjust+bp+2]
   mov byte ptr [di],1

MoveMajor:
   inc di
   inc word ptr [PixelCount+bp]

   add cx,word ptr [MinorSquaredtimes2+bp]
   adc si,word ptr [MinorSquaredTimes2+bp+2]

   cmp si,word ptr [MinorAdjust+bp+2]
   ja Done
   jb GenLoop
   cmp cx,word ptr [MinorAdjust+bp]
   jb GenLoop

Done:
   mov ax,[PixelCount+bp]
   pop di
   pop si
   mov sp,bp
   pop bp
   ret
_GenerateEOctant   endp

DrawParms         struc
         dw       ?       ;pushed bp
         dw       ?       ;return address
X        dw       ?
Y        dw       ?
DrawLength dw     ?
RowOffset  dw     ?
HorizMoveDir dw   ?
DrawList dw       ?

DrawParms         ends

SCREEN_SEGMENT        equ 0a000h
SCREEN_WIDTH_IN_BYTES equ 80
GC_INDEX              equ 3ceh

   public _DrawVOctant

_DrawVOctant   proc  near
   push bp
   mov bp,sp
   push si
   push di
   mov ax,SCREEN_SEGMENT
   mov es,ax
   mov ax,SCREEN_WIDTH_IN_BYTES
   mul [Y+bp]
   mov di,[X+bp]
   mov cx,di
   shr di,1
   shr di,1
   shr di,1
   add di,ax
   and cl,07h
if ISVGA
   mov ah,80h
   shr ah,cl
   cld
else
   mov al,80h
   shr al,cl
   mov dx,GC_INDEX+1
endif
   mov si,[DrawList+bp]
   sub bx,bx
   mov cx,[DrawLength+bp]
   jcxz VDrawDone
   cmp [HorizMoveDir+bp],0
   mov bp,[RowOffset+bp]
   jz VGoLeft

VDrawRightLoop:
if ISVGA
   and es:[di],ah
   lodsb
   and al,al
   jz VAdvanceOneLineRight
   ror ah,1
else
   out dx,al
   and es:[di],al
   cmp [si],bl
   jz VAdvanceOneLineRight
   ror al,1
endif
   adc di,bx

VAdvanceOneLineRight:
ife ISVGA
   inc si
endif
   add di,bp
   loop VDrawRightLoop
   jmp short VDrawDone

VGoLeft:
VDrawLeftLoop:
if ISVGA
   and es:[di],ah
   lodsb
   and al,al
   jz VAdvanceOneLineLeft
   rol ah,1
else
   out dx,al
   and es:[di],al
   cmp [si],bl
   jz VAdvanceOneLineLeft
   rol al,1
endif
   sbb di,bx

VAdvanceOneLineLeft:
ife ISVGA
   inc si
endif
   add di,bp
   loop VDrawLeftLoop

VDrawDone:
   pop di
   pop si
   pop bp
   ret

_DrawVOctant   endp

   public _DrawHOctant

_DrawHOctant   proc  near
   push bp
   mov bp,sp
   push si
   push di
   mov ax,SCREEN_SEGMENT
   mov es,ax
   mov ax,SCREEN_WIDTH_IN_BYTES
   mul [Y+bp]
   mov di,[X+bp]
   mov cx,di
   shr di,1
   shr di,1
   shr di,1
   add di,ax
   and cl,07h
   mov bh,80h
   shr bh,cl
if ISVGA
   cld
else
   mov dx,GC_INDEX+1
endif
   mov si,[DrawList+bp]
   sub bl,bl
   mov cx,[DrawLength+bp]
   jcxz HDrawDone
if ISVGA
   sub ah,ah
else
   sub al,al
endif
   cmp [HorizMoveDir+bp],0
   mov bp,[RowOffset+bp]
   jz HGoLeft

HDrawRightLoop:
if ISVGA
   or ah,bh
   lodsb
   and al,al
else
   or al,bh
   cmp [si],bl
endif
   jz HAdvanceOneLineRight

if ISVGA
   and es:[di],ah
   sub ah,ah
else
   out dx,al
   and es:[di],al
   sub al,al
endif
   add di,bp

HAdvanceOneLineRight:
ife ISVGA
   inc si
endif
   ror bh,1
   jnc HDrawLoopRightBottom

if ISVGA
   and es:[di],ah
   sub ah,ah
else
   out dx,al
   and es:[di],al
   sub al,al
endif
   inc di

HDrawLoopRightBottom:
   loop HDrawRightLoop
   jmp short HDrawDone

HGoLeft:
HDrawLeftLoop:
if ISVGA
   or ah,bh
   lodsb
   and al,al
else
   or al,bh
   cmp [si],bl
endif
   jz HAdvanceOneLineLeft

if ISVGA
   and es:[di],ah
   sub ah,ah
else
   out dx,al
   and es:[di],al
   sub al,al
endif
   add di,bp

HAdvanceOneLineLeft:
ife ISVGA
   inc si
endif
   rol bh,1
   jnc HDrawLoopLeftBottom

if ISVGA
   and es:[di],ah
   sub ah,ah
else
   out dx,al
   and es:[di],al
   sub al,al
endif
   dec di

HDrawLoopLeftBottom:
   loop HDrawLeftLoop

HDrawDone:
if ISVGA
   and es:[di],ah
else
   out dx,al
   and es:[di],al
endif
   pop di
   pop si
   pop bp
   ret

_DrawHOctant   endp

end

