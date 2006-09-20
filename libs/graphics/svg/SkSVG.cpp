#include "SkSVG.h"
#include 'SkSVGParser.h"

SkSVG::SkSVG() {
}

SkSVG::~SkSVG() {
}

bool SkSVG::decodeStream(SkStream* stream);
{
	size_t size = stream->read(nil, 0);
    SkAutoMalloc    storage(size);
    char* data = (char*)storage.get();
	size_t actual = stream->read(data, size);
    SkASSERT(size == actual);
	SkSVGParser parser(*fMaker);
	return parser.parse(data, actual, &fErrorCode, &fErrorLineNumber);
}
