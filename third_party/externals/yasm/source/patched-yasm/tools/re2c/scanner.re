#include <stdlib.h>
#include <string.h>
#include "tools/re2c/scanner.h"
#include "tools/re2c/parse.h"
#include "tools/re2c/globals.h"
#include "tools/re2c/parser.h"

#ifndef MAX
#define MAX(a,b) (((a)>(b))?(a):(b))
#endif

#define	BSIZE	8192

#define	YYCTYPE		unsigned char
#define	YYCURSOR	cursor
#define	YYLIMIT		s->lim
#define	YYMARKER	s->ptr
#define	YYFILL(n)	{cursor = fill(s, cursor);}

#define	RETURN(i)	{s->cur = cursor; return i;}

static unsigned char *fill(Scanner*, unsigned char*);

void
Scanner_init(Scanner *s, FILE *i)
{
    s->in = i;
    s->bot = s->tok = s->ptr = s->cur = s->pos = s->lim = s->top =
	     s->eof = NULL;
    s->tchar = s->tline = 0;
    s->cline = 1;
}

static unsigned char *
fill(Scanner *s, unsigned char *cursor)
{
    if(!s->eof){
	unsigned int cnt = s->tok - s->bot;
	if(cnt){
	    memcpy(s->bot, s->tok, s->lim - s->tok);
	    s->tok = s->bot;
	    s->ptr -= cnt;
	    cursor -= cnt;
	    s->pos -= cnt;
	    s->lim -= cnt;
	}
	if((s->top - s->lim) < BSIZE){
	    unsigned char *buf = malloc(((s->lim - s->bot) + BSIZE) + 1);
	    memcpy(buf, s->tok, s->lim - s->tok);
	    s->tok = buf;
	    s->ptr = &buf[s->ptr - s->bot];
	    cursor = &buf[cursor - s->bot];
	    s->pos = &buf[s->pos - s->bot];
	    s->lim = &buf[s->lim - s->bot];
	    s->top = &s->lim[BSIZE];
	    if (s->bot)
		free(s->bot);
	    s->bot = buf;
	}
	if((cnt = fread(s->lim, 1, BSIZE, s->in)) != BSIZE){
	    s->eof = &s->lim[cnt]; *s->eof++ = '\0';
	}
	s->lim += cnt;
    }
    return cursor;
}

/*!re2c
zero		= "\000";
any		= [\000-\377];
dot		= any \ [\n];
esc		= dot \ [\\];
istring		= "[" "^" ((esc \ [\]]) | "\\" dot)* "]" ;
cstring		= "["  ((esc \ [\]]) | "\\" dot)* "]" ;
dstring		= "\"" ((esc \ ["] ) | "\\" dot)* "\"";
sstring		= "'"  ((esc \ ['] ) | "\\" dot)* "'" ;
letter		= [a-zA-Z];
digit		= [0-9];
*/

int
Scanner_echo(Scanner *s, FILE *out)
{
    unsigned char *cursor = s->cur;
    int ignore_eoc = 0;

    /* Catch EOF */
    if (s->eof && cursor == s->eof)
	return 0;

    s->tok = cursor;
echo:
/*!re2c
	"/*!re2c"		{ fwrite(s->tok, 1, &cursor[-7] - s->tok, out);
				  s->tok = cursor;
				  RETURN(1); }
	"/*!max:re2c" {
		fprintf(out, "#define YYMAXFILL %u\n", maxFill);
		s->tok = s->pos = cursor;
		ignore_eoc = 1;
		goto echo;
	}
	"*" "/"		{
		if (ignore_eoc) {
		    ignore_eoc = 0;
		} else {
		    fwrite(s->tok, 1, cursor - s->tok, out);
		}
		s->tok = s->pos = cursor;
		goto echo;
	}
	"\n"			{ fwrite(s->tok, 1, cursor - s->tok, out);
				  s->tok = s->pos = cursor; s->cline++; oline++;
				  goto echo; }
	zero			{ fwrite(s->tok, 1, cursor - s->tok - 1, out); /* -1 so we don't write out the \0 */
				  if(cursor == s->eof) { RETURN(0); } }
	any			{ goto echo; }
*/
}


int
Scanner_scan(Scanner *s)
{
    unsigned char *cursor = s->cur;
    unsigned int depth;

scan:
    s->tchar = cursor - s->pos;
    s->tline = s->cline;
    s->tok = cursor;
/*!re2c
	"{"			{ depth = 1;
				  goto code;
				}
	"/*"			{ depth = 1;
				  goto comment; }

	"*/"			{ s->tok = cursor;
				  RETURN(0); }

	dstring			{ s->cur = cursor;
				  yylval.regexp = strToRE(Scanner_token(s));
				  return STRING; }

	sstring			{ s->cur = cursor;
				  yylval.regexp = strToCaseInsensitiveRE(Scanner_token(s));
				  return STRING; }

	"\""			{ Scanner_fatal(s, "unterminated string constant (missing \")"); }
	"'"			{ Scanner_fatal(s, "unterminated string constant (missing ')"); }

	istring			{ s->cur = cursor;
				  yylval.regexp = invToRE(Scanner_token(s));
				  return RANGE; }

	cstring			{ s->cur = cursor;
				  yylval.regexp = ranToRE(Scanner_token(s));
				  return RANGE; }

	"["			{ Scanner_fatal(s, "unterminated range (missing ])"); }

	[()|=;/\\]		{ RETURN(*s->tok); }

	[*+?]			{ yylval.op = *s->tok;
				  RETURN(CLOSE); }

	"{" [0-9]+ "}"		{ yylval.extop.minsize = atoi((char *)s->tok+1);
				  yylval.extop.maxsize = atoi((char *)s->tok+1);
				  RETURN(CLOSESIZE); }

	"{" [0-9]+ "," [0-9]+ "}"	{ yylval.extop.minsize = atoi((char *)s->tok+1);
				  yylval.extop.maxsize = MAX(yylval.extop.minsize,atoi(strchr((char *)s->tok, ',')+1));
				  RETURN(CLOSESIZE); }

	"{" [0-9]+ ",}"		{ yylval.extop.minsize = atoi((char *)s->tok+1);
				  yylval.extop.maxsize = -1;
				  RETURN(CLOSESIZE); }

	letter (letter|digit)*	{ SubStr substr;
				  s->cur = cursor;
				  substr = Scanner_token(s);
				  yylval.symbol = Symbol_find(&substr);
				  return ID; }

	[ \t]+			{ goto scan; }

	"\n"			{ if(cursor == s->eof) RETURN(0);
				  s->pos = cursor; s->cline++;
				  goto scan;
	    			}

	"."			{ s->cur = cursor;
				  yylval.regexp = mkDot();
				  return RANGE;
				}

	any			{ fprintf(stderr, "unexpected character: '%c'\n", *s->tok);
				  goto scan;
				}
*/

code:
/*!re2c
	"}"			{ if(--depth == 0){
					s->cur = cursor;
					yylval.token = Token_new(Scanner_token(s), s->tline);
					return CODE;
				  }
				  goto code; }
	"{"			{ ++depth;
				  goto code; }
	"\n"			{ if(cursor == s->eof) Scanner_fatal(s, "missing '}'");
				  s->pos = cursor; s->cline++;
				  goto code;
				}
	dstring | sstring | any	{ goto code; }
*/

comment:
/*!re2c
	"*/"			{ if(--depth == 0)
					goto scan;
				    else
					goto comment; }
	"/*"			{ ++depth;
				  goto comment; }
	"\n"			{ if(cursor == s->eof) RETURN(0);
				  s->tok = s->pos = cursor; s->cline++;
				  goto comment;
				}
        any			{ goto comment; }
*/
}

void
Scanner_fatal(Scanner *s, const char *msg)
{
    fprintf(stderr, "line %d, column %d: %s\n", s->tline, s->tchar + 1, msg);
    exit(1);
}
