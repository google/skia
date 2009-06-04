#include "SkParsePaint.h"
#include "SkTSearch.h"
#include "SkParse.h"
#include "SkImageDecoder.h"
#include "SkGradientShader.h"

static SkShader* inflate_shader(const SkDOM& dom, const SkDOM::Node* node)
{
	if ((node = dom.getFirstChild(node, "shader")) == NULL)
		return NULL;

	const char* str;

	if (dom.hasAttr(node, "type", "linear-gradient"))
	{
		SkColor		colors[2];
		SkPoint		pts[2];

		colors[0] = colors[1] = SK_ColorBLACK;	// need to initialized the alpha to opaque, since FindColor doesn't set it
		if ((str = dom.findAttr(node, "c0")) != NULL &&
			SkParse::FindColor(str, &colors[0]) &&
			(str = dom.findAttr(node, "c1")) != NULL &&
			SkParse::FindColor(str, &colors[1]) &&
			dom.findScalars(node, "p0", &pts[0].fX, 2) &&
			dom.findScalars(node, "p1", &pts[1].fX, 2))
		{
			SkShader::TileMode	mode = SkShader::kClamp_TileMode;
			int					index;

			if ((index = dom.findList(node, "tile-mode", "clamp,repeat,mirror")) >= 0)
				mode = (SkShader::TileMode)index;
			return SkGradientShader::CreateLinear(pts, colors, NULL, 2, mode);
		}
	}
	else if (dom.hasAttr(node, "type", "bitmap"))
	{
		if ((str = dom.findAttr(node, "src")) == NULL)
			return NULL;

		SkBitmap	bm;

		if (SkImageDecoder::DecodeFile(str, &bm))
		{
			SkShader::TileMode	mode = SkShader::kRepeat_TileMode;
			int					index;

			if ((index = dom.findList(node, "tile-mode", "clamp,repeat,mirror")) >= 0)
				mode = (SkShader::TileMode)index;

			return SkShader::CreateBitmapShader(bm, mode, mode);
		}
	}
	return NULL;
}

void SkPaint_Inflate(SkPaint* paint, const SkDOM& dom, const SkDOM::Node* node)
{
	SkASSERT(paint);
	SkASSERT(&dom);
	SkASSERT(node);

	SkScalar x;

	if (dom.findScalar(node, "stroke-width", &x))
		paint->setStrokeWidth(x);
	if (dom.findScalar(node, "text-size", &x))
		paint->setTextSize(x);
	
	bool	b;

	SkASSERT("legacy: use is-stroke" && !dom.findBool(node, "is-frame", &b));

	if (dom.findBool(node, "is-stroke", &b))
		paint->setStyle(b ? SkPaint::kStroke_Style : SkPaint::kFill_Style);
	if (dom.findBool(node, "is-antialias", &b))
		paint->setAntiAlias(b);
	if (dom.findBool(node, "is-lineartext", &b))
		paint->setLinearText(b);

	const char* str = dom.findAttr(node, "color");
	if (str)
	{
		SkColor	c = paint->getColor();
		if (SkParse::FindColor(str, &c))
			paint->setColor(c);
	}

	// do this AFTER parsing for the color
	if (dom.findScalar(node, "opacity", &x))
	{
		x = SkMaxScalar(0, SkMinScalar(x, SK_Scalar1));
		paint->setAlpha(SkScalarRound(x * 255));
	}

	int	index = dom.findList(node, "text-anchor", "left,center,right");
	if (index >= 0)
		paint->setTextAlign((SkPaint::Align)index);

	SkShader* shader = inflate_shader(dom, node);
	if (shader)
		paint->setShader(shader)->unref();
}

