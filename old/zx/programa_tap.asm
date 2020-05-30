; Generado automáticamente por 'midi2ay'

	org	8000h

inicio	di
	ld	sp,0
	ld	hl,0feffh
	ld	(hl),rsi and 0ffh
	inc	hl
	ld	(hl),rsi shr 8
	ld	a,0feh
	ld	i,a
	im	2
	ei
	jr	$

rsi	ld	hl,(pos)
bucle	ld	a,(hl)
	or	a
	jr	nz,finrsi
	inc	hl
	ld	a,(hl)
	cp	0ffh
	jr	z,fin
	cp	0feh
	jr	z,salta
	ld	bc,0fffdh
	out	(c),a
	inc	hl
	ld	a,(hl)
	ld	b,0bfh
	out	(c),a
	inc	hl
	jr	bucle
finrsi	ld	(pos),hl
	dec	a
	ld	(hl),a
	ei
	ret
salta	inc	hl
	inc	hl
	ld	(pos),hl
	ei
	ret
fin	xor	a
	out	(0feh),a
	jr	$

pos	dw	notas

notas
; CUERPO
;	db	0, 0ffh	; debe incluirse al final

end	inicio
