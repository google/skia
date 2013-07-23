#ifndef __DEFINED__SkPdfConfig
#define __DEFINED__SkPdfConfig

//#define PDF_TRACE
//#define PDF_TRACE_READ_TOKEN
//#define PDF_TRACE_DIFF_IN_PNG
//#define PDF_DEBUG_NO_CLIPING
//#define PDF_DEBUG_NO_PAGE_CLIPING
//#define PDF_DEBUG_3X

// TODO(edisonn): move in trace util.
#ifdef PDF_TRACE
void SkTraceMatrix(const SkMatrix& matrix, const char* sz);
void SkTraceRect(const SkRect& rect, const char* sz);
#else
#define SkTraceMatrix(a,b)
#define SkTraceRect(a,b)
#endif

struct NotOwnedString {
    const unsigned char* fBuffer;
    size_t fBytes;

    static void init(NotOwnedString* str) {
        str->fBuffer = NULL;
        str->fBytes = 0;
    }

    static void init(NotOwnedString* str, const char* sz) {
        str->fBuffer = (const unsigned char*)sz;
        str->fBytes = strlen(sz);
    }

    bool equals(const char* sz) {
        return strncmp((const char*)fBuffer, sz, fBytes) == 0 && fBytes == strlen(sz);

    }
};

#endif  // __DEFINED__SkPdfConfig
