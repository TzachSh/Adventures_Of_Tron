;---------------------------------------------------------------------
; dos.asm
; macros for assembly-language interface to Microsoft C
; support small memory model only
;---------------------------------------------------------------------

;=====================================================================
dseg	macro
DGROUP	group	_DATA
_DATA	segment	word public 'DATA'
	assume ds:DGROUP
	endm
;=====================================================================
endds	macro
_DATA	ends
	endm
;=====================================================================
pseg	macro
_TEXT	segment byte public 'CODE'
	assume cs:_TEXT, ds:DGROUP
	endm
;=====================================================================
endps	macro
_TEXT	ends
	endm
;=====================================================================
