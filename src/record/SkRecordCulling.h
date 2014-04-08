#ifndef SkRecordCulling_DEFINED
#define SkRecordCulling_DEFINED

#include "SkRecord.h"

// Annotates PushCull records in record with the relative offset of their paired PopCull.
void SkRecordAnnotateCullingPairs(SkRecord* record);

#endif//SkRecordCulling_DEFINED
