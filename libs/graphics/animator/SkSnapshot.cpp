#include "SkTypes.h"

#ifdef SK_SUPPORT_IMAGE_ENCODE

#include "SkSnapshot.h"
#include "SkAnimateMaker.h"
#include "SkCanvas.h"

#if SK_USE_CONDENSED_INFO == 0

const SkMemberInfo SkSnapshot::fInfo[] = {
	SK_MEMBER(filename, String),
	SK_MEMBER(quality, Float),
	SK_MEMBER(sequence, Boolean),
	SK_MEMBER(type, BitmapEncoding)
};

#endif

DEFINE_GET_MEMBER(SkSnapshot);

SkSnapshot::SkSnapshot()
{
	quality		= 100 * SK_Scalar1;
	type		= (SkImageEncoder::Type) -1;
	sequence	= false;
	fSeqVal		= 0;
}

bool SkSnapshot::draw(SkAnimateMaker& maker) {
	SkASSERT(type >= 0);
	SkASSERT(filename.size() > 0);
	SkImageEncoder* encoder = SkImageEncoder::Create((SkImageEncoder::Type) type);
	SkBitmap bitmap;
	maker.fCanvas->getPixels(&bitmap);
	SkString name(filename);
	if (sequence) {
		char num[4] = "000";
		num[0] = (char) (num[0] + fSeqVal / 100);
		num[1] = (char) (num[1] + fSeqVal / 10 % 10);
		num[2] = (char) (num[2] + fSeqVal % 10);
		name.append(num);
		if (++fSeqVal > 999)
			sequence = false;
	}
	if (type == SkImageEncoder::kJPEG_Type)
		name.append(".jpg");
	else if (type == SkImageEncoder::kPNG_Type)
		name.append(".png");
	encoder->encodeFile(name.c_str(), bitmap, SkScalarFloor(quality));
	return false;
}

#endif
