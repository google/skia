#include "Forth.h"
#include "ForthParser.h"
#include "SkString.h"

class drop_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        (void)fe->pop();
    }
};

class clearStack_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        fe->clearStack();
    }
};

class dup_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        fe->push(fe->top());
    }
};

class swap_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        int32_t a = fe->pop();
        int32_t b = fe->top();
        fe->setTop(a);
        fe->push(b);
    }
};

class rot_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        fe->push(fe->peek(1));
    }
};

///////////////// ints

class add_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(fe->top() + tmp);
    }
};

class sub_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(fe->top() - tmp);
    }
};

class mul_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(fe->top() * tmp);
    }
};

class div_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(fe->top() / tmp);
    }
};

class dot_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        SkString str;
        str.printf("%d ", fe->pop());
        fe->sendOutput(str.c_str());
    }
};

class abs_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        int32_t value = fe->top();
        if (value < 0) {
            fe->setTop(-value);
        }
    }
};

class min_ForthWord : public ForthWord {
public:
    virtual void exec(ForthEngine* fe) {
        int32_t value = fe->pop();
        if (value < fe->top()) {
            fe->setTop(value);
        }
    }
};

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

///////////////////////////////////////////////////////////////////////////////

class eq_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        fe->push(fe->pop() == fe->pop());
    }
};

class neq_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        fe->push(fe->pop() != fe->pop());
    }
};

class lt_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(fe->top() < tmp);
    }
};

class le_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(fe->top() <= tmp);
    }
};

class gt_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(fe->top() > tmp);
    }
};

class ge_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        intptr_t tmp = fe->pop();
        fe->setTop(fe->top() >= tmp);
    }
};

class feq_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        fe->push(fe->fpop() == fe->fpop());
    }
};

class fneq_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        fe->push(fe->fpop() != fe->fpop());
    }
};

class flt_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        float tmp = fe->fpop();
        fe->setTop(fe->ftop() < tmp);
    }
};

class fle_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        float tmp = fe->fpop();
        fe->setTop(fe->ftop() <= tmp);
    }
};

class fgt_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        float tmp = fe->fpop();
        fe->setTop(fe->ftop() > tmp);
    }
};

class fge_ForthWord : public ForthWord { public:
    virtual void exec(ForthEngine* fe) {
        float tmp = fe->fpop();
        fe->setTop(fe->ftop() >= tmp);
    }
};

///////////////////////////////////////////////////////////////////////////////

void ForthParser::addStdWords() {
    this->add("clr", 3, new clearStack_ForthWord);
    this->add("drop", 4, new drop_ForthWord);
    this->add("dup", 3, new dup_ForthWord);
    this->add("swap", 4, new swap_ForthWord);
    this->add("rot", 3, new rot_ForthWord);
    
    this->add("+", 1, new add_ForthWord);
    this->add("-", 1, new sub_ForthWord);
    this->add("*", 1, new mul_ForthWord);
    this->add("/", 1, new div_ForthWord);
    this->add(".", 1, new dot_ForthWord);
    this->add("abs", 3, new abs_ForthWord);
    this->add("min", 3, new min_ForthWord);
    this->add("max", 3, new max_ForthWord);
    
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

    this->add("f=", 2, new feq_ForthWord);
    this->add("f<>", 3, new fneq_ForthWord);
    this->add("f<", 2, new flt_ForthWord);
    this->add("f<=", 3, new fle_ForthWord);
    this->add("f>", 2, new fgt_ForthWord);
    this->add("f>=", 3, new fge_ForthWord);
}

