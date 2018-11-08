; ctxsw.asm - _ctxsw

	include	dos.asm

	dseg
; null data segment
	endds

	pseg

	public	_ctxsw

;-------------------------------------------------------------------------
; _ctxsw  --  context switch
;-------------------------------------------------------------------------
; void ctxsw(opp,npp)
; char *opp, *npp;
;---------------------------------------------------------------------
; Stack contents upon entry to ctxsw:
;	SP+4 => address of new context stack save area
;       SP+2 => address of old context stack save area
;       SP   => return address
; The addresses of the old and new context stack save areas are
; relative to the DS segment register, which must be set properly 
; to access the save/restore locations.
;
; The saved state consists of the current BP, SI and DI registers,
; and the FLAGS register
;---------------------------------------------------------------------
_ctxsw	proc	near
	push	bp
	mov	bp,sp		; frame pointer
	pushf			; flags save interrupt condition
	cli			; disable interrupts just to be sure
	push	si
	push	di		; preserve registers
	mov	bx,[bp+4]	; old stack save address
	mov	[bx],sp
	mov	bx,[bp+6]	; new stack save address
	mov	sp,[bx]
	pop	di
	pop	si
	popf			; restore interrupt state
	pop	bp
	ret
_ctxsw	endp

	endps

	end
