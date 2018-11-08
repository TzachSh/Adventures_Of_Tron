; intmap.asm - 
; low-level interrupt transfer table and dispatcher

	include	dos.asm

tblsize	equ	20h			; define max size of intmap table
stksize	equ	100h			; max size of system stack

	dseg

	public	_sys_imp

intstack	db	stksize dup (?)	; interrupt stack
topstack	label	byte
_sys_imp	dd	far ptr intmap

	endds

	pseg

	public	pcxflag, sssave, spsave

pcxflag		dw	1		; zero when rescheduling disabled
spsave		dw	?		; saved stack pointer register
sssave		dw	?		; saved stack segment register

;-------------------------------------------------------------------------
; intmap  --  interrupt dispatch table
;-------------------------------------------------------------------------
intmap	label	byte

	rept	tblsize

	db	?			; ivec   - interrupt vector number
	call	intcom
	dd	?			; oldisr - old isr from bios (seg:off)
	dw	-1			; newisr - new isr code address
	dw	?			; mdevno - minor device number
	dw	?			; iflag  - interrupt flag

	endm

	ASSUME DS:NOTHING

;-------------------------------------------------------------------------
; intcom  --  common interrupt dispatcher
;-------------------------------------------------------------------------
; This procedure is interupt handling code that is common to all interupt
; service routines.
intcom	proc	near
	push	bp
	mov	bp,sp
	push	ax			; push registers
	push	bx
	mov	bx,[bp+2]		; get pointer to intmap data
	mov	ax,cs:[bx+8]		; get interrupt flag
	cmp	ax,0			; zero?
	je	short nobios  		; yes, skip the call to the BIOS
	pushf				; push flags to simulate interrupt
	call	cs:dword ptr[bx]	; call BIOS ISR
	cli				; be *sure* interrupts are back off
nobios:
	push	cx			; save rest of registers
	push	dx
	push	si
	push	di
	push	ds
	push	es
	mov	cs:sssave,ss		; save stack environment
	mov	cs:spsave,sp
	mov	cx,cs			; get code segment
; bp+6 points to code segment where interrupt occurred
	cmp	cx,[bp+6]		; check if we own interrupt
	jne	short newstack
; time to do our ISR, since the stack and data segments are OK
	push	cs:word ptr[bx+6]	; pass minor dev. no.
	call	cs:word ptr[bx+4]	; call C ISR (saves si, di)
	add	sp,2			; deallocate parameter (C convention)
	jmp	short popregs
newstack:
; now set up temporary stack in DGROUP and do our ISR
	mov	ax,DGROUP
	mov	ds,ax			; set ds to DGROUP
	ASSUME	DS:DGROUP
	mov	ss,ax			; and set up temporary stack in DGROUP
	mov	sp,offset topstack
	xor	ax,ax			; clear pcxflag to prevent reschedules
	xchg	ax,cs:pcxflag
	push	ax			; save for later
	push	cs:word ptr[bx+6]	; pass minor dev. no.
	call	cs:word ptr[bx+4]	; call our routine (saves si, di)
	add	sp,2			; deallocate parameter (C convention)
	pop	cs:pcxflag		; restore pcxflag
	mov	ss,cs:sssave		; restore old stack
	mov	sp,cs:spsave
	ASSUME	DS:NOTHING
popregs:
	pop	es			; restore all registers
	pop	ds
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	bp
	add	sp,2			; remove pointer to intmap area
	iret
intcom	endp

	endps

	end
