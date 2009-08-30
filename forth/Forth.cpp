#include "Forth.h"
#include "ForthParser.h"
#include "SkTDArray.h"
#include "SkString.h"
#include "SkTDStack.h"

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

    // setup the initial stack with the callers input data
    if (block) {
        // walk the array backwards, so that the top of the stack is data[0]
        for (size_t i = 0; i < block->in_count; i++) {
            engine.push(block->in_data[i]);
        }
    }

    // now invoke the word
    this->exec(&engine);

    // now copy back the stack into the caller's output data
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
    enum {
        kCodeShift  = 2,
        kCodeMask   = 7,
        kCodeDataShift  = 5
    };
    static unsigned GetCode(intptr_t c) {
        return ((uint32_t)c >> kCodeShift) & kCodeMask;
    }
    static unsigned GetCodeData(intptr_t c) {
        return (uint32_t)c >> kCodeDataShift;
    }

    enum Bits {
        kWord_Bits          = 0,    // must be zero for function address
        kDataClear2_Bits    = 1,
        kDataShift2_Bits    = 2,
        kCodeShift2_Bits    = 3
    };

    enum Code {
        kPushInt_Code,  // for data that needs more than 30 bits
        kIF_Code,
        kELSE_Code,
        kDone_Code
    };
    static unsigned MakeCode(Code code) {
        return (code << kCodeShift) | kCodeShift2_Bits;
    }
    
    void appendInt(int32_t);
    void appendWord(ForthWord*);
    void appendIF();
    bool appendELSE();
    bool appendTHEN();
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
    SkTDStack<size_t>   fIfStack;
};

void FCode::appendInt(int32_t value) {
    if ((value & 3) == 0) {
        *fData.append() = value | kDataClear2_Bits;
    } else if ((value << 2 >> 2) == value) {
        *fData.append() = (value << 2) | kDataShift2_Bits;
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

void FCode::appendIF() {
    size_t ifIndex = fData.count();
    fIfStack.push(ifIndex);
    *fData.append() = MakeCode(kIF_Code);
}

bool FCode::appendELSE() {
    if (fIfStack.empty()) {
        return false;
    }

    size_t elseIndex = fData.count();
    *fData.append() = MakeCode(kELSE_Code);

    size_t ifIndex = fIfStack.top();
    // record the offset in the data part of the if-code
    fData[ifIndex] |= (elseIndex - ifIndex) << kCodeDataShift;

    // now reuse this IfStack entry to track the else
    fIfStack.top() = elseIndex;
    return true;
}

bool FCode::appendTHEN() {
    if (fIfStack.empty()) {
        return false;
    }

    // this is either an IF or an ELSE
    size_t index = fIfStack.top();
    // record the offset in the data part of the code
    fData[index] |= (fData.count() - index - 1) << kCodeDataShift;

    fIfStack.pop();
    return true;
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
                switch (GetCode(c)) {
                    case kPushInt_Code:
                        engine->push(*curr++);
                        break;
                    case kIF_Code:
                        if (!engine->pop()) {
                            // takes us past the ELSE or THEN
                            curr += GetCodeData(c);
                        }
                        break;
                    case kELSE_Code:
                        // takes us past the THEN
                        curr += GetCodeData(c);
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

ForthParser::ForthParser() : fDict(4096) {
    this->addStdWords();
}

ForthParser::~ForthParser() {
}

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
        } else if (4 == len && memcmp(token, "ELSE", 4) == 0) {
            if (!code->appendELSE()) {
                return parse_error("ELSE with no matching IF");
            }
        } else if (4 == len && memcmp(token, "THEN", 4) == 0) {
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

