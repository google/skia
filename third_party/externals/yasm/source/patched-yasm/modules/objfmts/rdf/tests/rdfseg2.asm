	;; library function for rdfseg - this file is linked as a far segment

[BITS 16]
[GLOBAL _puts]
_puts:
	;; can't remember how to print a string in DOS, but if anyone wants
	;; to actually test this program, it should be fairly easy to put
	;; in here!

	retf

	