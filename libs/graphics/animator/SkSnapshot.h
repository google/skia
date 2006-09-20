#ifndef SkSnapShot_DEFINED
#define SkSnapShot_DEFINED

#ifdef SK_SUPPORT_IMAGE_ENCODE

#include "SkDrawable.h"
#include "SkImageDecoder.h"
#include "SkMemberInfo.h"
#include "SkString.h"

class SkSnapshot: public SkDrawable {
	DECLARE_MEMBER_INFO(Snapshot);
	SkSnapshot();
	virtual bool draw(SkAnimateMaker& );
	private:
	SkString filename;
	SkScalar quality;
	SkBool sequence;
	int /*SkImageEncoder::Type*/	type;
	int fSeqVal;
};

#endif // SK_SUPPORT_IMAGE_ENCODE
#endif // SkSnapShot_DEFINED

