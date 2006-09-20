#include "SkMaskFilter.h"
#include "SkBlitter.h"
#include "SkBounder.h"
#include "SkBuffer.h"
#include "SkDraw.h"
#include "SkRegion.h"

size_t SkMask::computeImageSize() const
{
	return fBounds.height() * fRowBytes;
}

size_t SkMask::computeTotalImageSize() const
{
	size_t size = this->computeImageSize();

	if (fFormat == SkMask::k3D_Format)
		size *= 3;
	return size;
}

uint8_t* SkMask::AllocImage(size_t size)
{
	return (uint8_t*)sk_malloc_throw(SkAlign4(size));
}

void SkMask::FreeImage(uint8_t* image)
{
	sk_free(image);
}

bool SkMaskFilter::filterMask(SkMask*, const SkMask&, const SkMatrix&, SkPoint16*)
{
	return false;
}

bool SkMaskFilter::filterPath(const SkPath& devPath, const SkMatrix& matrix,
							  const SkRegion& clip, SkBounder* bounder,
							  SkBlitter* blitter)
{
	SkMask  srcM, dstM;

    if (!SkDraw::DrawToMask(devPath, &clip.getBounds(), this, &matrix, &srcM,
                            SkMask::kComputeBoundsAndRenderImage_CreateMode))
    {
        return false;
    }

	SkAutoMaskImage autoSrc(&srcM, false);

	if (!this->filterMask(&dstM, srcM, matrix, NULL))
		return false;

	SkAutoMaskImage			autoDst(&dstM, false);
	SkRegion::Cliperator	clipper(clip, dstM.fBounds);

	if (!clipper.done() && (bounder == NULL || bounder->doIRect(dstM.fBounds, clip)))
	{
		const SkRect16&	cr = clipper.rect();
		do {
			blitter->blitMask(dstM, cr);
			clipper.next();
		} while (!clipper.done());
	}

	return true;
}


