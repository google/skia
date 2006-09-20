#ifndef SkStream_Win_DEFINED
#define SkStream_Win_DEFINED

#ifndef SK_BUILD_FOR_WIN
#error "only valid for windows and wince builds"
#endif

#ifndef SkStream_DEFINED
#include "SkStream.h"
#endif
#include "SkString.h"
#include "Wininet.h"

/** \cond ZERO */
class SkURLStream : public SkStream {
public:
	SkURLStream(const char url[] = nil);
	virtual ~SkURLStream();

	/**	Close the current URL, and open a new URL.
		If URL is nil, just close the current URL.
	*/
	void setURL(const char url[]);

	// overrides
	virtual bool rewind();
	virtual size_t read(void* buffer, size_t size);
	
private:
	SkString fURL;
	HINTERNET fConnection;
	HINTERNET fURLStream;
};

/** \endcond */
#endif // SkStream_Win_DEFINED

