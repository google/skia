#include "Forth.h"
#include "ForthParser.h"
#include "SkString.h"

#define BEGIN_WORD(name)   \
    class name##_ForthWord : public ForthWord { \
    public:                                     \
        virtual void exec(ForthEngine* fe)

#define END_WORD };

///////////////////////////////////////////////////////////////////////////////

BEGIN_WORD(drop) {
    (void)fe->pop();
} END_WORD

BEGIN_WORD(over) {
    fe->push(fe->peek(1));
} END_WORD

BEGIN_WORD(dup) {
    fe->push(fe->top());
} END_WORD

BEGIN_WORD(swap) {
    intptr_t a = fe->pop();
    intptr_t b = fe->top();
    fe->setTop(a);
    fe->push(b);
} END_WORD

BEGIN_WORD(rot) {
    intptr_t c = fe->pop();
    intptr_t b = fe->pop();
    intptr_t a = fe->pop();
    fe->push(b);
    fe->push(c);
    fe->push(a);
} END_WORD

BEGIN_WORD(rrot) {
    intptr_t c = fe->pop();
    intptr_t b = fe->pop();
    intptr_t a = fe->pop();
    fe->push(c);
    fe->push(a);
    fe->push(b);
} END_WORD

BEGIN_WORD(swap2) {
    intptr_t d = fe->pop();
    intptr_t c = fe->pop();
    intptr_t b = fe->pop();
    intptr_t a = fe->pop();
    fe->push(c);
    fe->push(d);
    fe->push(a);
    fe->push(b);
} END_WORD

BEGIN_WORD(dup2) {
    fe->push(fe->peek(1));
    fe->push(fe->peek(1));
} END_WORD

BEGIN_WORD(over2) {
    fe->push(fe->peek(3));
    fe->push(fe->peek(3));
} END_WORD

BEGIN_WORD(drop2) {
    (void)fe->pop();
    (void)fe->pop();
} END_WORD

///////////////// logicals

BEGIN_WORD(logical_and) {
    intptr_t tmp = fe->pop();
    fe->setTop(-(tmp && fe->top()));
} END_WORD

BEGIN_WORD(logical_or) {
    intptr_t tmp = fe->pop();
    fe->setTop(-(tmp || fe->top()));
} END_WORD

BEGIN_WORD(logical_not) {
    fe->setTop(-(!fe->top()));
} END_WORD

BEGIN_WORD(if_dup) {
    intptr_t tmp = fe->top();
    if (tmp) {
        fe->push(tmp);
    }
} END_WORD

///////////////// ints

class add_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(fe->top() + tmp);
    }};

class sub_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(fe->top() - tmp);
    }};

class mul_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(fe->top() * tmp);
    }};

class div_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(fe->top() / tmp);
    }};

class mod_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(fe->top() % tmp);
    }};

class divmod_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t denom = fe->pop();
        intptr_t numer = fe->pop();
        fe->push(numer % denom);
        fe->push(numer / denom);
    }};

class dot_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        SkString str;
        str.printf("%d ", fe->pop());
        fe->sendOutput(str.c_str());
    }};

class abs_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        int32_t value = fe->top();
        if (value < 0) {
            fe->setTop(-value);
        }
    }};

class negate_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        fe->setTop(-fe->top());
    }};

class min_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        int32_t value = fe->pop();
        if (value < fe->top()) {
            fe->setTop(value);
        }
    }};

class max_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        int32_t value = fe->pop();
        if (value > fe->top()) {
            fe->setTop(value);
        }
    }
};

///////////////// floats

class fadd_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        float tmp = fe->fpop();
        fe->fsetTop(fe->ftop() + tmp);
    }
};

class fsub_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        float tmp = fe->fpop();
        fe->fsetTop(fe->ftop() - tmp);
    }
};

class fmul_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        float tmp = fe->fpop();
        fe->fsetTop(fe->ftop() * tmp);
    }
};

class fdiv_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        float tmp = fe->fpop();
        fe->fsetTop(fe->ftop() / tmp);
    }
};

class fdot_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        SkString str;
        str.printf("%g ", fe->fpop());
        fe->sendOutput(str.c_str());
    }
};

class fabs_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        float value = fe->ftop();
        if (value < 0) {
            fe->fsetTop(-value);
        }
    }
};

class fmin_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        float value = fe->fpop();
        if (value < fe->ftop()) {
            fe->fsetTop(value);
        }
    }
};

class fmax_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        float value = fe->fpop();
        if (value > fe->ftop()) {
            fe->fsetTop(value);
        }
    }
};

class floor_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        fe->fsetTop(floorf(fe->ftop()));
    }
};

class ceil_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        fe->fsetTop(ceilf(fe->ftop()));
    }
};

class round_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        fe->fsetTop(floorf(fe->ftop() + 0.5f));
    }
};

class f2i_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        fe->setTop((int)fe->ftop());
    }
};

class i2f_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        fe->fsetTop((float)fe->top());
    }
};

////////////////////////////// int compares

class eq_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        fe->push(-(fe->pop() == fe->pop()));
    }
};

class neq_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        fe->push(-(fe->pop() != fe->pop()));
    }
};

class lt_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(-(fe->top() < tmp));
    }
};

class le_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(-(fe->top() <= tmp));
    }
};

class gt_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(-(fe->top() > tmp));
    }
};

class ge_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(-(fe->top() >= tmp));
    }
};

BEGIN_WORD(lt0) {
    fe->setTop(fe->top() >> 31);
} END_WORD

BEGIN_WORD(ge0) {
    fe->setTop(~(fe->top() >> 31));
} END_WORD

BEGIN_WORD(gt0) {
    fe->setTop(-(fe->top() > 0));
} END_WORD

BEGIN_WORD(le0) {
    fe->setTop(-(fe->top() <= 0));
} END_WORD

/////////////////////////////// float compares

/*  negative zero is our nemesis, otherwise we could use = and <> from ints */

class feq_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        fe->push(-(fe->fpop() == fe->fpop()));
    }
};

class fneq_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        fe->push(-(fe->fpop() != fe->fpop()));
    }
};

class flt_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        float tmp = fe->fpop();
        fe->setTop(-(fe->ftop() < tmp));
    }
};

class fle_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        float tmp = fe->fpop();
        fe->setTop(-(fe->ftop() <= tmp));
    }
};

class fgt_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        float tmp = fe->fpop();
        fe->setTop(-(fe->ftop() > tmp));
    }
};

class fge_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        float tmp = fe->fpop();
        fe->setTop(-(fe->ftop() >= tmp));
    }
};

///////////////////////////////////////////////////////////////////////////////

#define ADD_LITERAL_WORD(sym, name) \
    this->add(sym, sizeof(sym)-1, new name##_ForthWord)

void ForthParser::addStdWords() {
    ADD_LITERAL_WORD("DROP", drop);
    ADD_LITERAL_WORD("DUP", dup);
    ADD_LITERAL_WORD("SWAP", swap);
    ADD_LITERAL_WORD("OVER", over);
    ADD_LITERAL_WORD("ROT", rot);
    ADD_LITERAL_WORD("-ROT", rrot);
    ADD_LITERAL_WORD("2SWAP", swap2);
    ADD_LITERAL_WORD("2DUP", dup2);
    ADD_LITERAL_WORD("2OVER", over2);
    ADD_LITERAL_WORD("2DROP", drop2);
    
    ADD_LITERAL_WORD("+", add);
    ADD_LITERAL_WORD("-", sub);
    ADD_LITERAL_WORD("*", mul);
    ADD_LITERAL_WORD("/", div);
    ADD_LITERAL_WORD("MOD", mod);
    ADD_LITERAL_WORD("/MOD", divmod);

    ADD_LITERAL_WORD(".", dot);
    ADD_LITERAL_WORD("ABS", abs);
    ADD_LITERAL_WORD("NEGATE", negate);
    ADD_LITERAL_WORD("MIN", min);
    ADD_LITERAL_WORD("MAX", max);

    ADD_LITERAL_WORD("AND", logical_and);
    ADD_LITERAL_WORD("OR", logical_or);
    ADD_LITERAL_WORD("0=", logical_not);
    ADD_LITERAL_WORD("?DUP", if_dup);

    this->add("f+", 2, new fadd_ForthWord);
    this->add("f-", 2, new fsub_ForthWord);
    this->add("f*", 2, new fmul_ForthWord);
    this->add("f/", 2, new fdiv_ForthWord);
    this->add("f.", 2, new fdot_ForthWord);
    this->add("fabs", 4, new fabs_ForthWord);
    this->add("fmin", 4, new fmin_ForthWord);
    this->add("fmax", 4, new fmax_ForthWord);
    this->add("fmax", 4, new fmax_ForthWord);
    this->add("floor", 5, new floor_ForthWord);
    this->add("ceil", 4, new ceil_ForthWord);
    this->add("round", 5, new round_ForthWord);
    this->add("f>i", 3, new f2i_ForthWord);
    this->add("i>f", 3, new i2f_ForthWord);

    this->add("=", 1, new eq_ForthWord);
    this->add("<>", 2, new neq_ForthWord);
    this->add("<", 1, new lt_ForthWord);
    this->add("<=", 2, new le_ForthWord);
    this->add(">", 1, new gt_ForthWord);
    this->add(">=", 2, new ge_ForthWord);
    ADD_LITERAL_WORD("0<", lt0);
    ADD_LITERAL_WORD("0>", gt0);
    ADD_LITERAL_WORD("0<=", le0);
    ADD_LITERAL_WORD("0>=", ge0);
    
    this->add("f=", 2, new feq_ForthWord);
    this->add("f<>", 3, new fneq_ForthWord);
    this->add("f<", 2, new flt_ForthWord);
    this->add("f<=", 3, new fle_ForthWord);
    this->add("f>", 2, new fgt_ForthWord);
    this->add("f>=", 3, new fge_ForthWord);
}

