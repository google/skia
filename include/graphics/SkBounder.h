#ifndef SkBounder_DEFINED
#define SkBounder_DEFINED

#include "SkTypes.h"
#include "SkRefCnt.h"

struct SkRect16;
struct SkPoint;
struct SkRect;
class SkPaint;
class SkPath;
class SkRegion;

/**	\class SkBounder

	Base class for intercepting the device bounds of shapes before they are drawn.
	Install a subclass of this in your canvas.
*/
class SkBounder : public SkRefCnt {
public:
	bool doIRect(const SkRect16&, const SkRegion&);
	bool doHairline(const SkPoint&, const SkPoint&, const SkPaint&, const SkRegion&);
	bool doRect(const SkRect&, const SkPaint&, const SkRegion&);
	bool doPath(const SkPath&, const SkPaint&, const SkRegion&, bool doFill);

protected:
	/**	Override in your subclass. This is called with the device bounds of an
		object (text, geometry, image) just before it is drawn. If your method
		returns false, the drawing for that shape is aborted. If your method
		returns true, drawing continues. The bounds your method receives have already
		been transformed in to device coordinates, and clipped to the current clip.
	*/
	virtual bool onIRect(const SkRect16&) = 0;

	/**	Called after each shape has been drawn. The default implementation does
		nothing, but your override could use this notification to signal itself
		that the offscreen being rendered into needs to be updated to the screen.
	*/
	virtual void commit();

	friend class SkAutoBounderCommit;
};

#endif

