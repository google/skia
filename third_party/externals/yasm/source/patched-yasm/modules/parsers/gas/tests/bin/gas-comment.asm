# This is a comment

/* So is this */

// and so is this

.byte 0		/* at end of line? */

.byte 0		/* at end of line,
multi-line? */

/* start of line? */ .byte 0

/* What about
a multi-line
comment? -- at start of line?
*/ .byte 0

.byte 0, /* in middle? */ 1

# Illegal; 1 seen on next line
#.byte 0, /* in middle,
#spanning lines? */ 1

/* EOF in comment?
