	xdef aes_call

	text

aes_call:
	move.l a0,  d1
	move.l #$c8, d0
	trap   #2
	rts
