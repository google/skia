#ifndef re2c_dfa_h
#define re2c_dfa_h

#include <stdio.h>
#include "tools/re2c/re.h"

extern void prtCh(FILE *, unsigned char);
extern void printSpan(FILE *, unsigned int, unsigned int);

struct DFA;
struct State;

typedef enum {
    MATCHACT = 1,
    ENTERACT,
    SAVEMATCHACT,
    MOVEACT,
    ACCEPTACT,
    RULEACT
} ActionType;

typedef struct Action {
    struct State	*state;
    ActionType		type;
    union {
	/* data for Enter */
	unsigned int		label;
	/* data for SaveMatch */
	unsigned int		selector;
	/* data for Accept */
	struct {
	    unsigned int		nRules;
	    unsigned int		*saves;
	    struct State	**rules;
	} Accept;
	/* data for Rule */
	RegExp		*rule;	/* RuleOp */
    } d;
} Action;

void Action_emit(Action*, FILE *, int *);

typedef struct Span {
    unsigned int		ub;
    struct State	*to;
} Span;

unsigned int Span_show(Span*, FILE *, unsigned int);

typedef struct Go {
    unsigned int	nSpans;
    Span		*span;
} Go;

typedef struct State {
    unsigned int	label;
    RegExp		*rule;	/* RuleOp */
    struct State	*next;
    struct State	*link;
    unsigned int	depth;		/* for finding SCCs */
    unsigned int	kCount;
    Ins			**kernel;
    unsigned int	isBase:1;
    Go			go;
    Action		*action;
} State;

void Go_genGoto(Go*, FILE *, State*, State*, int*);
void Go_genBase(Go*, FILE *, State*, State*, int*);
void Go_genLinear(Go*, FILE *, State*, State*, int*);
void Go_genBinary(Go*, FILE *, State*, State*, int*);
void Go_genSwitch(Go*, FILE *, State*, State*, int*);
void Go_compact(Go*);
void Go_unmap(Go*, Go*, State*);

State *State_new(void);
void State_delete(State*);
void State_emit(State*, FILE *, int *);
void State_out(FILE *, const State*);

typedef struct DFA {
    unsigned int	lbChar;
    unsigned int	ubChar;
    unsigned int	nStates;
    State		*head, **tail;
    State		*toDo;
} DFA;

DFA *DFA_new(Ins*, unsigned int, unsigned int, unsigned int, Char*);
void DFA_delete(DFA*);
void DFA_addState(DFA*, State**, State*);
State *DFA_findState(DFA*, Ins**, unsigned int);
void DFA_split(DFA*, State*);

void DFA_findSCCs(DFA*);
void DFA_emit(DFA*, FILE *);
void DFA_out(FILE *, const DFA*);

static Action *
Action_new_Match(State *s)
{
    Action *a = malloc(sizeof(Action));
    a->type = MATCHACT;
    a->state = s;
    s->action = a;
    return a;
}

static Action *
Action_new_Enter(State *s, unsigned int l)
{
    Action *a = malloc(sizeof(Action));
    a->type = ENTERACT;
    a->state = s;
    a->d.label = l;
    s->action = a;
    return a;
}

static Action *
Action_new_Save(State *s, unsigned int i)
{
    Action *a = malloc(sizeof(Action));
    a->type = SAVEMATCHACT;
    a->state = s;
    a->d.selector = i;
    s->action = a;
    return a;
}

static Action *
Action_new_Move(State *s)
{
    Action *a = malloc(sizeof(Action));
    a->type = MOVEACT;
    a->state = s;
    s->action = a;
    return a;
}

Action *Action_new_Accept(State*, unsigned int, unsigned int*, State**);

static Action *
Action_new_Rule(State *s, RegExp *r) /* RuleOp */
{
    Action *a = malloc(sizeof(Action));
    a->type = RULEACT;
    a->state = s;
    a->d.rule = r;
    s->action = a;
    return a;
}

static int
Action_isRule(Action *a)
{
    return a->type == RULEACT;
}

static int
Action_isMatch(Action *a)
{
    return a->type == MATCHACT;
}

static int
Action_readAhead(Action *a)
{
    return !Action_isMatch(a) ||
	(a->state && a->state->next && !Action_isRule(a->state->next->action));
}

#endif
