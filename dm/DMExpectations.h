#ifndef DMExpectations_DEFINED
#define DMExpectations_DEFINED

#include "DMTask.h"
#include "gm_expectations.h"

namespace DM {

struct Expectations {
    virtual ~Expectations() {}

    // Return true if bitmap is the correct output for task, else false.
    virtual bool check(const Task& task, SkBitmap bitmap) const = 0;
};

class NoExpectations : public Expectations {
public:
    NoExpectations() {}
    bool check(const Task&, SkBitmap) const SK_OVERRIDE { return true; }
};

class JsonExpectations : public Expectations {
public:
    explicit JsonExpectations(const char* path) : fGMExpectations(path) {}

    bool check(const Task& task, SkBitmap bitmap) const SK_OVERRIDE {
        SkString filename = task.name();
        filename.append(".png");
        const skiagm::Expectations expectations = fGMExpectations.get(filename.c_str());

        if (expectations.ignoreFailure() || expectations.empty()) {
            return true;
        }

        // Delay this calculation as long as possible.  It's expensive.
        const skiagm::GmResultDigest digest(bitmap);
        return expectations.match(digest);
    }

private:
    skiagm::JsonExpectationsSource fGMExpectations;
};

}  // namespace DM

#endif // DMExpectations_DEFINED
