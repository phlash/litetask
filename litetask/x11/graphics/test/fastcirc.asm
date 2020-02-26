; FASTCIRC.ASM
;
ISVGA equ 1

.model small
.code

GenerateOctantParms  struc
        dw     ?                  ; pushed bp
        dw     ?                  ; return address
PixList dw     ?
MajorAxis dw   ?
MinorAxis dw   ?
RadiusSqMinusMajorAxisSq dd ?
MinorAxisSquaredThreshold dd ?

GenerateOctantParms  ends

   public _GenerateOctant
_GenerateOctant   proc  near
   push bp
   mov bp,sp
   push si
   push di

   mov di,[PixList+bp]
   mov ax,[MajorAxis+bp]
   mov bx,[MinorAxis+bp]
   mov cx,word ptr [RadiusSqMinusMajorAxisSq+bp]
   mov dx,word ptr [RadiusSqMinusMajorAxisSq+bp+2]
   mov si,word ptr [MinorAxisSquaredThreshold+bp]
   mov bp,word ptr [MinorAxisSquaredThreshold+bp+2]

GenLoop:
   sub cx,1
   sbb dx,0
   sub cx,ax
   sbb dx,0
   sub cx,ax
   sbb dx,0
   cmp dx,bp
   jb IsMinorMove
   ja NoMinorMove
   cmp cx,si
   ja NoMinorMove

IsMinorMove:
   dec bx
   sub si,bx
   sbb bp,0
   sub si,bx
   sbb bp,0
   mov byte ptr [di],1
   inc di
   inc ax
   cmp ax,bx
   jbe GenLoop
   jmp short Done

NoMinorMove:
   mov byte ptr [di],0
   inc di
   inc ax
   cmp ax,bx
   jbe GenLoop

Done:
   pop di
   pop si
   pop bp
   ret
_GenerateOctant   endp

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

