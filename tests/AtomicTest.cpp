/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "Test.h"
#include "TestClassDef.h"
#include "SkThread.h"
#include "SkThreadUtils.h"
#include "SkTypes.h"

struct AddInfo {
    int32_t valueToAdd;
    int timesToAdd;
    unsigned int processorAffinity;
};

static int32_t base = 0;

static AddInfo gAdds[] = {
    { 3, 100, 23 },
    { 2, 200, 2 },
    { 7, 150, 17 },
};

static void addABunchOfTimes(void* data) {
    AddInfo* addInfo = static_cast<AddInfo*>(data);
    for (int i = 0; i < addInfo->timesToAdd; i++) {
        sk_atomic_add(&base, addInfo->valueToAdd);
    }
}

DEF_TEST(Atomic, reporter) {
    int32_t total = base;
    SkThread* threads[SK_ARRAY_COUNT(gAdds)];
    for (size_t i = 0; i < SK_ARRAY_COUNT(gAdds); i++) {
        total += gAdds[i].valueToAdd * gAdds[i].timesToAdd;
    }
    // Start the threads
    for (size_t i = 0; i < SK_ARRAY_COUNT(gAdds); i++) {
        threads[i] = new SkThread(addABunchOfTimes, &gAdds[i]);
        threads[i]->setProcessorAffinity(gAdds[i].processorAffinity);
        threads[i]->start();
    }

    // Now end the threads
    for (size_t i = 0; i < SK_ARRAY_COUNT(gAdds); i++) {
        threads[i]->join();
        delete threads[i];
    }
    REPORTER_ASSERT(reporter, total == base);
    // Ensure that the returned value from sk_atomic_add is correct.
    int32_t valueToModify = 3;
    const int32_t originalValue = valueToModify;
    REPORTER_ASSERT(reporter, originalValue == sk_atomic_add(&valueToModify, 7));
}
