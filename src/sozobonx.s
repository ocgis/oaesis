          .globl _link_in
          .globl _link_remove
          .globl _set_stack
          .globl _newmvec
          .globl _newbvec
          .globl _newtvec
          .globl _vdicall
          .globl _accstart
          .globl _VsetScreen
          .globl _VsetMode

          .globl _h_aes_call
          .globl _oldbvec
          .globl _oldmvec
          .globl _Moudev_handler
          .globl _p_fsel_extern
          .globl _aescall

          .text
	
myxgemdos:
          cmp.w	  #$c8,d0
          bne	  retaddr

          movem.l d0-d7/a0-a6,-(sp)

          move.l  _p_fsel_extern,a0
          tst.w   (a0)
          beq     intern

          move.l  d1,a0
          move.l  (a0),a0
          cmp.w   #$5a,(a0)
          beq     ignore
          cmp.w   #$5b,(a0)
          beq     ignore

intern:
          move.l  d1,-(sp)
          jsr     _h_aes_call
          addq.l  #$4,sp
          movem.l (sp)+,d0-d7/a0-a6

          rte

ignore:
          movem.l (sp)+,d0-d7/a0-a6
retaddr:
          jmp     _link_in

_link_in:
          move.l  $88,retaddr+2
          move.l  #myxgemdos,$88
          rts

_link_remove:
          move.l  retaddr+2,$88;
          rts


_set_stack:
          move.l  $4(sp),a0
          move.l  (sp),-(a0)
          move.l  a0,sp
          rts

_newmvec:
          move.l  sp,newstack+800
          lea     newstack+800,sp
          movem.l d0-d2/a0-a2,-(sp)
          lea     mmov+4(pc),a0
          move.w  d1,(a0)+            ; pass position
          move.w  d0,(a0)
          pea     -6(a0)
          jsr     _Moudev_handler
          addq.l  #$4,sp
          movem.l (sp)+,d0-d2/a0-a2
          move.l  (sp),sp
          rts

_newbvec:
          move.l  sp,newstack+800
          lea     newstack+800,sp
          movem.l d0-d2/a0-a2,-(sp)
          lea     mbut+6(pc),a0
          move.w  d0,(a0)             ; pass buttons
          pea     -6(a0)
          jsr     _Moudev_handler
          addq.l  #$4,sp
          movem.l (sp)+,d0-d2/a0-a2
          move.l  (sp),sp
          rts

_newtvec:
          move.l  sp,newstack+800
          lea     newstack+800,sp
          movem.l d0-d2/a0-a2,-(sp)
          pea     mtim
          jsr     _Moudev_handler
          addq.l  #$4,sp
          movem.l (sp)+,d0-d2/a0-a2
          move.l  (sp),sp
          rts

_vdicall:
          move.l  $4(sp),d1
          move.l  #$73,d0
          trap    #2
          rts

_accstart:
          move.l  4(sp),a0
          move.l  16(a0),a1
          move.l  a1,8(a0)
          add.l   12(a0),a1
          move.l  a1,16(a0)
          move.l  8(a0),a1
          jmp     (a1)

_VsetScreen:
	link  a6,#0
	move.w 18(a6),-(sp)
	move.w 16(a6),-(sp)
	move.l 12(a6),-(sp)
	move.l 8(a6),-(sp)
	move.w #$5,-(sp)
	trap  #14
	lea   14(sp),sp
	unlk  a6
	rts
		
_VsetMode:
	link  a6,#0
	move.w 8(a6),-(sp)
	move.w #$58,-(sp)
	trap  #14
	addq.l #4,sp
	unlk  a6
	rts

_aescall:
          move.l  $4(sp),d1
          move.l  #200,d0
          trap    #2
          rts
		
          .even
mmov:
          .dc.w 0, 2, 0, 0
mbut:
          .dc.w 0, 1, 1, 0
mtim:
          .dc.w 0, 0, 0, 20
	
newstack:
          .ds.l 201


