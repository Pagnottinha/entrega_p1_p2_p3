.data
	AUX ?
	expr_0 ?
	A ?
	RES ?
.code
	LDA 2
	@MUL 2
	STA AUX
	LDA 6
	@SUB AUX
	STA expr_0
	LDA 2
	@MUL expr_0
	ADD 5
	STA A
	LDA A
	@SUB 9
	STA RES
	HLT
