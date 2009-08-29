#include "Forth.h"
#include "SkTDArray.h"
#include "SkTDict.h"
#include "SkString.h"

ForthEngine::ForthEngine(ForthOutput* output) : fOutput(output) {
    size_t size = 32 * sizeof(intptr_t);
    fStackBase = reinterpret_cast<intptr_t*>(sk_malloc_throw(size));
    fStackStop = fStackBase + size/sizeof(intptr_t);
    fStackCurr = fStackStop;
}

ForthEngine::~ForthEngine() {
    sk_free(fStackBase);
}

void ForthEngine::sendOutput(const char text[]) {
    if (fOutput) {
        fOutput->show(text);
    } else {
        SkDebugf("%s", text);
    }
}

///////////////// ints

void ForthEngine::push(intptr_t value) {
    if (fStackCurr > fStackBase) {
        SkASSERT(fStackCurr <= fStackStop && fStackCurr > fStackBase);
        *--fStackCurr = value;
    } else {
        this->signal_error("overflow");
    }
}

intptr_t ForthEngine::peek(size_t index) const {
    SkASSERT(fStackCurr < fStackStop && fStackCurr >= fStackBase);
    if (fStackCurr + index < fStackStop) {
        return fStackCurr[index];
    } else {
        this->signal_error("peek out of range");
        return 0x80000001;
    }
}

void ForthEngine::setTop(intptr_t value) {
    if (fStackCurr < fStackStop) {
        SkASSERT(fStackCurr < fStackStop && fStackCurr >= fStackBase);
        *fStackCurr = value;
    } else {
        this->signal_error("underflow");
    }
}

intptr_t ForthEngine::pop() {
    if (fStackCurr < fStackStop) {
        SkASSERT(fStackCurr < fStackStop && fStackCurr >= fStackBase);
        return *fStackCurr++;
    } else {
        this->signal_error("underflow");
        return 0x80000001;
    }
}

///////////////////////////////////////////////////////////////////////////////

void ForthWord::call(ForthCallBlock* block) {
    ForthEngine engine(NULL);
    if (block) {
        // walk the array backwards, so that the top of the stack is data[0]
        for (size_t i = 0; i < block->in_count; i++) {
            engine.push(block->in_data[i]);
        }
    }
    this->exec(&engine);
    if (block) {
        size_t n = engine.depth();
        block->out_depth = n;
        if (n > block->out_count) {
            n = block->out_count;
        }
        for (size_t i = 0; i < n; i++) {
            block->out_data[i] = engine.peek(i);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

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

/*
    reading an initial 32bit value from the code stream:
 
    xxxxxxxx xxxxxxxx xxxxxxxx xxxxxx00
 
    Those last two bits are always 0 for a word, so we set those bits for other
    opcodes
 
    00 -- execute this word
    01 -- push (value & ~3) on the data stack
    10 -- push value >> 2 on the data stack (sign extended)
    11 -- switch (value >>> 2) for Code
 */

class FCode {
public:
    enum Bits {
        kWord_Bits          = 0,    // must be zero for function address
        kDataClear2_Bits    = 1,
        kDataShift2_Bits    = 2,
        kCodeShift2_Bits    = 3
    };
    enum Code {
        kPushInt_Code,
        kDone_Code
    };

    void appendInt(int32_t);
    void appendWord(ForthWord*);
    void appendIF() {}
    bool appendELSE() { return false; }
    bool appendTHEN() { return false; }
    void done();

    intptr_t* detach() {
        this->done();
        return fData.detach();
    }
    intptr_t* begin() {
        this->done();
        return fData.begin();
    }
    
    static void Exec(const intptr_t*, ForthEngine*);

private:
    SkTDArray<intptr_t> fData;
};

void FCode::appendInt(int32_t value) {
    if ((value & 3) == 0) {
        *fData.append() = value | kDataClear2_Bits;
    } else if ((value >> 2 << 2) == value) {
        *fData.append() = value | kDataShift2_Bits;
    } else {
        intptr_t* p = fData.append(2);
        *p++ = (kPushInt_Code << 2) | kCodeShift2_Bits;
        *p++ = value;
    }
}

void FCode::appendWord(ForthWord* word) {
    SkASSERT((reinterpret_cast<intptr_t>(word) & 3) == 0);
    *fData.append() = reinterpret_cast<intptr_t>(word);
}

void FCode::done() {
    *fData.append() = (kDone_Code << 2) | kCodeShift2_Bits;
}

void FCode::Exec(const intptr_t* curr, ForthEngine* engine) {
    for (;;) {
        intptr_t c = *curr++;
        switch (c & 3) {
            case kWord_Bits:
                reinterpret_cast<ForthWord*>(c)->exec(engine);
                break;
            case kDataClear2_Bits:
                engine->push(c & ~3);
                break;
            case kDataShift2_Bits:
                engine->push(c >> 2);
                break;
            case kCodeShift2_Bits:
                switch ((uint32_t)c >> 2) {
                    case kPushInt_Code:
                        engine->push(*curr++);
                        break;
                    case kDone_Code:
                        return;
                }
                break;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////

class CustomWord : public ForthWord {
public:
    // we assume ownership of code[]
    CustomWord(intptr_t code[]) : fCode(code) {}
    virtual ~CustomWord() { sk_free(fCode); }

    virtual void exec(ForthEngine* engine) {
        FCode::Exec(fCode, engine);
    }

private:
    intptr_t* fCode;
};

///////////////////////////////////////////////////////////////////////////////

class ForthParser {
public:
    ForthParser() : fDict(4096) {
        this->addStdWords();
    }

    const char* parse(const char text[], FCode*);

    void addWord(const char name[], ForthWord* word) {
        this->add(name, strlen(name), word);
    }

    ForthWord* find(const char name[], size_t len) const {
        ForthWord* word;
        return fDict.find(name, len, &word) ? word : NULL;
    }
    
private:
    void add(const char name[], size_t len, ForthWord* word) {
        (void)fDict.set(name, len, word);
    }

    void addStdWords() {
        this->add("clr", 3, new clearStack_ForthWord);
        this->add("drop", 4, new drop_ForthWord);
        this->add("dup", 3, new dup_ForthWord);
        this->add("swap", 4, new swap_ForthWord);
        
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
    }
    
    SkTDict<ForthWord*> fDict;
};

static const char* parse_error(const char msg[]) {
    SkDebugf("-- parser error: %s\n", msg);
    return NULL;
}

/** returns true if c is whitespace, including null
 */
static bool is_ws(int c) {
    return c <= ' ';
}

static const char* parse_token(const char** text, size_t* len) {
    const char* s = *text;
    while (is_ws(*s)) {
        if (0 == *s) {
            return NULL;
        }
        s++;
    }
    const char* token = s++;
    while (!is_ws(*s)) {
        s++;
    }
    *text = s;
    *len = s - token;
    return token;
}

static bool is_digit(int c) { return (unsigned)(c - '0') <= 9; }
static int hex_val(int c) {
    if (is_digit(c)) {
        return c - '0';
    } else {
        if (c <= 'Z') {
            return 10 + c - 'A';
        } else {
            return 10 + c - 'a';
        }
    }
}

static bool parse_num(const char str[], size_t len, int32_t* numBits) {
    if (1 == len && !is_digit(*str)) {
        return false;
    }
    const char* start = str;
    int32_t num = 0;
    bool neg = false;
    if (*str == '-') {
        neg = true;
        str += 1;
    } else if (*str == '#') {
        str++;
        while (str - start < len) {
            num = num*16 + hex_val(*str);
            str += 1;
        }
        *numBits = num;
        return true;
    }

    while (is_digit(*str)) {
        num = 10*num + *str - '0';
        str += 1;
    }
    SkASSERT(str - start <= len);
    if (str - start == len) {
        if (neg) {
            num = -num;
        }
        *numBits = num;
        return true;
    }
    // if we're not done with the token then the next char must be a decimal
    if (*str != '.') {
        return false;
    }
    str += 1;
    float x = num;
    float denom = 1;
    while (str - start < len && is_digit(*str)) {
        x = 10*x + *str - '0';
        denom *= 10;
        str += 1;
    }
    x /= denom;
    if (str - start == len) {
        if (neg) {
            x = -x;
        }
        *numBits = f2i_bits(x);
        return true;
    }
    return false;
}

static const char* parse_comment(const char text[]) {
    SkASSERT(*text == '(');
    while (')' != *++text) {
        if (0 == *text) {
            return NULL;
        }
    }
    return text + 1;    // skip past the closing ')'
}

const char* ForthParser::parse(const char text[], FCode* code) {
    for (;;) {
        size_t len;
        const char* token = parse_token(&text, &len);
        if (NULL == token) {
            break;
        }
        if (1 == len) {
            if ('(' == *token) {
                text = parse_comment(token);
                if (NULL == text) {
                    return NULL;
                }
                continue;
            }
            if (';' == *token) {
                break;
            }
            if (':' == *token) {
                token = parse_token(&text, &len);
                if (NULL == token) {
                    return parse_error("missing name after ':'");
                }
                FCode subCode;
                text = this->parse(text, &subCode);
                if (NULL == text) {
                    return NULL;
                }
                this->add(token, len, new CustomWord(subCode.detach()));
                continue;
            }
        }
        int32_t num;
        if (parse_num(token, len, &num)) {
            // note that num is just the bit representation of the float
            code->appendInt(num);
        } else if (2 == len && memcmp(token, "IF", 2) == 0) {
            code->appendIF();
        } else if (2 == len && memcmp(token, "ELSE", 4) == 0) {
            if (!code->appendELSE()) {
                return parse_error("ELSE with no matching IF");
            }
        } else if (2 == len && memcmp(token, "THEN", 4) == 0) {
            if (!code->appendTHEN()) {
                return parse_error("THEN with no matching IF");
            }
        } else{
            ForthWord* word = this->find(token, len);
            if (NULL == word) {
                SkString str(token, len);
                str.prepend("unknown word ");
                return parse_error(str.c_str());
            }
            code->appendWord(word);
        }
    }
    return text;
}

///////////////////////////////////////////////////////////////////////////////

class ForthEnv::Impl {
public:
    ForthParser fParser;
    FCode       fBuilder;
};

ForthEnv::ForthEnv() {
    fImpl = new Impl;
}

ForthEnv::~ForthEnv() {
    delete fImpl;
}

void ForthEnv::addWord(const char name[], ForthWord* word) {
    fImpl->fParser.addWord(name, word);
}

void ForthEnv::parse(const char text[]) {
    fImpl->fParser.parse(text, &fImpl->fBuilder);
}

ForthWord* ForthEnv::findWord(const char name[]) {
    return fImpl->fParser.find(name, strlen(name));
}

void ForthEnv::run(ForthOutput* output) {
    ForthEngine engine(output);
    FCode::Exec(fImpl->fBuilder.begin(), &engine);
}

#if 0
void ForthEnv::run(const char text[], ForthOutput* output) {
    FCode builder;

    if (fImpl->fParser.parse(text, &builder)) {
        ForthEngine engine(output);
        FCode::Exec(builder.begin(), &engine);
    }
}
#endif

