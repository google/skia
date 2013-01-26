
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef Forth_DEFINED
#define Forth_DEFINED

#include "SkTypes.h"

class ForthOutput {
public:
    virtual void show(const char output[]) = 0;
};

union FloatIntDual {
    int32_t fInt;
    float   fFloat;
};

static inline int32_t f2i_bits(float x) {
    FloatIntDual d;
    d.fFloat = x;
    return d.fInt;
}

static inline float i2f_bits(int32_t x) {
    FloatIntDual d;
    d.fInt = x;
    return d.fFloat;
}

class ForthEngine {
public:
    ForthEngine(ForthOutput*);
    ~ForthEngine();

    int         depth() const { return fStackStop - fStackCurr; }
    void        clearStack() { fStackCurr = fStackStop; }

    void        push(intptr_t value);
    intptr_t    top() const { return this->peek(0); }
    intptr_t    peek(size_t index) const;
    void        setTop(intptr_t value);
    intptr_t    pop();

    void        fpush(float value) { this->push(f2i_bits(value)); }
    float       fpeek(size_t i) const { return i2f_bits(this->fpeek(i)); }
    float       ftop() const { return i2f_bits(this->top()); }
    void        fsetTop(float value) { this->setTop(f2i_bits(value)); }
    float       fpop() { return i2f_bits(this->pop()); }

    void sendOutput(const char text[]);

private:
    ForthOutput* fOutput;
    intptr_t*   fStackBase;
    intptr_t*   fStackCurr;
    intptr_t*   fStackStop;

    void signal_error(const char msg[]) const {
        SkDebugf("ForthEngine error: %s\n", msg);
    }
};

struct ForthCallBlock {
    const intptr_t* in_data;
    size_t          in_count;
    intptr_t*       out_data;
    size_t          out_count;
    size_t          out_depth;
};

class ForthWord {
public:
    virtual ~ForthWord() {}
    virtual void exec(ForthEngine*) = 0;

    // todo: return error state of the engine
    void call(ForthCallBlock*);
};

class ForthEnv {
public:
    ForthEnv();
    ~ForthEnv();


    void addWord(const char name[], ForthWord*);

    void parse(const char code[]);

    ForthWord* findWord(const char name[]);

    void run(ForthOutput* = NULL);

private:
    class Impl;
    Impl* fImpl;
};

#endif
