; kbdio.asm - _kbdgetc, _kbdint

KBD	equ	16H		; keyboard request interrupt
KBDGETC	equ	0			; get the character
KBDPEND	equ	1			; check for character pending
NOCH	equ	-1		; no character
SPEC	equ	100H		; special character offset

	include dos.asm

	dseg
; dummy data segment
	endds

	pseg

	public	_kbdgetc
	public	_kbdint
	extrn	pcxflag:word

;------------------------------------------------------------------------
; _kbdgetc  --  get a character from the BIOS keyboard buffer
;------------------------------------------------------------------------
; int kbdgetc()
_kbdgetc	proc	near
	pushf				; push the flags
	push	si
	push	di			; save registers
	cli				; disable interrupts
	xor	ax,ax			; to defer rescheduling, ...
	xchg	ax,cs:pcxflag		; ... get and clear pcxflag ...
	push	ax			; ... and save for later
	mov	ah,KBDPEND		; get keyboard status first
	int	KBD
	jnz	getc1			; character there?
	mov	ax,NOCH			; if not, send the info back
	jmp	short getc9
getc1:	mov	ah,KBDGETC		; if so, actually get the char
	int	KBD
	or	al,al			; check the lower byte
	je	getc2			; is it a non-ASCII special?
	xor	ah,ah			; if not, just send the lower byte
	jmp	short getc9
getc2:	mov	al,ah			; move scan code to lower byte
	xor	ah,ah			; clear out upper byte
	add	ax,SPEC			; add special offset
getc9:	pop	cs:pcxflag		; restore pcxflag
	pop	di
	pop	si			; restore registers
	popf
	ret
_kbdgetc	endp

;-------------------------------------------------------------------------
; _kbdint  --  general access to KBD interrupt; returns flags
;------------------------------------------------------------------------
; int kbdint(r)
; union REGS *r;
_kbdint	proc	near
	push	bp
	mov	bp,sp			; follows C calling conventions
	pushf				; save flags
	push	si
	push	di			; save registers
	mov	si,[bp+4]		; get pointer to register structure
	mov	ax,[si]
	mov	bx,[si+2]
	mov	cx,[si+4]
	mov	dx,[si+6]		; set up registers for call
	push	si
	int	KBD			; call the KBD BIOS interrupt
	pop	si			; recover register pointer
	pushf				; save flags for return value
	mov	[si],ax
	mov	[si+2],bx
	mov	[si+4],cx
	mov	[si+6],dx		; return registers
	pop	ax			; return flags value
	pop	di
	pop	si			; restore registers
	popf				; and the flags
	pop	bp
	ret
_kbdint	endp

	endps

	end
