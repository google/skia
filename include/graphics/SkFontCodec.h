#ifndef SkFontCodec_DEFINED
#define SkFontCodec_DEFINED

#include "SkSFNT.h"

class SkFontCodec {
public:
	static void Compress(SkSFNT& font, const char fileName[]);

	/*	Format is [count] + [instruction, bitcount] * count
		Allocated with sk_malloc()
	*/
	static U8* BuildInstrHuffmanTable(SkSFNT&);
	static U8* BuildOutlineHuffmanTable(SkSFNT& font);

	SkDEBUGCODE(static void UnitTest();)
};

#endif

