; eidi.asm - _sys_disabl, _sys_enabl, _sys_restor, _sys_wait, _sys_hlt

	include	dos.asm		; segment macros

	dseg
; null data segment
	endds

	pseg

	public	_sys_disabl,_sys_restor,_sys_enabl,_sys_wait,_sys_hlt

;-------------------------------------------------------------------------
; _sys_disabl  --  return interrupt status and disable interrupts
;-------------------------------------------------------------------------
; int sys_disabl()
_sys_disabl	proc	near
	pushf			; put flag word on the stack
	cli			; disable interrupts!
	pop	ax		; deposit flag word in return register
	ret
_sys_disabl	endp

;-------------------------------------------------------------------------
; _sys_restor  --  restore interrupt status
;-------------------------------------------------------------------------
; void sys_restor(ps)
; int ps;
_sys_restor	proc	near
	push	bp
	mov	bp,sp		; C calling convenion
	push	[bp+4]
	popf			; restore flag word
	pop	bp
	ret
_sys_restor	endp

;-------------------------------------------------------------------------
; _sys_enabl  --  enable interrupts unconditionally
;-------------------------------------------------------------------------
; void sys_enabl()
_sys_enabl	proc	near
	sti			; enable interrupts
	ret
_sys_enabl	endp

;-------------------------------------------------------------------------
; _sys_wait  --  wait for interrupt
;-------------------------------------------------------------------------
; void sys_wait()
_sys_wait	proc	near
	pushf
	sti			; interrupts must be enabled here
	hlt
	popf
	ret
_sys_wait	endp

;-------------------------------------------------------------------------
; _sys_hlt  --  halt the current program and return to host
;-------------------------------------------------------------------------
; void sys_hlt()
_sys_hlt	proc	near
	mov	ah,4ch		; terminate function
	xor	al,al		; OK return code
	int	21h		; MS-DOS function call
	ret
_sys_hlt	endp

	endps

	end
