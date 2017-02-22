#ifndef SkExecutor_DEFINED
#define SkExecutor_DEFINED

#include <functional>

class SkExecutor {
public:
    virtual ~SkExecutor();

    static SkExecutor* NewThreadPool(int threads = 0);  // By default, number of cores.

    static SkExecutor& GetDefault();
    static void SetDefault(SkExecutor*);  // Does not take ownership.  Not thread safe.

    // Add work to execute.
    virtual void add(std::function<void(void)>) = 0;

    // If it makes sense for this executor, yield this thread's time to others.
    virtual void yield() {}

    // If it makes sense for this executor, use this thread to execute work for a little while.
    virtual void borrow() {}
};

#endif//SkExecutor_DEFINED
