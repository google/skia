#include "Test.h"
#include "SkRandom.h"
#include "SkRefCnt.h"
#include "SkTSearch.h"
#include "SkTSort.h"
#include "SkUtils.h"

class RefClass : public SkRefCnt {
public:
    RefClass(int n) : fN(n) {}
    int get() const { return fN; }

private:
    int fN;
};

static void test_refptr(skiatest::Reporter* reporter) {
    RefClass* r0 = new RefClass(0);

    SkRefPtr<RefClass> rc0;
    REPORTER_ASSERT(reporter, rc0.get() == NULL);
    REPORTER_ASSERT(reporter, !rc0);

    SkRefPtr<RefClass> rc1;
    REPORTER_ASSERT(reporter, rc0 == rc1);
    REPORTER_ASSERT(reporter, rc0.get() != r0);

    rc0 = r0;
    REPORTER_ASSERT(reporter, rc0);
    REPORTER_ASSERT(reporter, rc0 != rc1);
    REPORTER_ASSERT(reporter, rc0.get() == r0);

    rc1 = rc0;
    REPORTER_ASSERT(reporter, rc1);
    REPORTER_ASSERT(reporter, rc0 == rc1);
    REPORTER_ASSERT(reporter, rc0.get() == r0);

    rc0 = NULL;
    REPORTER_ASSERT(reporter, rc0.get() == NULL);
    REPORTER_ASSERT(reporter, !rc0);
    REPORTER_ASSERT(reporter, rc0 != rc1);

    r0->unref();
}

static void test_autounref(skiatest::Reporter* reporter) {
    RefClass obj(0);
    REPORTER_ASSERT(reporter, 1 == obj.getRefCnt());

    SkAutoTUnref<RefClass> tmp(&obj);
    REPORTER_ASSERT(reporter, &obj == tmp.get());
    REPORTER_ASSERT(reporter, 1 == obj.getRefCnt());

    REPORTER_ASSERT(reporter, &obj == tmp.detach());
    REPORTER_ASSERT(reporter, 1 == obj.getRefCnt());
    REPORTER_ASSERT(reporter, NULL == tmp.detach());
    REPORTER_ASSERT(reporter, NULL == tmp.get());

    obj.ref();
    REPORTER_ASSERT(reporter, 2 == obj.getRefCnt());
    {
        SkAutoTUnref<RefClass> tmp2(&obj);
    }
    REPORTER_ASSERT(reporter, 1 == obj.getRefCnt());
}

/////////////////////////////////////////////////////////////////////////////

#define kSEARCH_COUNT   91

static void test_search(skiatest::Reporter* reporter) {
    int         i, array[kSEARCH_COUNT];
    SkRandom    rand;

    for (i = 0; i < kSEARCH_COUNT; i++) {
        array[i] = rand.nextS();
    }

    SkTHeapSort<int>(array, kSEARCH_COUNT);
    // make sure we got sorted properly
    for (i = 1; i < kSEARCH_COUNT; i++) {
        REPORTER_ASSERT(reporter, array[i-1] <= array[i]);
    }

    // make sure we can find all of our values
    for (i = 0; i < kSEARCH_COUNT; i++) {
        int index = SkTSearch<int>(array, kSEARCH_COUNT, array[i], sizeof(int));
        REPORTER_ASSERT(reporter, index == i);
    }

    // make sure that random values are either found, or the correct
    // insertion index is returned
    for (i = 0; i < 10000; i++) {
        int value = rand.nextS();
        int index = SkTSearch<int>(array, kSEARCH_COUNT, value, sizeof(int));

        if (index >= 0) {
            REPORTER_ASSERT(reporter,
                            index < kSEARCH_COUNT && array[index] == value);
        } else {
            index = ~index;
            REPORTER_ASSERT(reporter, index <= kSEARCH_COUNT);
            if (index < kSEARCH_COUNT) {
                REPORTER_ASSERT(reporter, value < array[index]);
                if (index > 0) {
                    REPORTER_ASSERT(reporter, value > array[index - 1]);
                }
            } else {
                // we should append the new value
                REPORTER_ASSERT(reporter, value > array[kSEARCH_COUNT - 1]);
            }
        }
    }
}

static void test_utf16(skiatest::Reporter* reporter) {
    static const SkUnichar gUni[] = {
        0x10000, 0x18080, 0x20202, 0xFFFFF, 0x101234
    };

    uint16_t buf[2];

    for (size_t i = 0; i < SK_ARRAY_COUNT(gUni); i++) {
        size_t count = SkUTF16_FromUnichar(gUni[i], buf);
        REPORTER_ASSERT(reporter, count == 2);
        size_t count2 = SkUTF16_CountUnichars(buf, 2);
        REPORTER_ASSERT(reporter, count2 == 1);
        const uint16_t* ptr = buf;
        SkUnichar c = SkUTF16_NextUnichar(&ptr);
        REPORTER_ASSERT(reporter, c == gUni[i]);
        REPORTER_ASSERT(reporter, ptr - buf == 2);
    }
}

static void TestUTF(skiatest::Reporter* reporter) {
    static const struct {
        const char* fUtf8;
        SkUnichar   fUni;
    } gTest[] = {
        { "a",                  'a' },
        { "\x7f",               0x7f },
        { "\xC2\x80",           0x80 },
        { "\xC3\x83",           (3 << 6) | 3    },
        { "\xDF\xBF",           0x7ff },
        { "\xE0\xA0\x80",       0x800 },
        { "\xE0\xB0\xB8",       0xC38 },
        { "\xE3\x83\x83",       (3 << 12) | (3 << 6) | 3    },
        { "\xEF\xBF\xBF",       0xFFFF },
        { "\xF0\x90\x80\x80",   0x10000 },
        { "\xF3\x83\x83\x83",   (3 << 18) | (3 << 12) | (3 << 6) | 3    }
    };

    for (size_t i = 0; i < SK_ARRAY_COUNT(gTest); i++) {
        const char* p = gTest[i].fUtf8;
        int         n = SkUTF8_CountUnichars(p);
        SkUnichar   u0 = SkUTF8_ToUnichar(gTest[i].fUtf8);
        SkUnichar   u1 = SkUTF8_NextUnichar(&p);

        REPORTER_ASSERT(reporter, n == 1);
        REPORTER_ASSERT(reporter, u0 == u1);
        REPORTER_ASSERT(reporter, u0 == gTest[i].fUni);
        REPORTER_ASSERT(reporter,
                        p - gTest[i].fUtf8 == (int)strlen(gTest[i].fUtf8));
    }

    test_utf16(reporter);
    test_search(reporter);
    test_refptr(reporter);
    test_autounref(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Utils", UtfTestClass, TestUTF)
