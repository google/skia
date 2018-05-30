#ifndef SkMutex_DEFINED
#define SkMutex_DEFINED

#define SK_DECLARE_STATIC_MUTEX(name) static SkBaseMutex name;

class SkBaseMutex {
public:
};

class SkAutoMutexAcquire {
public:
    template <typename T>
    SkAutoMutexAcquire(T* mutex);
    
};

#endif
