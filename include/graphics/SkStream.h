#ifndef SkStream_DEFINED
#define SkStream_DEFINED

#include "SkScalar.h"

class SkStream {
public:
	virtual ~SkStream() {}
	/**	Called to rewind to the beginning of the stream. If this cannot be
		done, return false.
	*/
	virtual bool rewind() = 0;
	/**	If this stream represents a file, this method returns the file's name.
		If it does not, it returns nil (the default behavior).
	*/
	virtual const char* getFileName();
	/**	Called to read or skip size number of bytes. If buffer is nil, skip
		the bytes, else copy them into buffer. If this cannot be done, return false.
		If buffer is nil and size is zero, return the file length
		@param buffer	If buffer is nil, ignore and just skip size bytes, otherwise copy size bytes into buffer
		@param size	The number of bytes to skip or copy
		@return bytes read on success
	*/
	virtual size_t read(void* buffer, size_t size) = 0;
	static SkStream* GetURIStream(const char prefix[], const char path[]);
	static bool IsAbsoluteURI(const char path[]);
};

class SkWStream {
public:
	virtual ~SkWStream();

	/**	Called to write bytes to a SkWStream. Returns true on success
		@param buffer the address of at least size bytes to be written to the stream
		@param size	The number of bytes in buffer to write to the stream
		@return true on success
	*/
	virtual bool write(const void* buffer, size_t size) = 0;
	virtual void newline();
	virtual void flush();

	// helpers

	bool	writeText(const char text[]);
	bool	writeDecAsText(S32);
	bool	writeHexAsText(U32, int minDigits = 0);
	bool	writeScalarAsText(SkScalar);

	SkDEBUGCODE(static void UnitTest();)
};

////////////////////////////////////////////////////////////////////////////////////////

#include "SkString.h"

struct SkFILE;

class SkFILEStream : public SkStream {
public:
	SkFILEStream(const char path[] = nil);
	virtual ~SkFILEStream();

	/**	Returns true if the current path could be opened.
	*/
	bool isValid() const { return fFILE != nil; }
	/**	Close the current file, and open a new file with the specified
		path. If path is nil, just close the current file.
	*/
	void setPath(const char path[]);
    
    SkFILE* getSkFILE() const { return fFILE; }

	virtual bool rewind();
	virtual size_t read(void* buffer, size_t size);
	virtual const char* getFileName();

private:
	SkFILE*		fFILE;
	SkString	fName;
};

class SkMemoryStream : public SkStream {
public:
	SkMemoryStream(const void* src, size_t length);

	virtual bool rewind();
	virtual size_t read(void* buffer, size_t size);

private:
	const void* fSrc;
	size_t fSize, fOffset;
};

/**	\class SkBufferStream
	This is a wrapper class that adds buffering to another stream.
	The caller can provide the buffer, or ask SkBufferStream to allocated/free
	it automatically.
*/
class SkBufferStream : public SkStream {
public:
	/**	Provide the stream to be buffered (proxy), and the size of the buffer that
		should be used. This will be allocated and freed automatically. If bufferSize is 0,
		a default buffer size will be used.
	*/
	SkBufferStream(SkStream& proxy, size_t bufferSize = 0);
	/**	Provide the stream to be buffered (proxy), and a buffer and size to be used.
		This buffer is owned by the caller, and must be at least bufferSize bytes big.
		Passing nil for buffer will cause the buffer to be allocated/freed automatically.
		If buffer is not nil, it is an error for bufferSize to be 0.
	*/
	SkBufferStream(SkStream& proxy, void* buffer, size_t bufferSize);
	virtual ~SkBufferStream();

	virtual bool		rewind();
	virtual const char*	getFileName();
	virtual size_t		read(void* buffer, size_t size);
private:
	enum {
		kDefaultBufferSize	= 128
	};
	// illegal
	SkBufferStream(const SkBufferStream&);
	SkBufferStream&	operator=(const SkBufferStream&);

	SkStream&	fProxy;
	char*		fBuffer;
	size_t		fOrigBufferSize, fBufferSize, fBufferOffset;
	bool		fWeOwnTheBuffer;

	void	init(void*, size_t);
};

/////////////////////////////////////////////////////////////////////////////////////////////

class SkFILEWStream : public SkWStream {
public:
			SkFILEWStream(const char path[]);
	virtual ~SkFILEWStream();

	/**	Returns true if the current path could be opened.
	*/
	bool isValid() const { return fFILE != nil; }

	virtual bool write(const void* buffer, size_t size);
	virtual void flush();
private:
	SkFILE* fFILE;
};

class SkMemoryWStream : public SkWStream {
public:
	SkMemoryWStream(void* buffer, size_t size);
	virtual bool write(const void* buffer, size_t size);
    
private:
    char*   fBuffer;
    size_t  fMaxLength;
	size_t  fBytesWritten;
};

class SkDynamicMemoryWStream : public SkWStream {
public:
	SkDynamicMemoryWStream();
	virtual ~SkDynamicMemoryWStream();
	virtual bool write(const void* buffer, size_t size);
    // random access write
    // modifies stream and returns true if offset + size is less than or equal to getOffset()
    bool write(const void* buffer, size_t offset, size_t size); 
	size_t getOffset() { return fBytesWritten; }

    // copy what has been written to the stream into dst
    void    copyTo(void* dst) const;
    /*  return a cache of the flattened data returned by copyTo().
        This copy is only valid until the next call to write().
        The memory is managed by the stream class.
    */
    const char* getStream() const;

private:
    struct Block;
    Block*  fHead;
    Block*  fTail;
	size_t  fBytesWritten;
    mutable char*   fCopyToCache;
};


class SkDebugWStream : public SkWStream {
public:
	// overrides
	virtual bool write(const void* buffer, size_t size);
	virtual void newline();
};

// for now
typedef SkFILEStream SkURLStream;

#endif

