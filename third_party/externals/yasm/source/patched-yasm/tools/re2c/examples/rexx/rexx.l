#include "scanio.h"
#include "scanner.h"

#define	CURSOR		ch
#define	LOADCURSOR	ch = *cursor;
#define	ADVANCE		cursor++;
#define	BACK(n)		cursor -= (n);
#define	CHECK(n)	if((ScanCB.lim - cursor) < (n)){cursor = ScanFill(cursor);}
#define	MARK(n)		ScanCB.ptr = cursor; sel = (n);
#define	REVERT		cursor = ScanCB.ptr;
#define	MARKER		sel

#define	RETURN(i)	{ScanCB.cur = cursor; return i;}

int ScanToken(){
	uchar *cursor = ScanCB.cur;
	unsigned sel;
	uchar ch;
	ScanCB.tok = cursor;
	ScanCB.eot = NULL;
/*!re2c
all	= [\000-\377];
eof	= [\000];
any	= all\eof;
letter	= [a-z]|[A-Z];
digit	= [0-9];
symchr	= letter|digit|[.!?_];
const	= (digit|[.])symchr*([eE][+-]?digit+)?;
simple	= (symchr\(digit|[.]))(symchr\[.])*;
stem	= simple [.];
symbol	= symchr*;
sqstr	= ['] ((any\['\n])|(['][']))* ['];
dqstr	= ["] ((any\["\n])|(["]["]))* ["];
str	= sqstr|dqstr;
ob	= [ \t]*;
not	= [\\~];
A	= [aA];
B	= [bB];
C	= [cC];
D	= [dD];
E	= [eE];
F	= [fF];
G	= [gG];
H	= [hH];
I	= [iI];
J	= [jJ];
K	= [kK];
L	= [lL];
M	= [mM];
N	= [nN];
O	= [oO];
P	= [pP];
Q	= [qQ];
R	= [rR];
S	= [sS];
T	= [tT];
U	= [uU];
V	= [vV];
W	= [wW];
X	= [xX];
Y	= [yY];
Z	= [zZ];
*/

scan:
/*!re2c
"\n"
	    {
		++(ScanCB.lineNum);
		ScanCB.linePos = ScanCB.pos + (cursor - ScanCB.mrk);
		RETURN(SU_EOL);
	    }
"|" ob "|"
	    { RETURN(OP_CONCAT); }
"+"
	    { RETURN(OP_PLUS); }
"-"
	    { RETURN(OP_MINUS); }
"*"
	    { RETURN(OP_MULT); }
"/"
	    { RETURN(OP_DIV); }
"%"
	    { RETURN(OP_IDIV); }
"/" ob "/"
	    { RETURN(OP_REMAIN); }
"*" ob "*"
	    { RETURN(OP_POWER); }
"="
	    { RETURN(OP_EQUAL); }
not ob "=" | "<" ob ">" | ">" ob "<"
	    { RETURN(OP_EQUAL_N); }
">"
	    { RETURN(OP_GT); }
"<"
	    { RETURN(OP_LT); }
">" ob "=" | not ob "<"
	    { RETURN(OP_GE); }
"<" ob "=" | not ob ">"
	    { RETURN(OP_LE); }
"=" ob "="
	    { RETURN(OP_EQUAL_EQ); }
not ob "=" ob "="
	    { RETURN(OP_EQUAL_EQ_N); }
">" ob ">"
	    { RETURN(OP_GT_STRICT); }
"<" ob "<"
	    { RETURN(OP_LT_STRICT); }
">" ob ">" ob "=" | not ob "<" ob "<"
	    { RETURN(OP_GE_STRICT); }
"<" ob "<" ob "=" | not ob ">" ob ">"
	    { RETURN(OP_LE_STRICT); }
"&"
	    { RETURN(OP_AND); }
"|"
	    { RETURN(OP_OR); }
"&" ob "&"
	    { RETURN(OP_XOR); }
not
	    { RETURN(OP_NOT); }

":"
	    { RETURN(SU_COLON); }
","
	    { RETURN(SU_COMMA); }
"("
	    { RETURN(SU_POPEN); }
")"
	    { RETURN(SU_PCLOSE); }
";"
	    { RETURN(SU_EOC); }

A D D R E S S
	    { RETURN(RX_ADDRESS); }
A R G
	    { RETURN(RX_ARG); }
C A L L
	    { RETURN(RX_CALL); }
D O
	    { RETURN(RX_DO); }
D R O P
	    { RETURN(RX_DROP); }
E L S E
	    { RETURN(RX_ELSE); }
E N D
	    { RETURN(RX_END); }
E X I T
	    { RETURN(RX_EXIT); }
I F
	    { RETURN(RX_IF); }
I N T E R P R E T
	    { RETURN(RX_INTERPRET); }
I T E R A T E
	    { RETURN(RX_ITERATE); }
L E A V E
	    { RETURN(RX_LEAVE); }
N O P
	    { RETURN(RX_NOP); }
N U M E R I C
	    { RETURN(RX_NUMERIC); }
O P T I O N S
	    { RETURN(RX_OPTIONS); }
O T H E R W I S E
	    { RETURN(RX_OTHERWISE); }
P A R S E
	    { RETURN(RX_PARSE); }
P R O C E D U R E
	    { RETURN(RX_PROCEDURE); }
P U L L
	    { RETURN(RX_PULL); }
P U S H
	    { RETURN(RX_PUSH); }
Q U E U E
	    { RETURN(RX_QUEUE); }
R E T U R N
	    { RETURN(RX_RETURN); }
S A Y
	    { RETURN(RX_SAY); }
S E L E C T
	    { RETURN(RX_SELECT); }
S I G N A L
	    { RETURN(RX_SIGNAL); }
T H E N
	    { RETURN(RX_THEN); }
T R A C E
	    { RETURN(RX_TRACE); }
W H E N
	    { RETURN(RX_WHEN); }
O F F
	    { RETURN(RXS_OFF); }
O N
	    { RETURN(RXS_ON); }
B Y
	    { RETURN(RXS_BY); }
D I G I T S
	    { RETURN(RXS_DIGITS); }
E N G I N E E R I N G
	    { RETURN(RXS_ENGINEERING); }
E R R O R
	    { RETURN(RXS_ERROR); }
E X P O S E
	    { RETURN(RXS_EXPOSE); }
F A I L U R E
	    { RETURN(RXS_FAILURE); }
F O R
	    { RETURN(RXS_FOR); }
F O R E V E R
	    { RETURN(RXS_FOREVER); }
F O R M
	    { RETURN(RXS_FORM); }
F U Z Z
	    { RETURN(RXS_FUZZ); }
H A L T
	    { RETURN(RXS_HALT); }
L I N E I N
	    { RETURN(RXS_LINEIN); }
N A M E
	    { RETURN(RXS_NAME); }
N O T R E A D Y
	    { RETURN(RXS_NOTREADY); }
N O V A L U E
	    { RETURN(RXS_NOVALUE); }
S C I E N T I F I C
	    { RETURN(RXS_SCIENTIFIC); }
S O U R C E
	    { RETURN(RXS_SOURCE); }
S Y N T A X
	    { RETURN(RXS_SYNTAX); }
T O
	    { RETURN(RXS_TO); }
U N T I L
	    { RETURN(RXS_UNTIL); }
U P P E R
	    { RETURN(RXS_UPPER); }
V A L U E
	    { RETURN(RXS_VALUE); }
V A R
	    { RETURN(RXS_VAR); }
V E R S I O N
	    { RETURN(RXS_VERSION); }
W H I L E
	    { RETURN(RXS_WHILE); }
W I T H
	    { RETURN(RXS_WITH); }

const
	    { RETURN(SU_CONST); }
simple
	    { RETURN(SU_SYMBOL); }
stem
	    { RETURN(SU_SYMBOL_STEM); }
symbol
	    { RETURN(SU_SYMBOL_COMPOUND); }
str
	    { RETURN(SU_LITERAL); }
str [bB] / (all\symchr)
	    { RETURN(SU_LITERAL_BIN); }
str [xX] / (all\symchr)
	    { RETURN(SU_LITERAL_HEX); }

eof
	    { RETURN(SU_EOF); }
any
	    { RETURN(SU_ERROR); }
*/
}

bool StripToken(){
	uchar *cursor = ScanCB.cur;
	unsigned depth;
	uchar ch;
	bool blanks = FALSE;
	ScanCB.eot = cursor;
strip:
/*!re2c
"/*"
	    {
		depth = 1;
		goto comment;
	    }
"\r"
	    { goto strip; }
[ \t]
	    {
		blanks = TRUE;
		goto strip;
	    }
[] / all
	    { RETURN(blanks); }
*/

comment:
/*!re2c
"*/"
	    {
		if(--depth == 0)
		    goto strip;
		else
		    goto comment;
	    }
"\n"
	    {
		++(ScanCB.lineNum);
		ScanCB.linePos = ScanCB.pos + (cursor - ScanCB.mrk);
		goto comment;
	    }
"/*"
	    {
		++depth;
		goto comment;
	    }
eof
	    { RETURN(blanks); }
any
	    {
		goto comment;
	    }
*/
}
