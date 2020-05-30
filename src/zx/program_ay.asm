	org	8000h

INIT	ret
INTER	ld	hl,(pos)
loop	ld	a,(hl)
	or	a
	jr	nz,isrend
	inc	hl
	ld	a,(hl)
	cp	0ffh
	jr	z,stop
	cp	0feh
	jr	z,break
	ld	bc,0fffdh
	out	(c),a
	inc	hl
	ld	a,(hl)
	ld	b,0bfh
	out	(c),a
	inc	hl
	jr	loop
isrend	ld	(pos),hl
	dec	a
	ld	(hl),a
	ret
break	inc	hl
	inc	hl
	ld	(pos),hl
stop	ret

pos	dw	notas

notas

end
