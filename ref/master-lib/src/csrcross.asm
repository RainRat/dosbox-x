; master library
;
; Description:
;	�O���t�B�b�N�J�[�\���`��f�[�^ (2)�\��
;
; Author:
;	���ˏ��F
;
; Revision History:
;	93/ 4/13 Initial: master.lib 0.16  csrarrow.asm

.MODEL SMALL
.DATA

	public cursor_Cross
	public _cursor_Cross
cursor_Cross label byte
_cursor_Cross db 7,7
	dw	0000001010000000b
	dw	0000001010000000b
	dw	0000001010000000b
	dw	0000001010000000b
	dw	0000001010000000b
	dw	0000001010000000b
	dw	1111111011111110b
	dw	0000000000000000b
	dw	1111111011111110b
	dw	0000001010000000b
	dw	0000001010000000b
	dw	0000001010000000b
	dw	0000001010000000b
	dw	0000001010000000b
	dw	0000001010000000b
	dw	0000000000000000b

	dw	0000000100000000b
	dw	0000000100000000b
	dw	0000000100000000b
	dw	0000000100000000b
	dw	0000000100000000b
	dw	0000000100000000b
	dw	0000000100000000b
	dw	1111111111111110b
	dw	0000000100000000b
	dw	0000000100000000b
	dw	0000000100000000b
	dw	0000000100000000b
	dw	0000000100000000b
	dw	0000000100000000b
	dw	0000000100000000b
	dw	0000000000000000b

END