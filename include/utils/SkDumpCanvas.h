
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#ifndef SkDumpCanvas_DEFINED
#define SkDumpCanvas_DEFINED

#include "SkCanvas.h"

#ifdef SK_DEVELOPER

/** This class overrides all the draw methods on SkCanvas, and formats them
    as text, and then sends that to a Dumper helper object.

    Typical use might be to dump a display list to a log file to see what is
    being drawn.
 */
class SkDumpCanvas : public SkCanvas {
public:
    class Dumper;

    explicit SkDumpCanvas(Dumper* = 0);
    virtual ~SkDumpCanvas();

    enum Verb {
        kNULL_Verb,

        kSave_Verb,
        kRestore_Verb,

        kMatrix_Verb,

        kClip_Verb,

        kDrawPaint_Verb,
        kDrawPoints_Verb,
        kDrawOval_Verb,
        kDrawRect_Verb,
        kDrawRRect_Verb,
        kDrawPath_Verb,
        kDrawBitmap_Verb,
        kDrawText_Verb,
        kDrawPicture_Verb,
        kDrawVertices_Verb,
        kDrawData_Verb
    };

    /** Subclasses of this are installed on the DumpCanvas, and then called for
        each drawing command.
     */
    class Dumper : public SkRefCnt {
    public:
        SK_DECLARE_INST_COUNT(Dumper)

        virtual void dump(SkDumpCanvas*, SkDumpCanvas::Verb, const char str[],
                          const SkPaint*) = 0;

    private:
        typedef SkRefCnt INHERITED;
    };

    Dumper* getDumper() const { return fDumper; }
    void    setDumper(Dumper*);

    int getNestLevel() const { return fNestLevel; }

    virtual int save(SaveFlags) SK_OVERRIDE;
    virtual int saveLayer(const SkRect* bounds, const SkPaint* paint,
                          SaveFlags) SK_OVERRIDE;
    virtual void restore() SK_OVERRIDE;

    virtual bool translate(SkScalar dx, SkScalar dy) SK_OVERRIDE;
    virtual bool scale(SkScalar sx, SkScalar sy) SK_OVERRIDE;
    virtual bool rotate(SkScalar degrees) SK_OVERRIDE;
    virtual bool skew(SkScalar sx, SkScalar sy) SK_OVERRIDE;
    virtual bool concat(const SkMatrix& matrix) SK_OVERRIDE;
    virtual void setMatrix(const SkMatrix& matrix) SK_OVERRIDE;

    virtual bool clipRect(const SkRect&, SkRegion::Op, bool) SK_OVERRIDE;
    virtual bool clipRRect(const SkRRect&, SkRegion::Op, bool) SK_OVERRIDE;
    virtual bool clipPath(const SkPath&, SkRegion::Op, bool) SK_OVERRIDE;
    virtual bool clipRegion(const SkRegion& deviceRgn,
                            SkRegion::Op) SK_OVERRIDE;

    virtual void drawPaint(const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPoints(PointMode mode, size_t count, const SkPoint pts[],
                            const SkPaint& paint) SK_OVERRIDE;
    virtual void drawOval(const SkRect&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRect(const SkRect&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawRRect(const SkRRect&, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPath(const SkPath& path, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawBitmap(const SkBitmap& bitmap, SkScalar left, SkScalar top,
                            const SkPaint* paint) SK_OVERRIDE;
    virtual void drawBitmapRectToRect(const SkBitmap& bitmap, const SkRect* src,
                                const SkRect& dst, const SkPaint* paint) SK_OVERRIDE;
    virtual void drawBitmapMatrix(const SkBitmap& bitmap, const SkMatrix& m,
                                  const SkPaint* paint) SK_OVERRIDE;
    virtual void drawSprite(const SkBitmap& bitmap, int left, int top,
                            const SkPaint* paint) SK_OVERRIDE;
    virtual void drawText(const void* text, size_t byteLength, SkScalar x,
                          SkScalar y, const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPosText(const void* text, size_t byteLength,
                             const SkPoint pos[], const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPosTextH(const void* text, size_t byteLength,
                              const SkScalar xpos[], SkScalar constY,
                              const SkPaint& paint) SK_OVERRIDE;
    virtual void drawTextOnPath(const void* text, size_t byteLength,
                                const SkPath& path, const SkMatrix* matrix,
                                const SkPaint& paint) SK_OVERRIDE;
    virtual void drawPicture(SkPicture&) SK_OVERRIDE;
    virtual void drawVertices(VertexMode vmode, int vertexCount,
                              const SkPoint vertices[], const SkPoint texs[],
                              const SkColor colors[], SkXfermode* xmode,
                              const uint16_t indices[], int indexCount,
                              const SkPaint& paint) SK_OVERRIDE;
    virtual void drawData(const void*, size_t) SK_OVERRIDE;

private:
    Dumper* fDumper;
    int     fNestLevel; // for nesting recursive elements like pictures

    void dump(Verb, const SkPaint*, const char format[], ...);

    typedef SkCanvas INHERITED;
};

/** Formats the draw commands, and send them to a function-pointer provided
    by the caller.
 */
class SkFormatDumper : public SkDumpCanvas::Dumper {
public:
    SkFormatDumper(void (*)(const char text[], void* refcon), void* refcon);

    // override from baseclass that does the formatting, and in turn calls
    // the function pointer that was passed to the constructor
    virtual void dump(SkDumpCanvas*, SkDumpCanvas::Verb, const char str[],
                      const SkPaint*) SK_OVERRIDE;

private:
    void (*fProc)(const char*, void*);
    void* fRefcon;

    typedef SkDumpCanvas::Dumper INHERITED;
};

/** Subclass of Dumper that dumps the drawing command to SkDebugf
 */
class SkDebugfDumper : public SkFormatDumper {
public:
    SkDebugfDumper();

private:
    typedef SkFormatDumper INHERITED;
};

#endif

#endif
