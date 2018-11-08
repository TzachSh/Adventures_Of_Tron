; dskio.asm - _dskio, _dskint

	include	dos.asm

DSK	equ	13H			; disk i/o BIOS function

	dseg
; null data segment
	endds

	pseg

	public	_dskio
	public	_dskint

;-------------------------------------------------------------------------
; _dskio  --  perform disk read/write operation
;-------------------------------------------------------------------------
; int dskio(op,buf,drive,cyl,surf,sect)
; int op;			/* 2=read, 3=write			*/
; char *buf;			/* transfer address			*/
; int drive;			/* disk drive number			*/
; int cyl,surf,sect;		/* (cylinder,surface,sector) disk addr	*/
_dskio	proc	near
	push	bp			; set up the stack frame
	mov	bp,sp			; stack frame pointer
	pushf				; push the flags
	push	si
	push	di			; save registers
	mov	ah,[bp+4]		; operation code in ah
	mov	bx,[bp+6]		; buffer pointer in bx
	mov	dl,[bp+8]		; drive number in dl
	mov	ch,[bp+10]		; cylinder number in ch
	mov	dh,[bp+12]		; surface in dh
	mov	cl,[bp+14]		; sector number in cl
	mov	al,1	 		; transfer one block
	push	ds
	pop	es			; set es to our data segment
	int	DSK			; call the DSK BIOS interrupt
	mov	al,ah			; error return in al
	xor	ah,ah			; clear upper byte
	pop	di
	pop	si			; restore registers
	popf				; restore the flags
	pop	bp
	ret
_dskio	endp

;-------------------------------------------------------------------------
; _dskint  --  general access to DSK interrupt; returns flags
;-------------------------------------------------------------------------
; int dskint(r)
; union REGS *r;
_dskint	proc	near
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
	push	ds
	pop	es			; set es to our data segment
	push	si
	int	DSK			; call the DSK BIOS interrupt
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
_dskint	endp

	endps

	end
