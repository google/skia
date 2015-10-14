#include <time.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "tools/re2c/globals.h"
#include "tools/re2c/parse.h"
#include "tools/re2c/dfa.h"

static Symbol *first = NULL;

void
Symbol_init(Symbol *r, const SubStr *str)
{
    r->next = first;
    Str_init(&r->name, str);
    r->re = NULL;
    first = r;
}

Symbol *
Symbol_find(const SubStr *str)
{
    Symbol *sym;
    for(sym = first; sym; sym = sym->next)
	if(SubStr_eq(&sym->name, str)) return sym;
    return Symbol_new(str);
}

/*
void showIns(FILE *o, const Ins *i, const Ins *base){
    o.width(3);
    o << &i - &base << ": ";
    switch(i.i.tag){
    case CHAR: {
	o << "match ";
	for(const Ins *j = &(&i)[1]; j < (Ins*) i.i.link; ++j)
	    prtCh(o, j->c.value);
	break;
    } case GOTO:
	o << "goto " << ((Ins*) i.i.link - &base);
	break;
    case FORK:
	o << "fork " << ((Ins*) i.i.link - &base);
	break;
    case CTXT:
	o << "term " << ((RuleOp*) i.i.link)->accept;
	break;
    case TERM:
	o << "term " << ((RuleOp*) i.i.link)->accept;
	break;
    }
    o << "\n";
}
*/

static unsigned int
AltOp_fixedLength(RegExp *r)
{
    unsigned int l1 = RegExp_fixedLength(r->d.AltCatOp.exp1);
    /* XXX? Should be exp2? */
    unsigned int l2 = RegExp_fixedLength(r->d.AltCatOp.exp1);
    if(l1 != l2 || l1 == ~0u)
	return ~0u;
    return l1;
}

static unsigned int
CatOp_fixedLength(RegExp *r)
{
    unsigned int l1, l2;
    if((l1 = RegExp_fixedLength(r->d.AltCatOp.exp1)) != ~0u )
        if((l2 = RegExp_fixedLength(r->d.AltCatOp.exp2)) != ~0u)
	    return l1+l2;
    return ~0u;
}

unsigned int
RegExp_fixedLength(RegExp *r)
{
    switch (r->type) {
	case NULLOP:
	    return 0;
	case MATCHOP:
	    return 1;
	case ALTOP:
	    return AltOp_fixedLength(r);
	case CATOP:
	    return CatOp_fixedLength(r);
	default:
	    return ~0u;
    }
    return ~0u;
}

void
RegExp_calcSize(RegExp *re, Char *rep)
{
    Range *r;
    unsigned int c;

    switch (re->type) {
	case NULLOP:
	    re->size = 0;
	    break;
	case MATCHOP:
	    re->size = 1;
	    for(r = re->d.match; r; r = r->next)
		for(c = r->lb; c < r->ub; ++c)
		    if(rep[c] == c)
			++re->size;
	    break;
	case RULEOP:
	    RegExp_calcSize(re->d.RuleOp.exp, rep);
	    RegExp_calcSize(re->d.RuleOp.ctx, rep);
	    re->size = re->d.RuleOp.exp->size + re->d.RuleOp.ctx->size + 1;
	    break;
	case ALTOP:
	    RegExp_calcSize(re->d.AltCatOp.exp1, rep);
	    RegExp_calcSize(re->d.AltCatOp.exp2, rep);
	    re->size = re->d.AltCatOp.exp1->size
		       + re->d.AltCatOp.exp2->size + 2;
	    break;
	case CATOP:
	    RegExp_calcSize(re->d.AltCatOp.exp1, rep);
	    RegExp_calcSize(re->d.AltCatOp.exp2, rep);
	    re->size = re->d.AltCatOp.exp1->size + re->d.AltCatOp.exp2->size;
	    break;
	case CLOSEOP:
	    RegExp_calcSize(re->d.exp, rep);
	    re->size = re->d.exp->size + 1;
	    break;
	case CLOSEVOP:
	    RegExp_calcSize(re->d.CloseVOp.exp, rep);

	    if (re->d.CloseVOp.max >= 0)
		re->size = (re->d.CloseVOp.exp->size * re->d.CloseVOp.min) +
		    ((1 + re->d.CloseVOp.exp->size) *
		     (re->d.CloseVOp.max - re->d.CloseVOp.min));
	    else
		re->size = (re->d.CloseVOp.exp->size * re->d.CloseVOp.min) + 1;
	    break;
    }
}

static void
MatchOp_compile(RegExp *re, Char *rep, Ins *i)
{
    Ins *j;
    unsigned int bump;
    Range *r;
    unsigned int c;

    i->i.tag = CHAR;
    i->i.link = &i[re->size];
    j = &i[1];
    bump = re->size;
    for(r = re->d.match; r; r = r->next){
	for(c = r->lb; c < r->ub; ++c){
	    if(rep[c] == c){
		j->c.value = c;
		j->c.bump = --bump;
		j++;
	    }
	}
    }
}

static void
AltOp_compile(RegExp *re, Char *rep, Ins *i){
    Ins *j;

    i->i.tag = FORK;
    j = &i[re->d.AltCatOp.exp1->size + 1];
    i->i.link = &j[1];
    RegExp_compile(re->d.AltCatOp.exp1, rep, &i[1]);
    j->i.tag = GOTO;
    j->i.link = &j[re->d.AltCatOp.exp2->size + 1];
    RegExp_compile(re->d.AltCatOp.exp2, rep, &j[1]);
}

void
RegExp_compile(RegExp *re, Char *rep, Ins *i)
{
    Ins *jumppoint;
    int st = 0;

    switch (re->type) {
	case NULLOP:
	    break;
	case MATCHOP:
	    MatchOp_compile(re, rep, i);
	    break;
	case RULEOP:
	    re->d.RuleOp.ins = i;
	    RegExp_compile(re->d.RuleOp.exp, rep, &i[0]);
	    i += re->d.RuleOp.exp->size;
	    RegExp_compile(re->d.RuleOp.ctx, rep, &i[0]);
	    i += re->d.RuleOp.ctx->size;
	    i->i.tag = TERM;
	    i->i.link = re;
	    break;
	case ALTOP:
	    AltOp_compile(re, rep, i);
	    break;
	case CATOP:
	    RegExp_compile(re->d.AltCatOp.exp1, rep, &i[0]);
	    RegExp_compile(re->d.AltCatOp.exp2, rep,
			   &i[re->d.AltCatOp.exp1->size]);
	    break;
	case CLOSEOP:
	    RegExp_compile(re->d.exp, rep, &i[0]);
	    i += re->d.exp->size;
	    i->i.tag = FORK;
	    i->i.link = i - re->d.exp->size;
	    break;
	case CLOSEVOP:
	    jumppoint = i + ((1 + re->d.CloseVOp.exp->size) *
			     (re->d.CloseVOp.max - re->d.CloseVOp.min));
	    for(st = re->d.CloseVOp.min; st < re->d.CloseVOp.max; st++) {
		i->i.tag = FORK;
		i->i.link = jumppoint;
		i+=1;
		RegExp_compile(re->d.CloseVOp.exp, rep, &i[0]);
		i += re->d.CloseVOp.exp->size;
	    }
	    for(st = 0; st < re->d.CloseVOp.min; st++) {
		RegExp_compile(re->d.CloseVOp.exp, rep, &i[0]);
		i += re->d.CloseVOp.exp->size;
		if(re->d.CloseVOp.max < 0 && st == 0) {
		    i->i.tag = FORK;
		    i->i.link = i - re->d.CloseVOp.exp->size;
		    i++;
		}
	    }
	    break;
    }
}

static void
MatchOp_split(RegExp *re, CharSet *s)
{
    Range *r;
    unsigned int c;

    for(r = re->d.match; r; r = r->next){
	for(c = r->lb; c < r->ub; ++c){
	    CharPtn *x = s->rep[c], *a = x->nxt;
	    if(!a){
		if(x->card == 1)
		    continue;
		x->nxt = a = s->freeHead;
		if(!(s->freeHead = s->freeHead->nxt))
		    s->freeTail = &s->freeHead;
		a->nxt = NULL;
		x->fix = s->fix;
		s->fix = x;
	    }
	    if(--(x->card) == 0){
		*s->freeTail = x;
		*(s->freeTail = &x->nxt) = NULL;
	    }
	    s->rep[c] = a;
	    ++(a->card);
	}
    }
    for(; s->fix; s->fix = s->fix->fix)
	if(s->fix->card)
	    s->fix->nxt = NULL;
}

void
RegExp_split(RegExp *re, CharSet *s)
{
    switch (re->type) {
	case NULLOP:
	    break;
	case MATCHOP:
	    MatchOp_split(re, s);
	    break;
	case RULEOP:
	    RegExp_split(re->d.RuleOp.exp, s);
	    RegExp_split(re->d.RuleOp.ctx, s);
	    break;
	case ALTOP:
	    /* FALLTHROUGH */
	case CATOP:
	    RegExp_split(re->d.AltCatOp.exp1, s);
	    RegExp_split(re->d.AltCatOp.exp2, s);
	    break;
	case CLOSEOP:
	    RegExp_split(re->d.exp, s);
	    break;
	case CLOSEVOP:
	    RegExp_split(re->d.CloseVOp.exp, s);
	    break;
    }
}

void
RegExp_display(RegExp *re, FILE *o)
{
    switch (re->type) {
	case NULLOP:
	    fputc('_', o);
	    break;
	case MATCHOP:
	    Range_out(o, re->d.match);
	    break;
	case RULEOP:
	    RegExp_display(re->d.RuleOp.exp, o);
	    fputc('/', o);
	    RegExp_display(re->d.RuleOp.ctx, o);
	    fputc(';', o);
	    break;
	case ALTOP:
	    RegExp_display(re->d.AltCatOp.exp1, o);
	    fputc('|', o);
	    RegExp_display(re->d.AltCatOp.exp2, o);
	    break;
	case CATOP:
	    RegExp_display(re->d.AltCatOp.exp1, o);
	    RegExp_display(re->d.AltCatOp.exp2, o);
	    break;
	case CLOSEOP:
	    RegExp_display(re->d.exp, o);
	    fputc('+', o);
	    break;
    }
}

void
Range_out(FILE *o, const Range *r)
{
    if(!r)
	return;

    if((r->ub - r->lb) == 1){
	prtCh(o, r->lb);
    } else {
	prtCh(o, r->lb);
	fputc('-', o);
	prtCh(o, r->ub-1);
    }
    Range_out(o, r->next);
}

static Range *doUnion(Range *r1, Range *r2){
    Range *r, **rP = &r;
    for(;;){
	Range *s;
	if(r1->lb <= r2->lb){
	    s = Range_new_copy(r1);
	} else {
	    s = Range_new_copy(r2);
	}
	*rP = s;
	rP = &s->next;
	for(;;){
	    if(r1->lb <= r2->lb){
		if(r1->lb > s->ub)
		    break;
		if(r1->ub > s->ub)
		    s->ub = r1->ub;
		if(!(r1 = r1->next)){
		    unsigned int ub = 0;
		    for(; r2 && r2->lb <= s->ub; r2 = r2->next)
			ub = r2->ub;
		    if(ub > s->ub)
			s->ub = ub;
		    *rP = r2;
		    return r;
		}
	    } else {
		if(r2->lb > s->ub)
		    break;
		if(r2->ub > s->ub)
		    s->ub = r2->ub;
		if(!(r2 = r2->next)){
		    unsigned int ub = 0;
		    for(; r1 && r1->lb <= s->ub; r1 = r1->next)
			ub = r1->ub;
		    if(ub > s->ub)
			s->ub = ub;
		    *rP = r1;
		    return r;
		}
	    }
	}
    }
    *rP = NULL;
    return r;
}

static Range *doDiff(Range *r1, Range *r2){
    Range *r, *s, **rP = &r;
    for(; r1; r1 = r1->next){
	unsigned int lb = r1->lb;
	for(; r2 && r2->ub <= r1->lb; r2 = r2->next);
	for(; r2 && r2->lb <  r1->ub; r2 = r2->next){
	    if(lb < r2->lb){
		*rP = s = Range_new(lb, r2->lb);
		rP = &s->next;
	    }
	    if((lb = r2->ub) >= r1->ub)
		goto noMore;
	}
	*rP = s = Range_new(lb, r1->ub);
	rP = &s->next;
    noMore:;
    }
    *rP = NULL;
    return r;
}

static RegExp *merge(RegExp *m1, RegExp *m2){
    if(!m1)
	return m2;
    if(!m2)
	return m1;
    return RegExp_new_MatchOp(doUnion(m1->d.match, m2->d.match));
}

RegExp *mkDiff(RegExp *e1, RegExp *e2){
    RegExp *m1, *m2;
    Range *r;
    if(!(m1 = RegExp_isA(e1, MATCHOP)))
	return NULL;
    if(!(m2 = RegExp_isA(e2, MATCHOP)))
	return NULL;
    r = doDiff(m1->d.match, m2->d.match);
    return r? RegExp_new_MatchOp(r) : RegExp_new_NullOp();
}

static RegExp *doAlt(RegExp *e1, RegExp *e2){
    if(!e1)
	return e2;
    if(!e2)
	return e1;
    return RegExp_new_AltOp(e1, e2);
}

RegExp *mkAlt(RegExp *e1, RegExp *e2){
    RegExp *a;
    RegExp *m1, *m2;
    if((a = RegExp_isA(e1, ALTOP))){
	if((m1 = RegExp_isA(a->d.AltCatOp.exp1, MATCHOP)))
	    e1 = a->d.AltCatOp.exp2;
    } else if((m1 = RegExp_isA(e1, MATCHOP))){
	    e1 = NULL;
    }
    if((a = RegExp_isA(e2, ALTOP))){
	if((m2 = RegExp_isA(a->d.AltCatOp.exp1, MATCHOP)))
	    e2 = a->d.AltCatOp.exp2;
    } else if((m2 = RegExp_isA(e2, MATCHOP))){
	    e2 = NULL;
    }
    return doAlt(merge(m1, m2), doAlt(e1, e2));
}

static unsigned char unescape(SubStr *s){
    unsigned char c;
    unsigned char v;
    s->len--;
    if((c = *s->str++) != '\\' || s->len == 0)
	return xlat[c];
    s->len--;
    switch(c = *s->str++){
    case 'n':
	return xlat['\n'];
    case 't':
	return xlat['\t'];
    case 'v':
	return xlat['\v'];
    case 'b':
	return xlat['\b'];
    case 'r':
	return xlat['\r'];
    case 'f':
	return xlat['\f'];
    case 'a':
	return xlat['\a'];
    case '0': case '1': case '2': case '3':
    case '4': case '5': case '6': case '7': {
	v = c - '0';
	for(; s->len != 0 && '0' <= (c = *s->str) && c <= '7'; s->len--, s->str++)
	    v = v*8 + (c - '0');
	return v;
    } default:
	return xlat[c];
    }
}

static Range *getRange(SubStr *s){
    unsigned char lb = unescape(s), ub;
    if(s->len < 2 || *s->str != '-'){
	ub = lb;
    } else {
	s->len--; s->str++;
	ub = unescape(s);
	if(ub < lb){
	    unsigned char tmp;
	    tmp = lb; lb = ub; ub = tmp;
	}
    }
    return Range_new(lb, ub+1);
}

static RegExp *matchChar(unsigned int c){
    return RegExp_new_MatchOp(Range_new(c, c+1));
}

RegExp *strToRE(SubStr s){
    RegExp *re;
    s.len -= 2; s.str += 1;
    if(s.len == 0)
	return RegExp_new_NullOp();
    re = matchChar(unescape(&s));
    while(s.len > 0)
	re = RegExp_new_CatOp(re, matchChar(unescape(&s)));
    return re;
}

RegExp *strToCaseInsensitiveRE(SubStr s){
    unsigned char c;
    RegExp *re, *reL, *reU;
    s.len -= 2; s.str += 1;
    if(s.len == 0)
	return RegExp_new_NullOp();
    c = unescape(&s);
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
	reL = matchChar(tolower(c));
	reU = matchChar(toupper(c));
	re = mkAlt(reL, reU);
    } else {
	re = matchChar(c);
    }
    while(s.len > 0) {
	c = unescape(&s);
	if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
	    reL = matchChar(tolower(c));
	    reU = matchChar(toupper(c));
	    re = RegExp_new_CatOp(re, mkAlt(reL, reU));
    	} else {
	    re = RegExp_new_CatOp(re, matchChar(c));
	}
    }
    return re;
}

RegExp *ranToRE(SubStr s){
    Range *r;
    s.len -= 2; s.str += 1;
    if(s.len == 0)
	return RegExp_new_NullOp();
    r = getRange(&s);
    while(s.len > 0)
	r = doUnion(r, getRange(&s));
    return RegExp_new_MatchOp(r);
}

RegExp *invToRE(SubStr s)
{
    RegExp *any, *ran, *inv;
    SubStr *ss;


    s.len--;
    s.str++;

    ss = SubStr_new("[\\000-\\377]", strlen("[\\000-\\377]"));
    any = ranToRE(*ss);
    free(ss);
    if (s.len <= 2)
	return any;

    ran = ranToRE(s);
    inv = mkDiff(any, ran);

    free(ran);
    free(any);

    return inv;
}

RegExp *mkDot()
{
    SubStr *ss = SubStr_new("[\\000-\\377]", strlen("[\\000-\\377]"));
    RegExp * any = ranToRE(*ss);
    RegExp * ran = matchChar('\n');
    RegExp * inv = mkDiff(any, ran);

    free(ss);
    free(ran);
    free(any);

    return inv;
}

RegExp *
RegExp_new_RuleOp(RegExp *e, RegExp *c, Token *t, unsigned int a)
{
    RegExp *r = malloc(sizeof(RegExp));
    r->type = RULEOP;
    r->d.RuleOp.exp = e;
    r->d.RuleOp.ctx = c;
    r->d.RuleOp.ins = NULL;
    r->d.RuleOp.accept = a;
    r->d.RuleOp.code = t;
    return r;
}

static void optimize(Ins *i){
    while(!isMarked(i)){
	mark(i);
	if(i->i.tag == CHAR){
	    i = (Ins*) i->i.link;
	} else if(i->i.tag == GOTO || i->i.tag == FORK){
	    Ins *target = (Ins*) i->i.link;
	    optimize(target);
	    if(target->i.tag == GOTO)
		i->i.link = target->i.link == target? i : target;
	    if(i->i.tag == FORK){
		Ins *follow = (Ins*) &i[1];
		optimize(follow);
		if(follow->i.tag == GOTO && follow->i.link == follow){
		    i->i.tag = GOTO;
		} else if(i->i.link == i){
		    i->i.tag = GOTO;
		    i->i.link = follow;
		}
	    }
	    return;
	} else {
	    ++i;
	}
    }
}

void genCode(FILE *o, RegExp *re){
    CharSet cs;
    unsigned int j;
    Char rep[nChars];
    Ins *ins, *eoi;
    DFA *dfa;

    memset(&cs, 0, sizeof(cs));
    for(j = 0; j < nChars; ++j){
	cs.rep[j] = &cs.ptn[0];
	cs.ptn[j].nxt = &cs.ptn[j+1];
    }
    cs.freeHead = &cs.ptn[1];
    *(cs.freeTail = &cs.ptn[nChars-1].nxt) = NULL;
    cs.ptn[0].card = nChars;
    cs.ptn[0].nxt = NULL;
    RegExp_split(re, &cs);
/*
    for(unsigned int k = 0; k < nChars;){
	for(j = k; ++k < nChars && cs.rep[k] == cs.rep[j];);
	printSpan(cerr, j, k);
	cerr << "\t" << cs.rep[j] - &cs.ptn[0] << endl;
    }
*/
    for(j = 0; j < nChars; ++j){
	if(!cs.rep[j]->nxt)
	    cs.rep[j]->nxt = &cs.ptn[j];
	rep[j] = (Char) (cs.rep[j]->nxt - &cs.ptn[0]);
    }

    RegExp_calcSize(re, rep);
    ins = malloc(sizeof(Ins)*(re->size+1));
    memset(ins, 0, (re->size+1)*sizeof(Ins));
    RegExp_compile(re, rep, ins);
    eoi = &ins[re->size];
    eoi->i.tag = GOTO;
    eoi->i.link = eoi;

    optimize(ins);
    for(j = 0; j < re->size;){
	unmark(&ins[j]);
	if(ins[j].i.tag == CHAR){
	    j = (Ins*) ins[j].i.link - ins;
	} else {
	    j++;
	}
    }

    dfa = DFA_new(ins, re->size, 0, 256, rep);
    DFA_emit(dfa, o);
    DFA_delete(dfa);
    free(ins);
}
