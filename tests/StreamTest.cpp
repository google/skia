#include "Test.h"
#include "SkRandom.h"
#include "SkStream.h"

#define MAX_SIZE    (256 * 1024)

static void random_fill(SkRandom& rand, void* buffer, size_t size) {
    char* p = (char*)buffer;
    char* stop = p + size;
    while (p < stop) {
        *p++ = (char)(rand.nextU() >> 8);
    }
}

static void test_buffer(skiatest::Reporter* reporter) {
    SkRandom rand;
    SkAutoMalloc am(MAX_SIZE * 2);
    char* storage = (char*)am.get();
    char* storage2 = storage + MAX_SIZE;

    random_fill(rand, storage, MAX_SIZE);

    for (int sizeTimes = 0; sizeTimes < 100; sizeTimes++) {
        int size = rand.nextU() % MAX_SIZE;
        if (size == 0) {
            size = MAX_SIZE;
        }
        for (int times = 0; times < 100; times++) {
            int bufferSize = 1 + (rand.nextU() & 0xFFFF);
            SkMemoryStream mstream(storage, size);
            SkBufferStream bstream(&mstream, bufferSize);

            int bytesRead = 0;
            while (bytesRead < size) {
                int s = 17 + (rand.nextU() & 0xFFFF);
                int ss = bstream.read(storage2, s);
                REPORTER_ASSERT(reporter, ss > 0 && ss <= s);
                REPORTER_ASSERT(reporter, bytesRead + ss <= size);
                REPORTER_ASSERT(reporter,
                                memcmp(storage + bytesRead, storage2, ss) == 0);
                bytesRead += ss;
            }
            REPORTER_ASSERT(reporter, bytesRead == size);
        }
    }
}

static void TestRStream(skiatest::Reporter* reporter) {
    static const char s[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    char            copy[sizeof(s)];
    SkRandom        rand;

    for (int i = 0; i < 65; i++) {
        char*           copyPtr = copy;
        SkMemoryStream  mem(s, sizeof(s));
        SkBufferStream  buff(&mem, i);

        do {
            copyPtr += buff.read(copyPtr, rand.nextU() & 15);
        } while (copyPtr < copy + sizeof(s));
        REPORTER_ASSERT(reporter, copyPtr == copy + sizeof(s));
        REPORTER_ASSERT(reporter, memcmp(s, copy, sizeof(s)) == 0);
    }
    test_buffer(reporter);
}

static void TestWStream(skiatest::Reporter* reporter) {
    SkDynamicMemoryWStream  ds;
    const char s[] = "abcdefghijklmnopqrstuvwxyz";
    int i;
    for (i = 0; i < 100; i++) {
        REPORTER_ASSERT(reporter, ds.write(s, 26));
    }
    REPORTER_ASSERT(reporter, ds.getOffset() == 100 * 26);
    char* dst = new char[100 * 26 + 1];
    dst[100*26] = '*';
    ds.copyTo(dst);
    REPORTER_ASSERT(reporter, dst[100*26] == '*');
//     char* p = dst;
    for (i = 0; i < 100; i++) {
        REPORTER_ASSERT(reporter, memcmp(&dst[i * 26], s, 26) == 0);
    }
    REPORTER_ASSERT(reporter, memcmp(dst, ds.getStream(), 100*26) == 0);
    delete[] dst;
}

static void TestStreams(skiatest::Reporter* reporter) {
    TestRStream(reporter);
    TestWStream(reporter);
}

#include "TestClassDef.h"
DEFINE_TESTCLASS("Stream", StreamTestClass, TestStreams)
