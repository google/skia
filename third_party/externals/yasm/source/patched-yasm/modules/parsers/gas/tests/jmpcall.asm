#.text
label:
	call	*%eax
	call	%eax
	call	*(%eax)
	call	(%eax)
	call	label(,1)
	call	label
	call	label+5
	call	*label
	call	*label(%eax)
	jmp	%fs:label
	jmp	*%fs:label
