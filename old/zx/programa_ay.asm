	org	8000h

INIT	ret
INTER	ld	hl,(pos)
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
	ret
salta	inc	hl
	inc	hl
	ld	(pos),hl
fin	ret

pos	dw	notas

notas

end
