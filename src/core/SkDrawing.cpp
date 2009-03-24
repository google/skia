#include "SkDrawable.h"
#include "SkCanvas.h"

SkDrawable::SkDrawable() {
    fMatrix.reset();
    fParent = fFirstChild = fNextSibling = fPrevSibling = NULL;
}

SkDrawable::~SkDrawable() {
    this->detachAllChildren();
}

///////////////////////////////////////////////////////////////////////////////

void SkDrawable::resetMatrix() {
    fMatrix.reset();
}

void SkDrawable::getMatrix(SkMatrix* matrix) const {
    if (matrix) {
        *matrix = fMatrix;
    }
}

void SkDrawable::setMatrix(const SkMatrix& matrix) {
    if (fMatrix != matrix) {
        this->inval();
        fMatrix = matrix;
        this->inval();
    }
}

void SkDrawable::draw(SkCanvas* canvas) {
    SkAutoCanvasRestore ar(canvas, false);
    canvas->save(SkCanvas::kMatrix_SaveFlag);
    canvas->concat(fMatrix);

    this->onDraw(canvas);
    
    B2FIter iter(this);
    SkDrawable* child;
    while ((child = iter.next()) != NULL) {
        child->draw(canvas);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkDrawable::detachFromParent() {
	SkDrawable* parent = fParent;

	if (NULL == parent) {
		return;
    }
    
    this->inval();
    
	SkDrawable*	next = NULL;
    
	if (fNextSibling != this) {	// do we have any siblings
		fNextSibling->fPrevSibling = fPrevSibling;
		fPrevSibling->fNextSibling = fNextSibling;
		next = fNextSibling;
	}
    
	if (fParent->fFirstChild == this) {
		fParent->fFirstChild = next;
    }
    
	fParent = fNextSibling = fPrevSibling = NULL;
	this->unref();
}

SkDrawable* SkDrawable::attachChildToBack(SkDrawable* child) {
	SkASSERT(child != this);

	if (child == NULL || fFirstChild == child) {
		return child;
    }

	child->ref();
	child->detachFromParent();

	if (fFirstChild == NULL) {
		child->fNextSibling = child;
		child->fPrevSibling = child;
	} else {
		child->fNextSibling = fFirstChild;
		child->fPrevSibling = fFirstChild->fPrevSibling;
		fFirstChild->fPrevSibling->fNextSibling = child;
		fFirstChild->fPrevSibling = child;
	}

	fFirstChild = child;
	child->fParent = this;
	child->inval();
	return child;
}

SkDrawable* SkDrawable::attachChildToFront(SkDrawable* child) {
	SkASSERT(child != this);

    if (child == NULL || fFirstChild && fFirstChild->fPrevSibling == child) {
		return child;
    }

	child->ref();
	child->detachFromParent();

	if (fFirstChild == NULL) {
		fFirstChild = child;
		child->fNextSibling = child;
		child->fPrevSibling = child;
	} else {
		child->fNextSibling = fFirstChild;
		child->fPrevSibling = fFirstChild->fPrevSibling;
		fFirstChild->fPrevSibling->fNextSibling = child;
		fFirstChild->fPrevSibling = child;
	}

	child->fParent = this;
	child->inval();
	return child;
}

void SkDrawable::detachAllChildren() {
	while (fFirstChild) {
		fFirstChild->detachFromParent();
    }
}

///////////////////////////////////////////////////////////////////////////////

SkDrawable::B2FIter::B2FIter(const SkDrawable* parent) {
	fFirstChild = parent ? parent->fFirstChild : NULL;
	fChild = fFirstChild;
}

SkDrawable*	SkDrawable::B2FIter::next() {
	SkDrawable* curr = fChild;
    
	if (fChild) {
		SkDrawable* next = fChild->fNextSibling;
		if (next == fFirstChild) {
			next = NULL;
        }
		fChild = next;
	}
	return curr;
}


