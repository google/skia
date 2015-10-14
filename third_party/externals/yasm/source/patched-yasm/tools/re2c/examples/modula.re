#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef unsigned int uint;
typedef unsigned char uchar;

#define	BSIZE	8192

#define	YYCTYPE		uchar
#define	YYCURSOR	cursor
#define	YYLIMIT		s->lim
#define	YYMARKER	s->ptr
#define	YYFILL		{cursor = fill(s, cursor);}

#define	RETURN(i)	{s->cur = cursor; return i;}

typedef struct Scanner {
    int			fd;
    uchar		*bot, *tok, *ptr, *cur, *pos, *lim, *top, *eof;
    uint		line;
} Scanner;

uchar *fill(Scanner *s, uchar *cursor){
    if(!s->eof){
	uint cnt = s->tok - s->bot;
	if(cnt){
	    memcpy(s->bot, s->tok, s->lim - s->tok);
	    s->tok = s->bot;
	    s->ptr -= cnt;
	    cursor -= cnt;
	    s->pos -= cnt;
	    s->lim -= cnt;
	}
	if((s->top - s->lim) < BSIZE){
	    uchar *buf = (uchar*) malloc(((s->lim - s->bot) + BSIZE)*sizeof(uchar));
	    memcpy(buf, s->tok, s->lim - s->tok);
	    s->tok = buf;
	    s->ptr = &buf[s->ptr - s->bot];
	    cursor = &buf[cursor - s->bot];
	    s->pos = &buf[s->pos - s->bot];
	    s->lim = &buf[s->lim - s->bot];
	    s->top = &s->lim[BSIZE];
	    free(s->bot);
	    s->bot = buf;
	}
	if((cnt = read(s->fd, (char*) s->lim, BSIZE)) != BSIZE){
	    s->eof = &s->lim[cnt]; *(s->eof)++ = '\n';
	}
	s->lim += cnt;
    }
    return cursor;
}

int scan(Scanner *s){
	uchar *cursor = s->cur;
	uint depth;
std:
	s->tok = cursor;
/*!re2c
any	= [\000-\377];
digit	= [0-9];
letter	= [a-zA-Z];
*/

/*!re2c
	"(*"			{ depth = 1; goto comment; }

	digit +			{RETURN(1);}
	digit + / ".."   	{RETURN(1);}
	[0-7] + "B"        	{RETURN(2);}
	[0-7] + "C"        	{RETURN(3);}
	digit [0-9A-F] * "H"	{RETURN(4);}
	digit + "." digit * ("E" ([+-]) ? digit +) ?	{RETURN(5);}
	['] (any\[\n']) * [']	| ["] (any\[\n"]) * ["]	{RETURN(6);}

	"#"              	{RETURN(7);}
	"&"              	{RETURN(8);}
	"("              	{RETURN(9);}
	")"              	{RETURN(10);}
	"*"              	{RETURN(11);}
	"+"              	{RETURN(12);}
	","              	{RETURN(13);}
	"-"              	{RETURN(14);}
	"."              	{RETURN(15);}
	".."             	{RETURN(16);}
	"/"              	{RETURN(17);}
	":"              	{RETURN(18);}
	":="             	{RETURN(19);}
	";"              	{RETURN(20);}
	"<"              	{RETURN(21);}
	"<="             	{RETURN(22);}
	"<>"             	{RETURN(23);}
	"="              	{RETURN(24);}
	">"              	{RETURN(25);}
	">="             	{RETURN(26);}
	"["              	{RETURN(27);}
	"]"              	{RETURN(28);}
	"^"              	{RETURN(29);}
	"{"              	{RETURN(30);}
	"|"              	{RETURN(31);}
	"}"              	{RETURN(32);}
	"~"              	{RETURN(33);}

	"AND"              	{RETURN(34);}
	"ARRAY"            	{RETURN(35);}
	"BEGIN"           	{RETURN(36);}
	"BY"               	{RETURN(37);}
	"CASE"             	{RETURN(38);}
	"CONST"            	{RETURN(39);}
	"DEFINITION"       	{RETURN(40);}
	"DIV"              	{RETURN(41);}
	"DO"               	{RETURN(42);}
	"ELSE"             	{RETURN(43);}
	"ELSIF"            	{RETURN(44);}
	"END"              	{RETURN(45);}
	"EXIT"             	{RETURN(46);}
	"EXPORT"          	{RETURN(47);}
	"FOR"              	{RETURN(48);}
	"FROM"             	{RETURN(49);}
	"IF"               	{RETURN(50);}
	"IMPLEMENTATION"   	{RETURN(51);}
	"IMPORT"           	{RETURN(52);}
	"IN"               	{RETURN(53);}
	"LOOP"             	{RETURN(54);}
	"MOD"              	{RETURN(55);}
	"MODULE"           	{RETURN(56);}
	"NOT"              	{RETURN(57);}
	"OF"               	{RETURN(58);}
	"OR"               	{RETURN(59);}
	"POINTER"          	{RETURN(60);}
	"PROCEDURE"        	{RETURN(61);}
	"QUALIFIED"        	{RETURN(62);}
	"RECORD"           	{RETURN(63);}
	"REPEAT"           	{RETURN(64);}
	"RETURN"           	{RETURN(65);}
	"SET"              	{RETURN(66);}
	"THEN"             	{RETURN(67);}
	"TO"               	{RETURN(68);}
	"TYPE"             	{RETURN(69);}
	"UNTIL"            	{RETURN(70);}
	"VAR"              	{RETURN(71);}
	"WHILE"            	{RETURN(72);}
	"WITH"             	{RETURN(73);}

	letter (letter | digit) *	{RETURN(74);}

	[ \t]+			{ goto std; }

	"\n"
	    {
		if(cursor == s->eof) RETURN(0);
		s->pos = cursor; s->line++;
		goto std;
	    }

	any
	    {
		printf("unexpected character: %c\n", *s->tok);
		goto std;
	    }
*/
comment:
/*!re2c
	"*)"
	    {
		if(--depth == 0)
		    goto std;
		else
		    goto comment;
	    }
	"(*"			{ ++depth; goto comment; }
	"\n"
	    {
		if(cursor == s->eof) RETURN(0);
		s->tok = s->pos = cursor; s->line++;
		goto comment;
	    }
        any			{ goto comment; }
*/
}

/*
void putStr(FILE *o, char *s, uint l){
    while(l-- > 0)
	putc(*s++, o);
}
*/

main(){
    Scanner in;
    memset((char*) &in, 0, sizeof(in));
    in.fd = 0;
    while(scan(&in)){
/*
	putc('<', stdout);
	putStr(stdout, (char*) in.tok, in.cur - in.tok);
	putc('>', stdout);
	putc('\n', stdout);
*/
    }
}
