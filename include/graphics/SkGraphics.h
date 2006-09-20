#ifndef SkGraphics_DEFINED
#define SkGraphics_DEFINED

#include "SkTypes.h"

class SkGraphics {
public:
	static void Init(bool runUnitTests);
	static void Term();

	/**	Call this if the heap that the graphics engine uses is low on memory.
		It will attempt to free some of its caches. Returns true if it was
		able to, or false if it could do nothing.

		This may be called from any thread, and guarantees not to call
		new or sk_malloc (though it will hopefully call delete and/or sk_free).
		It also will never throw an exception.
	*/
	static bool FreeCaches(size_t bytesNeeded);

private:
	/**	This is automatically called by SkGraphics::Init(), and must be
		implemented by the host OS. This allows the host OS to register a callback
		with the C++ runtime to call SkGraphics::FreeCaches()
	*/
	static void InstallNewHandler();
};

#endif

