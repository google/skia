#ifndef SkThread_platform_DEFINED
#define SkThread_platform_DEFINED

#ifdef ANDROID

#include <utils/threads.h>
#include <utils/Atomic.h>

#define sk_atomic_inc(addr)     android_atomic_inc(addr)
#define sk_atomic_dec(addr)     android_atomic_dec(addr)

class SkMutex : android::Mutex {
public:
	SkMutex() {}
	~SkMutex() {}

	void	acquire() { this->lock(); }
	void	release() { this->unlock(); }
};

#else   /* SkThread_empty.cpp */

int32_t sk_atomic_inc(int32_t* addr);
int32_t sk_atomic_dec(int32_t* addr);

class SkMutex {
public:
	SkMutex();
	~SkMutex();

	void	acquire();
	void	release();
};

#endif

#endif
