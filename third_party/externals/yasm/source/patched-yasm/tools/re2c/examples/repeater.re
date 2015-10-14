#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define RET(n) printf("%d\n", n); return n

int scan(char *s, int l){
char *p = s;
char *q;
#define YYCTYPE         char
#define YYCURSOR        p
#define YYLIMIT         (s+l)
#define YYMARKER        q
#define YYFILL(n)
/*!re2c
	'a'{1}"\n"	    {RET(1);}
	'a'{2,3}"\n"	{RET(2);}
	'a'{4,}"\n"	    {RET(3);}
	'a'{6}"\n"	    {RET(4);}
	[^aq]|"\n"      {RET(0);}
*/
}

#define do_scan(str) scan(str, strlen(str))

main()
{
	do_scan("a\n");
	do_scan("aa\n");
	do_scan("aaa\n");
	do_scan("aaaa\n");
	do_scan("A\n");
	do_scan("AA\n");
	do_scan("aAa\n");
	do_scan("AaaA\n");
	do_scan("Q");
	do_scan("AaaAa\n");
	do_scan("AaaAaA\n");
	do_scan("A");
	do_scan("\n");
	do_scan("0");
}
