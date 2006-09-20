#include "SkDrawable.h"

bool SkDrawable::doEvent(SkDisplayEvent::Kind , SkEventState* ) {
	return false;
}

bool SkDrawable::isDrawable() const { 
	return true; 
}

void SkDrawable::initialize() {
}

void SkDrawable::setSteps(int steps) {
}

