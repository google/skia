#ifndef SkParsePaint_DEFINED
#define SkParsePaint_DEFINED

#include "SkPaint.h"
#include "SkDOM.h"

/**	"color"				color
	"opacity"			scalar	[0..1]
	"stroke-width"		scalar	(0...inf)
	"text-size"			scalar	(0..inf)
	"is-stroke"			bool
	"is-antialias"		bool
	"is-lineartext"		bool
*/
void SkPaint_Inflate(SkPaint*, const SkDOM&, const SkDOM::Node*);

#endif

