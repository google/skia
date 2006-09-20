#include "SkGlobals.h"
#include "SkThread.h"

static SkGlobals::BootStrap	gBootStrap;

SkGlobals::BootStrap& SkGlobals::GetBootStrap()
{
	return gBootStrap;
}


