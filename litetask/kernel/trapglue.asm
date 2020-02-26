; TRAPGLUE.ASM - Assembly wrappers for TRAP.C functions

; $Author:   Phlash  $
; $Date:   24 Jun 1995 20:24:42  $
; $Revision:   1.0  $

INCLUDE KERNEL.INC

DGROUP GROUP _DATA

_DATA SEGMENT PUBLIC WORD 'DATA'
_DATA ENDS

_TEXT SEGMENT PUBLIC WORD 'CODE'

INTFUNCTION _divZeroTrap, _divZero
INTFUNCTION _oneStepTrap, _oneStep
INTFUNCTION _nmiIntTrap, _nmiInt
INTFUNCTION _breakPointTrap, _breakPoint
INTFUNCTION _overFlowTrap, _overFlow
INTFUNCTION _breakKeyTrap, _breakKey

_TEXT ENDS

END

; End
