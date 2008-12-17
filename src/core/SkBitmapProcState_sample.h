
#if DSTSIZE==32
    #define DSTTYPE SkPMColor
#elif DSTSIZE==16
    #define DSTTYPE uint16_t
#else
    #error "need DSTSIZE to be 32 or 16"
#endif

static void MAKENAME(_nofilter_DXDY)(const SkBitmapProcState& s,
                                     const uint32_t* SK_RESTRICT xy,
                                     int count, DSTTYPE* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fDoFilter == false);
    SkDEBUGCODE(CHECKSTATE(s);)

#ifdef PREAMBLE
    PREAMBLE(s);
#endif
    const char* SK_RESTRICT srcAddr = (const char*)s.fBitmap->getPixels();
    int i, rb = s.fBitmap->rowBytes();

    uint32_t XY;
    SRCTYPE src;
    
    for (i = (count >> 1); i > 0; --i) {
        XY = *xy++;
        SkASSERT((XY >> 16) < (unsigned)s.fBitmap->height() &&
                 (XY & 0xFFFF) < (unsigned)s.fBitmap->width());
        src = ((const SRCTYPE*)(srcAddr + (XY >> 16) * rb))[XY & 0xFFFF];
        *colors++ = RETURNDST(src);
        
        XY = *xy++;
        SkASSERT((XY >> 16) < (unsigned)s.fBitmap->height() &&
                 (XY & 0xFFFF) < (unsigned)s.fBitmap->width());
        src = ((const SRCTYPE*)(srcAddr + (XY >> 16) * rb))[XY & 0xFFFF];
        *colors++ = RETURNDST(src);
    }
    if (count & 1) {
        XY = *xy++;
        SkASSERT((XY >> 16) < (unsigned)s.fBitmap->height() &&
                 (XY & 0xFFFF) < (unsigned)s.fBitmap->width());
        src = ((const SRCTYPE*)(srcAddr + (XY >> 16) * rb))[XY & 0xFFFF];
        *colors++ = RETURNDST(src);
    }

#ifdef POSTAMBLE
    POSTAMBLE(s);
#endif
}

static void MAKENAME(_nofilter_DX)(const SkBitmapProcState& s,
                                   const uint32_t* SK_RESTRICT xy,
                                   int count, DSTTYPE* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fInvType <= (SkMatrix::kTranslate_Mask | SkMatrix::kScale_Mask));
    SkASSERT(s.fDoFilter == false);
    SkDEBUGCODE(CHECKSTATE(s);)

#ifdef PREAMBLE
    PREAMBLE(s);
#endif
    const SRCTYPE* SK_RESTRICT srcAddr = (const SRCTYPE*)s.fBitmap->getPixels();
    int i;

    // bump srcAddr to the proper row, since we're told Y never changes
    SkASSERT((unsigned)xy[0] < (unsigned)s.fBitmap->height());
    srcAddr = (const SRCTYPE*)((const char*)srcAddr +
                                                xy[0] * s.fBitmap->rowBytes());
    // buffer is y32, x16, x16, x16, x16, x16
    const uint16_t* SK_RESTRICT xx = (const uint16_t*)(xy + 1);
    
    SRCTYPE src;
    
    for (i = (count >> 2); i > 0; --i) {
        SkASSERT(*xx < (unsigned)s.fBitmap->width());
        src = srcAddr[*xx++]; *colors++ = RETURNDST(src);
        
        SkASSERT(*xx < (unsigned)s.fBitmap->width());
        src = srcAddr[*xx++]; *colors++ = RETURNDST(src);
        
        SkASSERT(*xx < (unsigned)s.fBitmap->width());
        src = srcAddr[*xx++]; *colors++ = RETURNDST(src);
        
        SkASSERT(*xx < (unsigned)s.fBitmap->width());
        src = srcAddr[*xx++]; *colors++ = RETURNDST(src);
    }
    for (i = (count & 3); i > 0; --i) {
        SkASSERT(*xx < (unsigned)s.fBitmap->width());
        src = srcAddr[*xx++]; *colors++ = RETURNDST(src);
    }
    
#ifdef POSTAMBLE
    POSTAMBLE(s);
#endif
}

///////////////////////////////////////////////////////////////////////////////

static void MAKENAME(_filter_DX)(const SkBitmapProcState& s,
                                 const uint32_t* SK_RESTRICT xy,
                                  int count, DSTTYPE* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fDoFilter);
    SkDEBUGCODE(CHECKSTATE(s);)

#ifdef PREAMBLE
    PREAMBLE(s);
#endif
    const char* SK_RESTRICT srcAddr = (const char*)s.fBitmap->getPixels();
    unsigned rb = s.fBitmap->rowBytes();
    unsigned subY;
    const SRCTYPE* SK_RESTRICT row0;
    const SRCTYPE* SK_RESTRICT row1;

    // setup row ptrs and update proc_table
    {
        uint32_t XY = *xy++;
        unsigned y0 = XY >> 14;
        row0 = (const SRCTYPE*)(srcAddr + (y0 >> 4) * rb);
        row1 = (const SRCTYPE*)(srcAddr + (XY & 0x3FFF) * rb);
        subY = y0 & 0xF;
    }
    
    do {
        uint32_t XX = *xy++;    // x0:14 | 4 | x1:14
        unsigned x0 = XX >> 14;
        unsigned x1 = XX & 0x3FFF;
        unsigned subX = x0 & 0xF;        
        x0 >>= 4;

        uint32_t c = FILTER_PROC(subX, subY,
                                 SRC_TO_FILTER(row0[x0]),
                                 SRC_TO_FILTER(row0[x1]),
                                 SRC_TO_FILTER(row1[x0]),
                                 SRC_TO_FILTER(row1[x1]));
        *colors++ = FILTER_TO_DST(c);

    } while (--count != 0);
    
#ifdef POSTAMBLE
    POSTAMBLE(s);
#endif
}
static void MAKENAME(_filter_DXDY)(const SkBitmapProcState& s,
                                   const uint32_t* SK_RESTRICT xy,
                                   int count, DSTTYPE* SK_RESTRICT colors) {
    SkASSERT(count > 0 && colors != NULL);
    SkASSERT(s.fDoFilter);
    SkDEBUGCODE(CHECKSTATE(s);)
        
#ifdef PREAMBLE
        PREAMBLE(s);
#endif
    const char* SK_RESTRICT srcAddr = (const char*)s.fBitmap->getPixels();
    int rb = s.fBitmap->rowBytes();
    
    do {
        uint32_t data = *xy++;
        unsigned y0 = data >> 14;
        unsigned y1 = data & 0x3FFF;
        unsigned subY = y0 & 0xF;
        y0 >>= 4;
        
        data = *xy++;
        unsigned x0 = data >> 14;
        unsigned x1 = data & 0x3FFF;
        unsigned subX = x0 & 0xF;
        x0 >>= 4;
        
        const SRCTYPE* SK_RESTRICT row0 = (const SRCTYPE*)(srcAddr + y0 * rb);
        const SRCTYPE* SK_RESTRICT row1 = (const SRCTYPE*)(srcAddr + y1 * rb);
        
        uint32_t c = FILTER_PROC(subX, subY,
                                 SRC_TO_FILTER(row0[x0]),
                                 SRC_TO_FILTER(row0[x1]),
                                 SRC_TO_FILTER(row1[x0]),
                                 SRC_TO_FILTER(row1[x1]));
        *colors++ = FILTER_TO_DST(c);
    } while (--count != 0);
    
#ifdef POSTAMBLE
    POSTAMBLE(s);
#endif
}

#undef MAKENAME
#undef DSTSIZE
#undef DSTTYPE
#undef SRCTYPE
#undef CHECKSTATE
#undef RETURNDST
#undef SRC_TO_FILTER
#undef FILTER_TO_DST

#ifdef PREAMBLE
    #undef PREAMBLE
#endif
#ifdef POSTAMBLE
    #undef POSTAMBLE
#endif

#undef FILTER_PROC_TYPE
#undef GET_FILTER_TABLE
#undef GET_FILTER_ROW
#undef GET_FILTER_ROW_PROC
#undef GET_FILTER_PROC
