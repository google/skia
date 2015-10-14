#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "tools/re2c/globals.h"
#include "tools/re2c/substr.h"
#include "tools/re2c/dfa.h"

#define octCh(c) ('0' + c%8)

void prtCh(FILE *o, unsigned char c){
    unsigned char oc = talx[c];
    switch(oc){
    case '\'': fputs("\\'", o); break;
    case '\n': fputs("\\n", o); break;
    case '\t': fputs("\\t", o); break;
    case '\v': fputs("\\v", o); break;
    case '\b': fputs("\\b", o); break;
    case '\r': fputs("\\r", o); break;
    case '\f': fputs("\\f", o); break;
    case '\a': fputs("\\a", o); break;
    case '\\': fputs("\\\\", o); break;
    default:
	if(isprint(oc))
	    fputc(oc, o);
	else
	    fprintf(o, "\\%c%c%c", octCh(c/64), octCh(c/8), octCh(c));
    }
}

void printSpan(FILE *o, unsigned int lb, unsigned int ub){
    if(lb > ub)
	fputc('*', o);
    fputc('[', o);
    if((ub - lb) == 1){
	prtCh(o, lb);
    } else {
	prtCh(o, lb);
	fputc('-', o);
	prtCh(o, ub-1);
    }
    fputc(']', o);
}

unsigned int
Span_show(Span *s, FILE *o, unsigned int lb)
{
    if(s->to){
	printSpan(o, lb, s->ub);
	fprintf(o, " %u; ", s->to->label);
    }
    return s->ub;
}

void
State_out(FILE *o, const State *s){
    unsigned int lb, i;
    fprintf(o, "state %u", s->label);
    if(s->rule)
	fprintf(o, " accepts %u", s->rule->d.RuleOp.accept);
    fputs("\n", o); oline++;
    lb = 0;
    for(i = 0; i < s->go.nSpans; ++i)
	lb = Span_show(&s->go.span[i], o, lb);
}

void
DFA_out(FILE *o, const DFA *dfa){
    State *s;
    for(s = dfa->head; s; s = s->next) {
	State_out(o, s);
	fputs("\n\n", o); oline+=2;
    }
}

State *
State_new(void)
{
    State *s = malloc(sizeof(State));
    s->label = 0;
    s->rule = NULL;
    s->next = NULL;
    s->link = NULL;
    s->depth = 0;
    s->kCount = 0;
    s->kernel = NULL;
    s->isBase = 0;
    s->action = NULL;
    s->go.nSpans = 0;
    s->go.span = NULL;
    return s;
}

void
State_delete(State *s)
{
    if (s->kernel)
	free(s->kernel);
    if (s->go.span)
	free(s->go.span);
    free(s);
}

static Ins **closure(Ins **cP, Ins *i){
    while(!isMarked(i)){
	mark(i);
	*(cP++) = i;
	if(i->i.tag == FORK){
	    cP = closure(cP, i + 1);
	    i = (Ins*) i->i.link;
	} else if(i->i.tag == GOTO){
	    i = (Ins*) i->i.link;
	} else
	    break;
    }
    return cP;
}

typedef struct GoTo {
    Char	ch;
    void	*to;
} GoTo;

DFA *
DFA_new(Ins *ins, unsigned int ni, unsigned int lb, unsigned int ub, Char *rep)
{
    DFA *d = malloc(sizeof(DFA));
    Ins **work = malloc(sizeof(Ins*)*(ni+1));
    unsigned int nc = ub - lb;
    GoTo *goTo = malloc(sizeof(GoTo)*nc);
    Span *span = malloc(sizeof(Span)*nc);

    d->lbChar = lb;
    d->ubChar = ub;
    memset((char*) goTo, 0, nc*sizeof(GoTo));
    d->tail = &d->head;
    d->head = NULL;
    d->nStates = 0;
    d->toDo = NULL;
    DFA_findState(d, work, closure(work, &ins[0]) - work);
    while(d->toDo){
	State *s = d->toDo;

	Ins **cP, **iP, *i;
	unsigned int nGoTos = 0;
	unsigned int j;

	d->toDo = s->link;
	s->rule = NULL;
	for(iP = s->kernel; (i = *iP); ++iP){
	    if(i->i.tag == CHAR){
		Ins *j2;
		for(j2 = i + 1; j2 < (Ins*) i->i.link; ++j2){
		    if(!(j2->c.link = goTo[j2->c.value - lb].to))
			goTo[nGoTos++].ch = j2->c.value;
		    goTo[j2->c.value - lb].to = j2;
		}
	    } else if(i->i.tag == TERM){
		if(!s->rule || ((RegExp *)i->i.link)->d.RuleOp.accept < s->rule->d.RuleOp.accept)
		    s->rule = (RegExp *)i->i.link;
	    }
	}

	for(j = 0; j < nGoTos; ++j){
	    GoTo *go = &goTo[goTo[j].ch - lb];
	    i = (Ins*) go->to;
	    for(cP = work; i; i = (Ins*) i->c.link)
		cP = closure(cP, i + i->c.bump);
	    go->to = DFA_findState(d, work, cP - work);
	}

	s->go.nSpans = 0;
	for(j = 0; j < nc;){
	    State *to = (State*) goTo[rep[j]].to;
	    while(++j < nc && goTo[rep[j]].to == to);
	    span[s->go.nSpans].ub = lb + j;
	    span[s->go.nSpans].to = to;
	    s->go.nSpans++;
	}

	for(j = nGoTos; j-- > 0;)
	    goTo[goTo[j].ch - lb].to = NULL;

	s->go.span = malloc(sizeof(Span)*s->go.nSpans);
	memcpy((char*) s->go.span, (char*) span, s->go.nSpans*sizeof(Span));

	Action_new_Match(s);

    }
    free(work);
    free(goTo);
    free(span);

    return d;
}

void
DFA_delete(DFA *d){
    State *s;
    while((s = d->head)){
	d->head = s->next;
	State_delete(s);
    }
}

void DFA_addState(DFA *d, State **a, State *s){
    s->label = d->nStates++;
    s->next = *a;
    *a = s;
    if(a == d->tail)
	d->tail = &s->next;
}

State *DFA_findState(DFA *d, Ins **kernel, unsigned int kCount){
    Ins **cP, **iP, *i;
    State *s;

    kernel[kCount] = NULL;

    cP = kernel;
    for(iP = kernel; (i = *iP); ++iP){
	 if(i->i.tag == CHAR || i->i.tag == TERM){
	     *cP++ = i;
	} else {
	     unmark(i);
	}
    }
    kCount = cP - kernel;
    kernel[kCount] = NULL;

    for(s = d->head; s; s = s->next){
	 if(s->kCount == kCount){
	     for(iP = s->kernel; (i = *iP); ++iP)
		 if(!isMarked(i))
		     goto nextState;
	     goto unmarkAll;
	 }
	 nextState:;
    }

    s = State_new();
    DFA_addState(d, d->tail, s);
    s->kCount = kCount;
    s->kernel = malloc(sizeof(Ins*)*(kCount+1));
    memcpy(s->kernel, kernel, (kCount+1)*sizeof(Ins*));
    s->link = d->toDo;
    d->toDo = s;

unmarkAll:
    for(iP = kernel; (i = *iP); ++iP)
	 unmark(i);

    return s;
}
