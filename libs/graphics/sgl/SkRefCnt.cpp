#include "SkRefCnt.h"

SkAutoUnref::~SkAutoUnref()
{
	if (fObj)
		fObj->unref();
}

bool SkAutoUnref::ref()
{
	if (fObj)
	{
		fObj->ref();
		return true;
	}
	return false;
}

bool SkAutoUnref::unref()
{
	if (fObj)
	{
		fObj->unref();
		fObj = nil;
		return true;
	}
	return false;
}

SkRefCnt* SkAutoUnref::detach()
{
	SkRefCnt* obj = fObj;

	fObj = nil;
	return obj;
}
