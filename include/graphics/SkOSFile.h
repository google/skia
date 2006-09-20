// Copyright Skia Inc. 2004 - 2005
// 
#ifndef SkOSFile_DEFINED
#define SkOSFile_DEFINED

#include "SkString.h"

#if defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX)
	#include <dirent.h>
#endif

struct SkFILE;

enum SkFILE_Flags {
	kRead_SkFILE_Flag	= 0x01,
	kWrite_SkFILE_Flag	= 0x02
};

SkFILE*	sk_fopen(const char path[], SkFILE_Flags);
void	sk_fclose(SkFILE*);

size_t	sk_fgetsize(SkFILE*);
/**	Return true if the file could seek back to the beginning
*/
bool	sk_frewind(SkFILE*);

size_t	sk_fread(void* buffer, size_t byteCount, SkFILE*);
size_t	sk_fwrite(const void* buffer, size_t byteCount, SkFILE*);
void	sk_fflush(SkFILE*);

int		sk_fseek( SkFILE*, size_t, int );
size_t	sk_ftell( SkFILE* );

class SkOSFile {
public:
	class Iter {
	public:
		Iter();
		Iter(const char path[], const char suffix[] = nil);
		~Iter();

		void reset(const char path[], const char suffix[] = nil);
		bool next(SkString* name, bool getDir = false);

	private:
#ifdef SK_BUILD_FOR_WIN
		HANDLE		fHandle;
		U16*		fPath16;
#elif defined(SK_BUILD_FOR_MAC) || defined(SK_BUILD_FOR_UNIX)
		DIR*		fDIR;
		SkString	fPath, fSuffix;
#endif
	};
};

class SkUTF16_Str {
public:
	SkUTF16_Str(const char src[]);
	~SkUTF16_Str()
	{
		sk_free(fStr);
	}
	const U16* get() const { return fStr; }

private:
	U16*	fStr;
};

#endif

