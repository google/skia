#ifndef SkDrawable_DEFINED
#define SkDrawable_DEFINED

#include "SkDisplayable.h"
#include "SkDisplayEvent.h"
#include "SkMath.h"

struct SkEventState;

class SkDrawable :  public SkDisplayable {
public:
	virtual bool doEvent(SkDisplayEvent::Kind , SkEventState* state );
	virtual bool draw(SkAnimateMaker& ) = 0; 
	virtual void initialize();
	virtual bool isDrawable() const;
	virtual void setSteps(int steps);
};

#endif // SkDrawable_DEFINED
