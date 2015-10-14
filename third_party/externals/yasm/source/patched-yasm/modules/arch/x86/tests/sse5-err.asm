fmaddpd xmm1, xmm2, xmm1, xmm3		; illegal
fmaddpd xmm1, xmm2, xmm3, xmm3		; illegal
fmaddpd xmm1, xmm2, xmm2, xmm3		; illegal

fmaddps xmm1, xmm2, xmm1, xmm3		; illegal
fmaddps xmm1, xmm2, xmm3, xmm3		; illegal
fmaddps xmm1, xmm2, xmm2, xmm3		; illegal

fmaddsd xmm1, xmm2, xmm1, xmm3		; illegal
fmaddsd xmm1, xmm2, xmm3, xmm3		; illegal
fmaddsd xmm1, xmm2, xmm2, xmm3		; illegal

fmaddss xmm1, xmm2, xmm1, xmm3		; illegal
fmaddss xmm1, xmm2, xmm3, xmm3		; illegal
fmaddss xmm1, xmm2, xmm2, xmm3		; illegal

fmsubpd xmm1, xmm2, xmm1, xmm3		; illegal
fmsubpd xmm1, xmm2, xmm3, xmm3		; illegal
fmsubpd xmm1, xmm2, xmm2, xmm3		; illegal

fmsubps xmm1, xmm2, xmm1, xmm3		; illegal
fmsubps xmm1, xmm2, xmm3, xmm3		; illegal
fmsubps xmm1, xmm2, xmm2, xmm3		; illegal

fmsubsd xmm1, xmm2, xmm1, xmm3		; illegal
fmsubsd xmm1, xmm2, xmm3, xmm3		; illegal
fmsubsd xmm1, xmm2, xmm2, xmm3		; illegal

fmsubss xmm1, xmm2, xmm1, xmm3		; illegal
fmsubss xmm1, xmm2, xmm3, xmm3		; illegal
fmsubss xmm1, xmm2, xmm2, xmm3		; illegal

fnmaddpd xmm1, xmm2, xmm1, xmm3		; illegal
fnmaddpd xmm1, xmm2, xmm3, xmm3		; illegal
fnmaddpd xmm1, xmm2, xmm2, xmm3		; illegal

fnmaddps xmm1, xmm2, xmm1, xmm3		; illegal
fnmaddps xmm1, xmm2, xmm3, xmm3		; illegal
fnmaddps xmm1, xmm2, xmm2, xmm3		; illegal

fnmaddsd xmm1, xmm2, xmm1, xmm3		; illegal
fnmaddsd xmm1, xmm2, xmm3, xmm3		; illegal
fnmaddsd xmm1, xmm2, xmm2, xmm3		; illegal

fnmaddss xmm1, xmm2, xmm1, xmm3		; illegal
fnmaddss xmm1, xmm2, xmm3, xmm3		; illegal
fnmaddss xmm1, xmm2, xmm2, xmm3		; illegal

fnmsubpd xmm1, xmm2, xmm1, xmm3		; illegal
fnmsubpd xmm1, xmm2, xmm3, xmm3		; illegal
fnmsubpd xmm1, xmm2, xmm2, xmm3		; illegal

fnmsubps xmm1, xmm2, xmm1, xmm3		; illegal
fnmsubps xmm1, xmm2, xmm3, xmm3		; illegal
fnmsubps xmm1, xmm2, xmm2, xmm3		; illegal

fnmsubsd xmm1, xmm2, xmm1, xmm3		; illegal
fnmsubsd xmm1, xmm2, xmm3, xmm3		; illegal
fnmsubsd xmm1, xmm2, xmm2, xmm3		; illegal

fnmsubss xmm1, xmm2, xmm1, xmm3		; illegal
fnmsubss xmm1, xmm2, xmm3, xmm3		; illegal
fnmsubss xmm1, xmm2, xmm2, xmm3		; illegal

pcmov xmm1, xmm2, xmm1, xmm3		; illegal
pcmov xmm1, xmm2, xmm3, xmm3		; illegal
pcmov xmm1, xmm2, xmm2, xmm3		; illegal

permpd xmm1, xmm2, xmm1, xmm3		; illegal
permpd xmm1, xmm2, xmm3, xmm3		; illegal
permpd xmm1, xmm2, xmm2, xmm3		; illegal

permps xmm1, xmm2, xmm1, xmm3		; illegal
permps xmm1, xmm2, xmm3, xmm3		; illegal
permps xmm1, xmm2, xmm2, xmm3		; illegal

pmacsdd xmm1, xmm2, xmm1, xmm3		; illegal
pmacsdd xmm1, xmm1, xmm2, xmm3		; illegal - better message?

pmacsdqh xmm1, xmm2, xmm1, xmm3		; illegal
pmacsdqh xmm1, xmm1, xmm2, xmm3		; illegal - better message?

pmacsdql xmm1, xmm2, xmm1, xmm3		; illegal
pmacsdql xmm1, xmm1, xmm2, xmm3		; illegal - better message?

pmacssdd xmm1, xmm2, xmm1, xmm3		; illegal
pmacssdd xmm1, xmm1, xmm2, xmm3		; illegal - better message?

pmacssdqh xmm1, xmm2, xmm1, xmm3	; illegal
pmacssdqh xmm1, xmm1, xmm2, xmm3	; illegal - better message?

pmacssdql xmm1, xmm2, xmm1, xmm3	; illegal
pmacssdql xmm1, xmm1, xmm2, xmm3	; illegal - better message?

pmacsswd xmm1, xmm2, xmm1, xmm3		; illegal
pmacsswd xmm1, xmm1, xmm2, xmm3		; illegal - better message?

pmacssww xmm1, xmm2, xmm1, xmm3		; illegal
pmacssww xmm1, xmm1, xmm2, xmm3		; illegal - better message?

pmacswd xmm1, xmm2, xmm1, xmm3		; illegal
pmacswd xmm1, xmm1, xmm2, xmm3		; illegal - better message?

pmacsww xmm1, xmm2, xmm1, xmm3		; illegal
pmacsww xmm1, xmm1, xmm2, xmm3		; illegal - better message?

pmadcsswd xmm1, xmm2, xmm1, xmm3	; illegal
pmadcsswd xmm1, xmm1, xmm2, xmm3	; illegal - better message?

pmadcswd xmm1, xmm2, xmm1, xmm3		; illegal
pmadcswd xmm1, xmm1, xmm2, xmm3		; illegal - better message?

pperm xmm1, xmm2, xmm1, xmm3		; illegal
pperm xmm1, xmm2, xmm3, xmm3		; illegal
pperm xmm1, xmm2, xmm2, xmm3		; illegal

