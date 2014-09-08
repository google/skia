#ifndef DMExpectations_DEFINED
#define DMExpectations_DEFINED

#include "DMTask.h"

namespace DM {

struct Expectations {
    virtual ~Expectations() {}

    // Return true if bitmap is the correct output for task, else false.
    virtual bool check(const Task& task, SkBitmap bitmap) const {
        return true;
    }
};

}  // namespace DM

#endif // DMExpectations_DEFINED
