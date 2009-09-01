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

void Forth_test_stdwords();
void Forth_test_stdwords() {
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
            gRecs[i].fProc(word, &engine, &reporter);
        }
    }
    
    if (0 == reporter.fFailureCount) {
        SkDebugf("--- success!\n");
    } else {
        SkDebugf("--- %d failures\n", reporter.fFailureCount);
    }
}

