;Copyright (c) 2021 Scott Maday
;
;Permission is hereby granted, free of charge, to any person obtaining a copy
;of this software and associated documentation files (the "Software"), to deal
;in the Software without restriction, including without limitation the rights
;to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;copies of the Software, and to permit persons to whom the Software is
;furnished to do so, subject to the following conditions:
;
;The above copyright notice and this permission notice shall be included in all
;copies or substantial portions of the Software.
;
;THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;SOFTWARE.

[bits 64]
EXTERN InterruptHandlers

%macro	PUSH_REG	0
	push	r15
	push	r14
	push	r13
	push	r12
	push	r11
	push	r10
	push	r9
	push	r8
	push	rbp
	push	rdi
	push	rsi
	push	rdx
	push	rcx
	push	rbx
	push	rax
	push	cr3
%endmacro

%macro	POP_REG		0
	pop cr3
	pop	rax
	pop	rbx
	pop	rcx
	pop	rdx
	pop	rsi
	pop	rdi
	pop	rbp
	pop	r8
	pop	r9
	pop	r10
	pop	r11
	pop	r12
	pop	r13
	pop	r14
	pop	r15
%endmacro


%macro	ISR_DEFINE	2
	GLOBAL	isr%1
	isr%1:
		%if	%2 == 0
			push	QWORD 0				; push for so the stack size is consistent
		%endif
		PUSH_REG
		mov		rax, [InterruptHandlers + 8 * %1]
		cmp		rax, 0					; hopefully, a valid function doesn't point to 0
		jz		%%.finish
		mov		rdi, rsp				; we'll create a struct out of the stack pointer
		mov		rsi, %1
		call	rax
		%%.finish:
			POP_REG
			%if	%2 == 0
				add		rsp, 8
			%endif
			iretq
%endmacro

; Exceptions
ISR_DEFINE	0, 0	; Divide By Zero Exception
ISR_DEFINE	1, 0	; Debug Exception
ISR_DEFINE	2, 0	; Non Maskable Interrupt Exception
ISR_DEFINE	3, 0	; Breakpoint Exception
ISR_DEFINE	4, 0	; Into Detected Overflow Exception
ISR_DEFINE	5, 0	; Out of Bounds Exception
ISR_DEFINE	6, 0	; Invalid Opcode Exception
ISR_DEFINE	7, 0	; No Coprocessor Exception
ISR_DEFINE	8, 1	; Double Fault Exception
ISR_DEFINE	9, 0	; Coprocessor Segment Overrun Exception
ISR_DEFINE	10, 1	; Segment Not Present Exception
ISR_DEFINE	11, 1	; Segment Not Present Exception
ISR_DEFINE	12, 1	; Stack Fault Exception
ISR_DEFINE	13, 1	; General Protection Fault Exception
ISR_DEFINE	14, 1	; Page Fault Exception
ISR_DEFINE	16, 0	; Coprocessor Fault Exception
ISR_DEFINE	17, 1	; Alignment Check Exception (486+)
ISR_DEFINE	18, 0	; Machine Check Exception (Pentium/586+)
ISR_DEFINE	19, 0	; SIMD Floating-Point Exception
ISR_DEFINE	20, 0	; SIMD Floating-Point Exception
ISR_DEFINE	30, 1	; Security Exception
; Interrupt requests
ISR_DEFINE	32, 0	; PIC Timer
ISR_DEFINE	33, 0	; Keyboard Interrupt
ISR_DEFINE	35, 0	; COM2
ISR_DEFINE	36, 0	; COM1
ISR_DEFINE	37, 0	; LPT2
ISR_DEFINE	39, 0	; LPT1

ISR_DEFINE  128, 0   ; Syscall