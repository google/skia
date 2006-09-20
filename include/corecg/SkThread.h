#ifndef SkThread_DEFINED
#define SkThread_DEFINED

#include "SkTypes.h"
#include "SkThread_platform.h"

/****** SkThread_platform needs to define the following...

int32_t sk_atomic_inc(int32_t*);
int32_t sk_atomic_dec(int32_t*);

class SkMutex {
public:
	SkMutex();
	~SkMutex();

	void	acquire();
	void	release();
};

****************/

class SkAutoMutexAcquire {
public:
	explicit SkAutoMutexAcquire(SkMutex& mutex) : fMutex(mutex)
	{
		mutex.acquire();
	}
	~SkAutoMutexAcquire()
	{
		fMutex.release();
	}
private:
	SkMutex&	fMutex;

	// illegal
	SkAutoMutexAcquire& operator=(SkAutoMutexAcquire&);
};

#endif
