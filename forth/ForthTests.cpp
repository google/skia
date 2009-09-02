#include "Forth.h"
#include "SkString.h"

class Reporter {
public:
    int fFailureCount;

    Reporter() : fFailureCount(0) {}
    void reportFailure(const char expression[], const char file[], int line);
    void reportFailure(const char msg[]);
};
    
typedef void (*ForthWordTestProc)(ForthWord*, ForthEngine*, Reporter*);

#define FORTH_ASSERT(reporter, expression)      \
    do {                                        \
        if (!(expression)) {                    \
            reporter->reportFailure(#expression, __FILE__, __LINE__);   \
        }                                       \
    } while (0)

static void drop_test0(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(-17);
    word->exec(fe);
    FORTH_ASSERT(reporter, 0 == fe->depth());
}

static void drop_test1(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(-17);
    fe->push(93);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, -17 == fe->peek(0));
}

static void dup_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(-17);
    word->exec(fe);
    FORTH_ASSERT(reporter, 2 == fe->depth());
    FORTH_ASSERT(reporter, -17 == fe->peek(0));
    FORTH_ASSERT(reporter, -17 == fe->peek(1));
}

static void swap_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(-17);
    fe->push(42);
    word->exec(fe);
    FORTH_ASSERT(reporter, 2 == fe->depth());
    FORTH_ASSERT(reporter, -17 == fe->peek(0));
    FORTH_ASSERT(reporter, 42 == fe->peek(1));
}

static void over_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(1);
    fe->push(2);
    word->exec(fe);
    FORTH_ASSERT(reporter, 3 == fe->depth());
    FORTH_ASSERT(reporter, 1 == fe->peek(0));
    FORTH_ASSERT(reporter, 2 == fe->peek(1));
    FORTH_ASSERT(reporter, 1 == fe->peek(2));
}

static void rot_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(1);
    fe->push(2);
    fe->push(3);
    word->exec(fe);
    FORTH_ASSERT(reporter, 3 == fe->depth());
    FORTH_ASSERT(reporter, 2 == fe->peek(2));
    FORTH_ASSERT(reporter, 3 == fe->peek(1));
    FORTH_ASSERT(reporter, 1 == fe->peek(0));
}

static void rrot_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(1);
    fe->push(2);
    fe->push(3);
    word->exec(fe);
    FORTH_ASSERT(reporter, 3 == fe->depth());
    FORTH_ASSERT(reporter, 2 == fe->peek(0));
    FORTH_ASSERT(reporter, 1 == fe->peek(1));
    FORTH_ASSERT(reporter, 3 == fe->peek(2));
}

static void swap2_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(1);
    fe->push(2);
    fe->push(3);
    fe->push(4);
    word->exec(fe);
    FORTH_ASSERT(reporter, 4 == fe->depth());
    FORTH_ASSERT(reporter, 2 == fe->peek(0));
    FORTH_ASSERT(reporter, 1 == fe->peek(1));
    FORTH_ASSERT(reporter, 4 == fe->peek(2));
    FORTH_ASSERT(reporter, 3 == fe->peek(3));
}

static void dup2_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(1);
    fe->push(2);
    word->exec(fe);
    FORTH_ASSERT(reporter, 4 == fe->depth());
    FORTH_ASSERT(reporter, 1 == fe->peek(3));
    FORTH_ASSERT(reporter, 2 == fe->peek(2));
    FORTH_ASSERT(reporter, 1 == fe->peek(1));
    FORTH_ASSERT(reporter, 2 == fe->peek(0));
}

static void over2_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(1);
    fe->push(2);
    fe->push(3);
    fe->push(4);
    word->exec(fe);
    FORTH_ASSERT(reporter, 6 == fe->depth());
    FORTH_ASSERT(reporter, 1 == fe->peek(5));
    FORTH_ASSERT(reporter, 2 == fe->peek(4));
    FORTH_ASSERT(reporter, 3 == fe->peek(3));
    FORTH_ASSERT(reporter, 4 == fe->peek(2));
    FORTH_ASSERT(reporter, 1 == fe->peek(1));
    FORTH_ASSERT(reporter, 2 == fe->peek(0));
}

static void drop2_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(1);
    fe->push(2);
    fe->push(3);
    fe->push(4);
    word->exec(fe);
    FORTH_ASSERT(reporter, 2 == fe->depth());
    FORTH_ASSERT(reporter, 1 == fe->peek(1));
    FORTH_ASSERT(reporter, 2 == fe->peek(0));
}

//////////////////////////////////////////////////////////////////////////////

static void iadd_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(35);
    fe->push(99);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 134 == fe->top());
    fe->push(-135);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, -1 == fe->top());
}

static void isub_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(35);
    fe->push(99);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 35-99 == fe->top());
}

static void imul_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(15);
    fe->push(-20);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, -300 == fe->top());
    fe->push(0);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 0 == fe->top());
}

static void idiv_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(100);
    fe->push(25);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 4 == fe->top());
    fe->setTop(10);
    fe->push(-3);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, -3 == fe->top());
    fe->setTop(-1);
    fe->push(-1);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 1 == fe->top());
}

static void imod_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(10);
    fe->push(3);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 1 == fe->top());
    fe->push(5);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 1 == fe->top());
}

static void idivmod_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(10);
    fe->push(3);
    word->exec(fe);
    FORTH_ASSERT(reporter, 2 == fe->depth());
    FORTH_ASSERT(reporter, 1 == fe->peek(1));
    FORTH_ASSERT(reporter, 3 == fe->peek(0));
}

static void idot_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(1);
    fe->push(2);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 1 == fe->top());
}

static void iabs_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(10);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 10 == fe->top());
    fe->setTop(-10);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 10 == fe->top());
}

static void inegate_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(10);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, -10 == fe->top());
    fe->setTop(-10);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 10 == fe->top());
}

static void imin_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(10);
    fe->push(3);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 3 == fe->top());
    fe->push(-10);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, -10 == fe->top());
}

static void imax_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(10);
    fe->push(3);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 10 == fe->top());
    fe->push(-10);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 10 == fe->top());
}

///////////////////////////////////////////////////////////////////////////////

static void logical_and_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    const static int data[] = {
        0, 0, 0,
        2, 0, 0,
        0, -1, 0,
        1, 5, -1
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(data)/3; i++) {
        fe->push(data[i*3 + 0]);
        fe->push(data[i*3 + 1]);
        word->exec(fe);
        FORTH_ASSERT(reporter, 1 == fe->depth());
        FORTH_ASSERT(reporter, data[i*3 + 2] == fe->top());
        fe->pop();
    }
}

static void logical_or_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    const static int data[] = {
        0, 0, 0,
        2, 0, -1,
        0, -1, -1,
        1, 5, -1
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(data)/3; i++) {
        fe->push(data[i*3 + 0]);
        fe->push(data[i*3 + 1]);
        word->exec(fe);
        FORTH_ASSERT(reporter, 1 == fe->depth());
        FORTH_ASSERT(reporter, data[i*3 + 2] == fe->top());
        fe->pop();
    }
}

static void logical_not_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    const static int data[] = {
        0, -1,
        5, 0,
        -1, 0
    };
    for (size_t i = 0; i < SK_ARRAY_COUNT(data)/2; i++) {
        fe->push(data[i*2 + 0]);
        word->exec(fe);
        FORTH_ASSERT(reporter, 1 == fe->depth());
        FORTH_ASSERT(reporter, data[i*2 + 1] == fe->top());
        fe->pop();
    }
}

static void if_dup_test(ForthWord* word, ForthEngine* fe, Reporter* reporter) {
    fe->push(10);
    word->exec(fe);
    FORTH_ASSERT(reporter, 2 == fe->depth());
    FORTH_ASSERT(reporter, 10 == fe->peek(1));
    FORTH_ASSERT(reporter, 10 == fe->peek(0));
    fe->pop();
    fe->pop();
    fe->push(0);
    word->exec(fe);
    FORTH_ASSERT(reporter, 1 == fe->depth());
    FORTH_ASSERT(reporter, 0 == fe->top());
}

static const struct {
    const char*         fName;
    ForthWordTestProc   fProc;
} gRecs[] = {
    { "DROP",   drop_test0 },   { "DROP",   drop_test1 },
    { "DUP",    dup_test },
    { "SWAP",   swap_test },
    { "OVER",   over_test },
    { "ROT",    rot_test },
    { "-ROT",   rrot_test },
    { "2SWAP",  swap2_test },
    { "2DUP",   dup2_test },
    { "2OVER",  over2_test },
    { "2DROP",  drop2_test },

    { "+",      iadd_test },
    { "-",      isub_test },
    { "*",      imul_test },
    { "/",      idiv_test },
    { "MOD",    imod_test },
    { "/MOD",   idivmod_test },

//    { ".",      idot_test },
    { "ABS",    iabs_test },
    { "NEGATE", inegate_test },
    { "MIN",    imin_test },
    { "MAX",    imax_test },

    { "AND",    logical_and_test },
    { "OR",     logical_or_test },
    { "0=",     logical_not_test },
    { "?DUP",   if_dup_test },
};

///////////////////////////////////////////////////////////////////////////////

void Reporter::reportFailure(const char expression[], const char file[],
                             int line) {
    SkDebugf("failed %s:%d: %s\n", file, line, expression);
    fFailureCount += 1;
}

void Reporter::reportFailure(const char msg[]) {
    SkDebugf("%s\n");
    fFailureCount += 1;
}

void Forth_test_stdwords(bool verbose);
void Forth_test_stdwords(bool verbose) {
    ForthEnv env;
    Reporter reporter;

    for (size_t i = 0; i < SK_ARRAY_COUNT(gRecs); i++) {
        ForthEngine engine(NULL);
        
        ForthWord* word = env.findWord(gRecs[i].fName);
        if (NULL == word) {
            SkString str;
            str.printf("--- can't find stdword %d", gRecs[i].fName);
            reporter.reportFailure(str.c_str());
        } else {
            if (verbose) {
                SkDebugf("--- testing %s %p\n", gRecs[i].fName, word);
            }
            gRecs[i].fProc(word, &engine, &reporter);
        }
    }
    
    if (0 == reporter.fFailureCount) {
        SkDebugf("--- success!\n");
    } else {
        SkDebugf("--- %d failures\n", reporter.fFailureCount);
    }
}

