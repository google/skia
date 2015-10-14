#ifdef _WIN32
#include <windows.h>
#include <io.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "tools/re2c/substr.h"
#include "tools/re2c/globals.h"
#include "tools/re2c/dfa.h"
#include "tools/re2c/parse.h"

#ifdef _WIN32
/* tmpfile() replacment for Windows.
 *
 * On Windows tmpfile() creates the file in the root directory. This
 * may fail due to unsufficient privileges.
 */
static FILE *
win32_tmpfile (void)
{
    DWORD path_len;
    WCHAR path_name[MAX_PATH + 1];
    WCHAR file_name[MAX_PATH + 1];
    HANDLE handle;
    int fd;
    FILE *fp;

    path_len = GetTempPathW (MAX_PATH, path_name);
    if (path_len <= 0 || path_len >= MAX_PATH)
	return NULL;

    if (GetTempFileNameW (path_name, L"ps_", 0, file_name) == 0)
	return NULL;

    handle = CreateFileW (file_name,
			 GENERIC_READ | GENERIC_WRITE,
			 0,
			 NULL,
			 CREATE_ALWAYS,
			 FILE_ATTRIBUTE_NORMAL | FILE_FLAG_DELETE_ON_CLOSE,
			 NULL);
    if (handle == INVALID_HANDLE_VALUE) {
	DeleteFileW (file_name);
	return NULL;
    }

    fd = _open_osfhandle((intptr_t) handle, 0);
    if (fd < 0) {
	CloseHandle (handle);
	return NULL;
    }

    fp = fdopen(fd, "w+b");
    if (fp == NULL) {
	_close(fd);
	return NULL;
    }

    return fp;
}
#endif

static void useLabel(size_t value) {
    while (value >= vUsedLabelAlloc) {
	vUsedLabels = realloc(vUsedLabels, vUsedLabelAlloc * 2);
	if (!vUsedLabels) {
	    fputs("Out of memory.\n", stderr);
	    exit(EXIT_FAILURE);
	}
	memset(vUsedLabels + vUsedLabelAlloc, 0, vUsedLabelAlloc);
	vUsedLabelAlloc *= 2;
    }
    vUsedLabels[value] = 1;
}

/* there must be at least one span in list;  all spans must cover
 * same range
 */

void Go_compact(Go *g){
    /* arrange so that adjacent spans have different targets */
    unsigned int i = 0, j;
    for(j = 1; j < g->nSpans; ++j){
	if(g->span[j].to != g->span[i].to){
	    ++i; g->span[i].to = g->span[j].to;
	}
	g->span[i].ub = g->span[j].ub;
    }
    g->nSpans = i + 1;
}

void Go_unmap(Go *g, Go *base, State *x){
    Span *s = g->span, *b = base->span, *e = &b[base->nSpans];
    unsigned int lb = 0;
    s->ub = 0;
    s->to = NULL;
    for(; b != e; ++b){
	if(b->to == x){
	    if((s->ub - lb) > 1)
		s->ub = b->ub;
	} else {
	    if(b->to != s->to){
		if(s->ub){
		    lb = s->ub; ++s;
		}
		s->to = b->to;
	    }
	    s->ub = b->ub;
	}
    }
    s->ub = e[-1].ub; ++s;
    g->nSpans = s - g->span;
}

static void doGen(Go *g, State *s, unsigned char *bm, unsigned char m){
    Span *b = g->span, *e = &b[g->nSpans];
    unsigned int lb = 0;
    for(; b < e; ++b){
	if(b->to == s)
	    for(; lb < b->ub; ++lb) bm[lb] |= m;
	lb = b->ub;
    }
}
#if 0
static void prt(FILE *o, Go *g, State *s){
    Span *b = g->span, *e = &b[g->nSpans];
    unsigned int lb = 0;
    for(; b < e; ++b){
	if(b->to == s)
	    printSpan(o, lb, b->ub);
	lb = b->ub;
    }
}
#endif
static int matches(Go *g1, State *s1, Go *g2, State *s2){
    Span *b1 = g1->span, *e1 = &b1[g1->nSpans];
    unsigned int lb1 = 0;
    Span *b2 = g2->span, *e2 = &b2[g2->nSpans];
    unsigned int lb2 = 0;
    for(;;){
	for(; b1 < e1 && b1->to != s1; ++b1) lb1 = b1->ub;
	for(; b2 < e2 && b2->to != s2; ++b2) lb2 = b2->ub;
	if(b1 == e1) return b2 == e2;
	if(b2 == e2) return 0;
	if(lb1 != lb2 || b1->ub != b2->ub) return 0;
	++b1; ++b2;
    }
}

typedef struct BitMap {
    Go			*go;
    State		*on;
    struct BitMap	*next;
    unsigned int	i;
    unsigned char	m;
} BitMap;

static BitMap *BitMap_find_go(Go*, State*);
static BitMap *BitMap_find(State*);
static void BitMap_gen(FILE *, unsigned int, unsigned int);
/* static void BitMap_stats(void);*/
static BitMap *BitMap_new(Go*, State*);

static BitMap *BitMap_first = NULL;

BitMap *
BitMap_new(Go *g, State *x)
{
    BitMap *b = malloc(sizeof(BitMap));
    b->go = g;
    b->on = x;
    b->next = BitMap_first;
    BitMap_first = b;
    return b;
}

BitMap *
BitMap_find_go(Go *g, State *x){
    BitMap *b;
    for(b = BitMap_first; b; b = b->next){
	if(matches(b->go, b->on, g, x))
	    return b;
    }
    return BitMap_new(g, x);
}

BitMap *
BitMap_find(State *x){
    BitMap *b;
    for(b = BitMap_first; b; b = b->next){
	if(b->on == x)
	    return b;
    }
    return NULL;
}

void BitMap_gen(FILE *o, unsigned int lb, unsigned int ub){
    BitMap *b = BitMap_first;
    if(b){
	unsigned int n = ub - lb;
	unsigned int i;
	unsigned char *bm = malloc(sizeof(unsigned char)*n);
	fputs("\tstatic unsigned char yybm[] = {", o);
	for(i = 0; b; i += n){
	    unsigned char m;
	    unsigned int j;
	    memset(bm, 0, n);
	    for(m = 0x80; b && m; b = b->next, m >>= 1){
		b->i = i; b->m = m;
		doGen(b->go, b->on, bm-lb, m);
	    }
	    for(j = 0; j < n; ++j){
		if(j%8 == 0) {fputs("\n\t", o); oline++;}
		fprintf(o, "%3u, ", (unsigned int) bm[j]);
	    }
	}
	fputs("\n\t};\n", o); oline+=2;
        free(bm);
    }
}

#if 0
void BitMap_stats(void){
    unsigned int n = 0;
    BitMap *b;
    for(b = BitMap_first; b; b = b->next){
	prt(stderr, b->go, b->on); fputs("\n", stderr);
	++n;
    }
    fprintf(stderr, "%u bitmaps\n", n);
    BitMap_first = NULL;
}
#endif

static void genGoTo(FILE *o, State *from, State *to, int *readCh,
		    const char *indent)
{
#if 0
    if (*readCh && from->label + 1 != to->label)
    {
	fputs("%syych = *YYCURSOR;\n", indent, o); oline++;
	*readCh = 0;
    }
#endif
    fprintf(o, "%sgoto yy%u;\n", indent, to->label); oline++;
    useLabel(to->label);
}

static void genIf(FILE *o, const char *cmp, unsigned int v, int *readCh)
{
#if 0
    if (*readCh)
    {
	fputs("\tif((yych = *YYCURSOR) ", o);
	*readCh = 0;
    } else {
#endif
	fputs("\tif(yych ", o);
#if 0
    }
#endif
    fprintf(o, "%s '", cmp);
    prtCh(o, v);
    fputs("')", o);
}

static void indent(FILE *o, unsigned int i){
    while(i-- > 0)
	fputc('\t', o);
}

static void need(FILE *o, unsigned int n, int *readCh)
{
    unsigned int fillIndex;
    int hasFillIndex = (0<=vFillIndexes);
    if (hasFillIndex) {
	fillIndex = vFillIndexes++;
	fprintf(o, "\tYYSETSTATE(%u);\n", fillIndex);
	++oline;
    }

    if(n == 1) {
	fputs("\tif(YYLIMIT == YYCURSOR) YYFILL(1);\n", o); oline++;
    } else {
	fprintf(o, "\tif((YYLIMIT - YYCURSOR) < %u) YYFILL(%u);\n", n, n);
	oline++;
    }

    if (hasFillIndex) {
	fprintf(o, "yyFillLabel%u:\n", fillIndex);
	++oline;
    }

    fputs("\tyych = *YYCURSOR;\n", o); oline++;
    *readCh = 0;
}

void
Action_emit(Action *a, FILE *o, int *readCh)
{
    int first = 1;
    unsigned int i;
    unsigned int back;

    switch (a->type) {
	case MATCHACT:
	    if(a->state->link){
		fputs("\t++YYCURSOR;\n", o);
		need(o, a->state->depth, readCh);
#if 0
	    } else if (!Action_readAhead(a)) {
		/* do not read next char if match */
		fputs("\t++YYCURSOR;\n", o);
		*readCh = 1;
#endif
	    } else {
		fputs("\tyych = *++YYCURSOR;\n", o);
		*readCh = 0;
	    }
	    oline++;
	    break;
	case ENTERACT:
	    if(a->state->link){
		fputs("\t++YYCURSOR;\n", o);
		fprintf(o, "yy%u:\n", a->d.label); oline+=2;
		need(o, a->state->depth, readCh);
	    } else {
		/* we shouldn't need 'rule-following' protection here */
		fputs("\tyych = *++YYCURSOR;\n", o);
		fprintf(o, "yy%u:\n", a->d.label); oline+=2;
		*readCh = 0;
	    }
	    break;
	case SAVEMATCHACT:
	    if (bUsedYYAccept) {
		fprintf(o, "\tyyaccept = %u;\n", a->d.selector);
		oline++;
	    }
	    if(a->state->link){
		fputs("\tYYMARKER = ++YYCURSOR;\n", o); oline++;
		need(o, a->state->depth, readCh);
	    } else {
		fputs("\tyych = *(YYMARKER = ++YYCURSOR);\n", o); oline++;
		*readCh = 0;
	    }
	    break;
	case MOVEACT:
	    break;
	case ACCEPTACT:
	    for(i = 0; i < a->d.Accept.nRules; ++i)
		if(a->d.Accept.saves[i] != ~0u){
		    if(first){
			first = 0;
			bUsedYYAccept = 1;
			fputs("\tYYCURSOR = YYMARKER;\n", o);
			fputs("\tswitch(yyaccept){\n", o); oline+=2;
		    }
		    fprintf(o, "\tcase %u:", a->d.Accept.saves[i]);
		    genGoTo(o, a->state, a->d.Accept.rules[i], readCh, "\t");
		}
	    if(!first) {
		fputs("\t}\n", o); oline++;
	    }
	    break;
	case RULEACT:
	    back = RegExp_fixedLength(a->d.rule->d.RuleOp.ctx);
	    if(back != ~0u && back > 0u)
		fprintf(o, "\tYYCURSOR -= %u;", back);
	    fprintf(o, "\n"); oline++;
	    line_source(o, a->d.rule->d.RuleOp.code->line);
	    SubStr_out(&a->d.rule->d.RuleOp.code->text, o);
	    fprintf(o, "\n"); oline++;
	    if (!iFlag)
		fprintf(o, "#line %u \"%s\"\n", oline++, outputFileName);
	    break;
    }
}

Action *
Action_new_Accept(State *x, unsigned int n, unsigned int *s, State **r)
{
    Action *a = malloc(sizeof(Action));
    a->type = ACCEPTACT;
    a->state = x;
    a->d.Accept.nRules = n;
    a->d.Accept.saves = s;
    a->d.Accept.rules = r;
    x->action = a;
    return a;
}

static void doLinear(FILE *o, unsigned int i, Span *s, unsigned int n,
		     State *from, State *next, int *readCh){
    for(;;){
	State *bg = s[0].to;
	while(n >= 3 && s[2].to == bg && (s[1].ub - s[0].ub) == 1){
	    if(s[1].to == next && n == 3){
		indent(o, i);
		genIf(o, "!=", s[0].ub, readCh);
		genGoTo(o, from, bg, readCh, "\t");
		indent(o, i);
		genGoTo(o, from, next, readCh, "\t");
		return;
	    } else {
		indent(o, i);
		genIf(o, "==", s[0].ub, readCh);
		genGoTo(o, from, s[1].to, readCh, "\t");
	    }
	    n -= 2; s += 2;
	}
	if(n == 1){
	    indent(o, i);
	    genGoTo(o, from, s[0].to, readCh, "\t");
	    return;
	} else if(n == 2 && bg == next){
	    indent(o, i);
	    genIf(o, ">=", s[0].ub, readCh);
	    genGoTo(o, from, s[1].to, readCh, "\t");
	    indent(o, i);
	    genGoTo(o, from, next, readCh, "\t");
	    return;
	} else {
	    indent(o, i);
	    genIf(o, "<=", s[0].ub - 1, readCh);
	    genGoTo(o, from, bg, readCh, "\t");
	    n -= 1; s += 1;
	}
    }
    indent(o, i);
    genGoTo(o, from, next, readCh, "\t");
}

void
Go_genLinear(Go *g, FILE *o, State *from, State *next, int *readCh){
    doLinear(o, 0, g->span, g->nSpans, from, next, readCh);
}

static void genCases(FILE *o, unsigned int lb, Span *s){
    if(lb < s->ub){
	for(;;){
	    fputs("\tcase '", o); prtCh(o, lb); fputs("':", o);
	    if(++lb == s->ub)
		break;
	    fputs("\n", o); oline++;
	}
    }
}

void
Go_genSwitch(Go *g, FILE *o, State *from, State *next, int *readCh){
    if(g->nSpans <= 2){
	Go_genLinear(g, o, from, next, readCh);
    } else {
	State *def = g->span[g->nSpans-1].to;
	Span **sP = malloc(sizeof(Span*)*(g->nSpans-1)), **r, **s, **t;
	unsigned int i;

	t = &sP[0];
	for(i = 0; i < g->nSpans; ++i)
	    if(g->span[i].to != def)
		*(t++) = &g->span[i];

	    if (dFlag)
		fputs("\tYYDEBUG(-1, yych);\n", o);

#if 0
	if (*readCh) {
	    fputs("\tswitch((yych = *YYCURSOR)) {\n", o);
	    *readCh = 0;
	} else
#endif
	    fputs("\tswitch(yych){\n", o);
	oline++;
	while(t != &sP[0]){
	    State *to;
	    r = s = &sP[0];
	    if(*s == &g->span[0])
		genCases(o, 0, *s);
	    else
		genCases(o, (*s)[-1].ub, *s);
	    to = (*s)->to;
	    while(++s < t){
		if((*s)->to == to)
		    genCases(o, (*s)[-1].ub, *s);
		else
		    *(r++) = *s;
	    }
	    genGoTo(o, from, to, readCh, "\t");
	    t = r;
	}
	fputs("\tdefault:", o);
	genGoTo(o, from, def, readCh, "\t");
	fputs("\t}\n", o); oline++;

	free(sP);
    }
}

static void doBinary(FILE *o, unsigned int i, Span *s, unsigned int n,
		     State *from, State *next, int *readCh){
    if(n <= 4){
	doLinear(o, i, s, n, from, next, readCh);
    } else {
	unsigned int h = n/2;
	indent(o, i);
	genIf(o, "<=", s[h-1].ub - 1, readCh);
	fputs("{\n", o); oline++;
	doBinary(o, i+1, &s[0], h, from, next, readCh);
	indent(o, i); fputs("\t} else {\n", o); oline++;
	doBinary(o, i+1, &s[h], n - h, from, next, readCh);
	indent(o, i); fputs("\t}\n", o); oline++;
    }
}

void
Go_genBinary(Go *g, FILE *o, State *from, State *next, int *readCh){
    doBinary(o, 0, g->span, g->nSpans, from, next, readCh);
}

void
Go_genBase(Go *g, FILE *o, State *from, State *next, int *readCh){
    if(g->nSpans == 0)
	return;
    if(!sFlag){
	Go_genSwitch(g, o, from, next, readCh);
	return;
    }
    if(g->nSpans > 8){
	Span *bot = &g->span[0], *top = &g->span[g->nSpans-1];
	unsigned int util;
	if(bot[0].to == top[0].to){
	    util = (top[-1].ub - bot[0].ub)/(g->nSpans - 2);
	} else {
	    if(bot[0].ub > (top[0].ub - top[-1].ub)){
		util = (top[0].ub - bot[0].ub)/(g->nSpans - 1);
	    } else {
		util = top[-1].ub/(g->nSpans - 1);
	    }
	}
	if(util <= 2){
	    Go_genSwitch(g, o, from, next, readCh);
	    return;
	}
    }
    if(g->nSpans > 5){
	Go_genBinary(g, o, from, next, readCh);
    } else {
	Go_genLinear(g, o, from, next, readCh);
    }
}

void
Go_genGoto(Go *g, FILE *o, State *from, State *next, int *readCh){
    unsigned int i;
    if(bFlag){
	for(i = 0; i < g->nSpans; ++i){
	    State *to = g->span[i].to;
	    if(to && to->isBase){
		BitMap *b = BitMap_find(to);
		if(b && matches(b->go, b->on, g, to)){
		    Go go;
		    go.span = malloc(sizeof(Span)*g->nSpans);
		    Go_unmap(&go, g, to);
		    fprintf(o, "\tif(yybm[%u+", b->i);
#if 0
		    if (*readCh)
			fputs("(yych = *YYCURSOR)", o);
		    else
#endif
			fputs("yych", o);
		    fprintf(o, "] & %u) {\n", (unsigned int) b->m); oline++;
		    genGoTo(o, from, to, readCh, "\t\t");
		    fputs("\t}\n", o); oline++;
		    Go_genBase(&go, o, from, next, readCh);
		    free(go.span);
		    return;
		}
	    }
	}
    }
    Go_genBase(g, o, from, next, readCh);
}

void State_emit(State *s, FILE *o, int *readCh){
    if (vUsedLabels[s->label])
	fprintf(o, "yy%u:", s->label);
    if (dFlag)
	fprintf(o, "\n\tYYDEBUG(%u, *YYCURSOR);\n", s->label);
    Action_emit(s->action, o, readCh);
}

static unsigned int merge(Span *x0, State *fg, State *bg){
    Span *x = x0, *f = fg->go.span, *b = bg->go.span;
    unsigned int nf = fg->go.nSpans, nb = bg->go.nSpans;
    State *prev = NULL, *to;
    /* NB: we assume both spans are for same range */
    for(;;){
	if(f->ub == b->ub){
	    to = f->to == b->to? bg : f->to;
	    if(to == prev){
		--x;
	    } else {
		x->to = prev = to;
	    }
	    x->ub = f->ub;
	    ++x; ++f; --nf; ++b; --nb;
	    if(nf == 0 && nb == 0)
		return x - x0;
	}
	while(f->ub < b->ub){
	    to = f->to == b->to? bg : f->to;
	    if(to == prev){
		--x;
	    } else {
		x->to = prev = to;
	    }
	    x->ub = f->ub;
	    ++x; ++f; --nf;
	}
	while(b->ub < f->ub){
	    to = b->to == f->to? bg : f->to;
	    if(to == prev){
		--x;
	    } else {
		x->to = prev = to;
	    }
	    x->ub = b->ub;
	    ++x; ++b; --nb;
	}
    }
}

const unsigned int cInfinity = ~0;

typedef struct SCC {
    State	**top, **stk;
} SCC;

static void SCC_init(SCC*, unsigned int);
static SCC *SCC_new(unsigned int);
static void SCC_destroy(SCC*);
static void SCC_delete(SCC*);
static void SCC_traverse(SCC*, State*);

static void
SCC_init(SCC *s, unsigned int size)
{
    s->top = s->stk = malloc(sizeof(State*)*size);
}

static SCC *
SCC_new(unsigned int size){
    SCC *s = malloc(sizeof(SCC));
    s->top = s->stk = malloc(sizeof(State*)*size);
    return s;
}

static void
SCC_destroy(SCC *s){
    free(s->stk);
}

static void
SCC_delete(SCC *s){
    free(s->stk);
    free(s);
}

static void SCC_traverse(SCC *s, State *x){
    unsigned int k, i;

    *s->top = x;
    k = ++s->top - s->stk;
    x->depth = k;
    for(i = 0; i < x->go.nSpans; ++i){
	State *y = x->go.span[i].to;
	if(y){
	    if(y->depth == 0)
		SCC_traverse(s, y);
	    if(y->depth < x->depth)
		x->depth = y->depth;
	}
    }
    if(x->depth == k)
	do {
	    (*--s->top)->depth = cInfinity;
	    (*s->top)->link = x;
	} while(*s->top != x);
}

static unsigned int maxDist(State *s){
    unsigned int mm = 0, i;
    for(i = 0; i < s->go.nSpans; ++i){
	State *t = s->go.span[i].to;
	if(t){
	    unsigned int m = 1;
	    if(!t->link) {
		if (t->depth == -1)
		    t->depth = maxDist(t);
		m += t->depth;
	    }
	    if(m > mm)
		mm = m;
	}
    }
    return mm;
}

static void calcDepth(State *head){
    State *t, *s;
    for(s = head; s; s = s->next){
	if(s->link == s){
	    unsigned int i;
	    for(i = 0; i < s->go.nSpans; ++i){
		t = s->go.span[i].to;
		if(t && t->link == s)
		    goto inSCC;
	    }
	    s->link = NULL;
	} else {
	inSCC:
	    s->depth = maxDist(s);
	}
    }
}
 
void DFA_findSCCs(DFA *d){
    SCC scc;
    State *s;

    SCC_init(&scc, d->nStates);
    for(s = d->head; s; s = s->next){
	s->depth = 0;
	s->link = NULL;
    }

    for(s = d->head; s; s = s->next)
	if(!s->depth)
	    SCC_traverse(&scc, s);

    calcDepth(d->head);

    SCC_destroy(&scc);
}

void DFA_split(DFA *d, State *s){
    State *move = State_new();
    Action_new_Move(move);
    DFA_addState(d, &s->next, move);
    move->link = s->link;
    move->rule = s->rule;
    move->go = s->go;
    s->rule = NULL;
    s->go.nSpans = 1;
    s->go.span = malloc(sizeof(Span));
    s->go.span[0].ub = d->ubChar;
    s->go.span[0].to = move;
}

void DFA_emit(DFA *d, FILE *o){
    static unsigned int label = 0;
    State *s;
    unsigned int i, bitmap_brace = 0;
    unsigned int nRules = 0;
    unsigned int nSaves = 0;
    unsigned int *saves;
    unsigned int nOrgOline;
    State **rules;
    State *accept = NULL;
    Span *span;
    FILE *tmpo;
    int hasFillLabels;
    int maxFillIndexes, orgVFillIndexes;
    unsigned int start_label;

    hasFillLabels = (0<=vFillIndexes);
    if (hasFillLabels && label!=0) {
	fputs("re2c : error : multiple /*!re2c blocks aren't supported when -f is specified\n", stderr);
	exit(1);
    }

    DFA_findSCCs(d);
    d->head->link = d->head;

    maxFill = 1;
    for(s = d->head; s; s = s->next) {
	s->depth = maxDist(s);
	if (maxFill < s->depth)
	    maxFill = s->depth;
	if(s->rule && s->rule->d.RuleOp.accept >= nRules)
		nRules = s->rule->d.RuleOp.accept + 1;
    }

    saves = malloc(sizeof(unsigned int)*nRules);
    memset(saves, ~0, (nRules)*sizeof(unsigned int));

    /* mark backtracking points */
    for(s = d->head; s; s = s->next){
	RegExp *ignore = NULL;/*RuleOp*/
	if(s->rule){
	    for(i = 0; i < s->go.nSpans; ++i)
		if(s->go.span[i].to && !s->go.span[i].to->rule){
		    free(s->action);
		    if(saves[s->rule->d.RuleOp.accept] == ~0u)
			saves[s->rule->d.RuleOp.accept] = nSaves++;
		    Action_new_Save(s, saves[s->rule->d.RuleOp.accept]);
		    continue;
		}
	    ignore = s->rule;
	}
    }

    /* insert actions */
    rules = malloc(sizeof(State*)*nRules);
    memset(rules, 0, (nRules)*sizeof(State*));
    for(s = d->head; s; s = s->next){
	State *ow;
	if(!s->rule){
	    ow = accept;
	} else {
	    if(!rules[s->rule->d.RuleOp.accept]){
		State *n = State_new();
		Action_new_Rule(n, s->rule);
		rules[s->rule->d.RuleOp.accept] = n;
		DFA_addState(d, &s->next, n);
	    }
	    ow = rules[s->rule->d.RuleOp.accept];
	}
	for(i = 0; i < s->go.nSpans; ++i)
	    if(!s->go.span[i].to){
		if(!ow){
		    ow = accept = State_new();
		    Action_new_Accept(accept, nRules, saves, rules);
		    DFA_addState(d, &s->next, accept);
		}
		s->go.span[i].to = ow;
	    }
    }

    /* split ``base'' states into two parts */
    for(s = d->head; s; s = s->next){
	s->isBase = 0;
	if(s->link){
	    for(i = 0; i < s->go.nSpans; ++i){
		if(s->go.span[i].to == s){
		    s->isBase = 1;
		    DFA_split(d, s);
		    if(bFlag)
			BitMap_find_go(&s->next->go, s);
		    s = s->next;
		    break;
		}
	    }
	}
    }

    /* find ``base'' state, if possible */
    span = malloc(sizeof(Span)*(d->ubChar - d->lbChar));
    for(s = d->head; s; s = s->next){
	if(!s->link){
	    for(i = 0; i < s->go.nSpans; ++i){
		State *to = s->go.span[i].to;
		if(to && to->isBase){
		    unsigned int nSpans;
		    to = to->go.span[0].to;
		    nSpans = merge(span, s, to);
		    if(nSpans < s->go.nSpans){
			free(s->go.span);
			s->go.nSpans = nSpans;
			s->go.span = malloc(sizeof(Span)*nSpans);
			memcpy(s->go.span, span, nSpans*sizeof(Span));
		    }
		    break;
		}
	    }
	}
    }
    free(span);

    free(d->head->action);

    if(bFlag) {
	fputs("{\n", o);
	oline++;
	bitmap_brace = 1;
	BitMap_gen(o, d->lbChar, d->ubChar);
    }

    bUsedYYAccept = 0;

    start_label = label;

    Action_new_Enter(d->head, label++);

    for(s = d->head; s; s = s->next)
	s->label = label++;

    nOrgOline = oline;
    maxFillIndexes = vFillIndexes;
    orgVFillIndexes = vFillIndexes;
#ifdef _WIN32
    tmpo = win32_tmpfile();
#else
    tmpo = tmpfile();
#endif
    for(s = d->head; s; s = s->next){
	int readCh = 0;
	State_emit(s, tmpo, &readCh);
	Go_genGoto(&s->go, tmpo, s, s->next, &readCh);
    }
    fclose(tmpo);
    maxFillIndexes = vFillIndexes;
    vFillIndexes = orgVFillIndexes;
    oline = nOrgOline;

    fputs("\n", o);
    oline++;
    if (!iFlag)
	fprintf(o, "#line %u \"%s\"\n", oline++, outputFileName);

    if (!hasFillLabels) {
	fputs("{\n\tYYCTYPE yych;\n", o);
	oline += 2;
	if (bUsedYYAccept) {
	    fputs("\tunsigned int yyaccept;\n", o);
	    oline++;
	}
    } else {
	fputs("{\n\n", o);
	oline += 2;
    }

    if (!hasFillLabels) {
	fprintf(o, "\tgoto yy%u;\n", start_label);
	oline++;
	useLabel(label);
    } else {
	int i;
	fputs("\tswitch(YYGETSTATE()) {\n", o);
	fputs("\t\tcase -1: goto yy0;\n", o);

	for (i=0; i<maxFillIndexes; ++i)
	    fprintf(o, "\t\tcase %u: goto yyFillLabel%u;\n", i, i);

	fputs("\t\tdefault: /* abort() */;\n", o);
	fputs("\t}\n", o);
	fputs("yyNext:\n", o);

	oline += maxFillIndexes;
	oline += 5;
    }

    for(s = d->head; s; s = s->next){
	int readCh = 0;
	State_emit(s, o, &readCh);
	Go_genGoto(&s->go, o, s, s->next, &readCh);
    }
    fputs("}\n", o); oline++;
    if (bitmap_brace) {
	fputs("}\n", o);
	oline++;
    }

    BitMap_first = NULL;

    free(saves);
    free(rules);
}
