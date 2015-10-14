#ifndef re2c_re_h
#define re2c_re_h

#include <stdio.h>
#include "tools/re2c/token.h"
#include "tools/re2c/ins.h"

typedef struct extop {
    char    op;
    int	    minsize;
    int	    maxsize;
} ExtOp;

typedef struct CharPtn {
    unsigned int	card;
    struct CharPtn	*fix;
    struct CharPtn	*nxt;
} CharPtn;

typedef struct CharSet {
    CharPtn	*fix;
    CharPtn	*freeHead, **freeTail;
    CharPtn	*rep[nChars];
    CharPtn	ptn[nChars];
} CharSet;

typedef struct Range {
    struct Range	*next;
    unsigned int	lb, ub;		/* [lb,ub) */
} Range;

static void
Range_init(Range *r, unsigned int l, unsigned int u)
{
    r->next = NULL;
    r->lb = l;
    r->ub = u;
}

static Range *
Range_new(unsigned int l, unsigned int u)
{
    Range *r = malloc(sizeof(Range));
    r->next = NULL;
    r->lb = l;
    r->ub = u;
    return r;
}

static void
Range_copy(Range *ro, const Range *r)
{
    ro->next = NULL;
    ro->lb = r->lb;
    ro->ub = r->ub;
}

static Range *
Range_new_copy(Range *r)
{
    Range *ro = malloc(sizeof(Range));
    ro->next = NULL;
    ro->lb = r->lb;
    ro->ub = r->ub;
    return ro;
}

void Range_out(FILE *, const Range *);

typedef enum {
	NULLOP = 1,
	MATCHOP,
	RULEOP,
	ALTOP,
	CATOP,
	CLOSEOP,
	CLOSEVOP
} RegExpType;

typedef struct RegExp {
    RegExpType	type;
    unsigned int	size;
    union {
	/* for MatchOp */
	Range	*match;
	/* for RuleOp */
	struct {
	    struct RegExp	*exp;
	    struct RegExp	*ctx;
	    Ins		*ins;
	    unsigned int	accept;
	    Token	*code;
	    unsigned int	line;
	} RuleOp;
	/* for AltOp and CatOp*/
	struct {
	    struct RegExp	*exp1, *exp2;
	} AltCatOp;
	/* for CloseOp */
	struct RegExp	*exp;
	/* for CloseVOp*/
	struct {
	    struct RegExp	*exp;
	    int		min;
	    int		max;
	} CloseVOp;
    } d;
} RegExp;

static RegExp *
RegExp_isA(RegExp *r, RegExpType t)
{
    return r->type == t ? r : NULL;
}

void RegExp_split(RegExp*, CharSet*);
void RegExp_calcSize(RegExp*, Char*);
unsigned int RegExp_fixedLength(RegExp*);
void RegExp_compile(RegExp*, Char*, Ins*);
void RegExp_display(RegExp*, FILE *);

static RegExp *
RegExp_new_NullOp(void)
{
    RegExp *r = malloc(sizeof(RegExp));
    r->type = NULLOP;
    return r;
}

static RegExp *
RegExp_new_MatchOp(Range *m)
{
    RegExp *r = malloc(sizeof(RegExp));
    r->type = MATCHOP;
    r->d.match = m;
    return r;
}

RegExp *RegExp_new_RuleOp(RegExp*, RegExp*, Token*, unsigned int);

static RegExp *
RegExp_new_AltOp(RegExp *e1, RegExp *e2)
{
    RegExp *r = malloc(sizeof(RegExp));
    r->type = ALTOP;
    r->d.AltCatOp.exp1 = e1;
    r->d.AltCatOp.exp2 = e2;
    return r;
}

static RegExp *
RegExp_new_CatOp(RegExp *e1, RegExp *e2)
{
    RegExp *r = malloc(sizeof(RegExp));
    r->type = CATOP;
    r->d.AltCatOp.exp1 = e1;
    r->d.AltCatOp.exp2 = e2;
    return r;
}

static RegExp *
RegExp_new_CloseOp(RegExp *e)
{
    RegExp *r = malloc(sizeof(RegExp));
    r->type = CLOSEOP;
    r->d.exp = e;
    return r;
}

static RegExp *
RegExp_new_CloseVOp(RegExp *e, int lb, int ub)
{
    RegExp *r = malloc(sizeof(RegExp));
    r->type = CLOSEVOP;
    r->d.CloseVOp.exp = e;
    r->d.CloseVOp.min = lb;
    r->d.CloseVOp.max = ub;
    return r;
}

extern void genCode(FILE *, RegExp*);
extern RegExp *mkDiff(RegExp*, RegExp*);
extern RegExp *mkDot(void);
extern RegExp *strToRE(SubStr);
extern RegExp *strToCaseInsensitiveRE(SubStr);
extern RegExp *ranToRE(SubStr);
extern RegExp *invToRE(SubStr);

extern RegExp *mkAlt(RegExp*, RegExp*);

#endif
