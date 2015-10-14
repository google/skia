/*!re2c
	"print"		{return PRINT;}
	[a-z]+		{return ID;}
	[0-9]+		{return DEC;}
	"0x" [0-9a-f]+	{return HEX;}
	[\000-\377]	{return ERR;}
*/
