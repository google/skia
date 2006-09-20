#ifndef SkString_DEFINED
#define SkString_DEFINED

#include "SkScalar.h"

/*	Some helper functions for C strings
*/

bool SkStrStartsWith(const char string[], const char prefix[]);
bool SkStrEndsWith(const char string[], const char suffix[]);
int SkStrStartsWithOneOf(const char string[], const char prefixes[]);

#define SkStrAppendS32_MaxSize  11
char*   SkStrAppendS32(char buffer[], int32_t);
#define SkStrAppendScalar_MaxSize  11
char*   SkStrAppendScalar(char buffer[], SkScalar);

/**	\class SkString

	Light weight class for managing strings. Uses reference
	counting to make string assignments and copies very fast
	with no extra RAM cost. Assumes UTF8 encoding.
*/
class SkString {
public:
				SkString();
	explicit	SkString(size_t len);
	explicit	SkString(const char text[]);
				SkString(const char text[], size_t len);
	explicit	SkString(const SkString&);
				~SkString();

	bool		isEmpty() const { return fRec->fLength == 0; }
	size_t		size() const { return (size_t) fRec->fLength; }
	const char*	c_str() const { return fRec->data(); }

	bool	equals(const SkString&) const;
	bool	equals(const char text[]) const;
	bool	equals(const char text[], size_t len) const;

	bool	startsWith(const char prefix[]) const
	{
		return SkStrStartsWith(fRec->data(), prefix);
	}
	bool	endsWith(const char suffix[]) const
	{
		return SkStrEndsWith(fRec->data(), suffix);
	}

	friend int operator==(const SkString& a, const SkString& b)
	{
		return a.equals(b);
	}
	friend int operator!=(const SkString& a, const SkString& b)
	{
		return !a.equals(b);
	}

	// these methods edit the string

	SkString&	operator=(const SkString&);

	char*	writable_str();

	void	reset();
	void	resize(size_t len) { this->set(nil, len); }
	void	set(const SkString& src) { *this = src; }
	void	set(const char text[]);
	void	set(const char text[], size_t len);
	void	setUTF16(const U16[]);

	void	insert(size_t offset, const SkString& src) { this->insert(offset, src.c_str(), src.size()); }
	void	insert(size_t offset, const char text[]);
	void	insert(size_t offset, const char text[], size_t len);
	void	insertUnichar(size_t offset, SkUnichar);
	void	insertS32(size_t offset, S32 value);
	void	insertHex(size_t offset, U32 value, int minDigits = 0);
	void	insertScalar(size_t offset, SkScalar);

	void	append(const SkString& str) { this->insert((size_t)-1, str); }
	void	append(const char text[]) { this->insert((size_t)-1, text); }
	void	append(const char text[], size_t len) { this->insert((size_t)-1, text, len); }
	void	appendUnichar(SkUnichar uni) { this->insertUnichar((size_t)-1, uni); }
	void	appendS32(S32 value) { this->insertS32((size_t)-1, value); }
	void	appendHex(U32 value, int minDigits = 0) { this->insertHex((size_t)-1, value, minDigits); }
	void	appendScalar(SkScalar value) { this->insertScalar((size_t)-1, value); }

	void	prepend(const SkString& str) { this->insert(0, str); }
	void	prepend(const char text[]) { this->insert(0, text); }
	void	prepend(const char text[], size_t len) { this->insert(0, text, len); }
	void	prependUnichar(SkUnichar uni) { this->insertUnichar(0, uni); }
	void	prependS32(S32 value) { this->insertS32(0, value); }
	void	prependHex(U32 value, int minDigits = 0) { this->insertHex(0, value, minDigits); }
	void	prependScalar(SkScalar value) { this->insertScalar((size_t)-1, value); }

	void	printf(const char format[], ...);

	void	remove(size_t offset, size_t length);

	/**	Swap contents between this and other. This function is guaranteed
		to never fail or throw.
	*/
	void	swap(SkString& other);

  /** @cond UNIT_TEST */
	SkDEBUGCODE(static void UnitTest();)
  /** @endcond */
    
private:
#ifdef SK_DEBUG
	const char* fStr;
#endif
	struct Rec {
		U16	fLength;
		U16	fRefCnt;
		// data[]
		char* data() { return (char*)(this) + sizeof(Rec); }
		const char* data() const { return (const char*)(this) + sizeof(Rec); }
	};
	Rec* fRec;

#ifdef SK_DEBUG
	void validate() const;
#else
	void validate() const {}
#endif

	static Rec*	AllocRec(const char text[], U16CPU len);
	static Rec*	RefRec(Rec*);
};

class SkAutoUCS2 {
public:
	SkAutoUCS2(const char utf8[]);
	~SkAutoUCS2();

	/**	This returns the number of ucs2 characters
	*/
	int			count() const { return fCount; }
	/**	This returns a null terminated ucs2 string
	*/
	const U16*	getUCS2() const { return fUCS2; }

private:
	int		fCount;
	U16*	fUCS2;
};

#endif

