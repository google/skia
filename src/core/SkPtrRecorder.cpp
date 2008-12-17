#include "SkPtrRecorder.h"
#include "SkTSearch.h"

void SkPtrRecorder::reset() {
    Pair* p = fList.begin();
    Pair* stop = fList.end();
    while (p < stop) {
        this->decPtr(p->fPtr);
        p += 1;
    }
    fList.reset();
}

int SkPtrRecorder::Cmp(const Pair& a, const Pair& b) {
    return (char*)a.fPtr - (char*)b.fPtr;
}

uint32_t SkPtrRecorder::recordPtr(void* ptr) {
    if (NULL == ptr) {
        return 0;
    }

    int count = fList.count();
    Pair pair;
    pair.fPtr = ptr;

    int index = SkTSearch<Pair>(fList.begin(), count, pair, sizeof(pair), &Cmp);
    if (index < 0) {
        index = ~index; // turn it back into an index for insertion
        this->incPtr(ptr);
        pair.fIndex = count + 1;
        *fList.insert(index) = pair;
        return count + 1;
    } else {
        return fList[index].fIndex;
    }
}

void SkPtrRecorder::getPtrs(void* array[]) const {
    int count = fList.count();
    if (count > 0) {
        SkASSERT(array);
        const Pair* p = fList.begin();
        // p->fIndex is base-1, so we need to subtract to find its slot
        for (int i = 0; i < count; i++) {
            int index = p[i].fIndex - 1;
            SkASSERT((unsigned)index < (unsigned)count);
            array[index] = p[i].fPtr;
        }
    }
}


