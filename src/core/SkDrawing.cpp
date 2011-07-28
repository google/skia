
/*
 * Copyright 2011 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
#include "SkDrawing.h"
#include "SkCanvas.h"

SkDrawing::SkDrawing() {
    fMatrix.reset();
    fParent = fFirstChild = fNextSibling = fPrevSibling = NULL;
}

SkDrawing::~SkDrawing() {
    this->detachAllChildren();
}

///////////////////////////////////////////////////////////////////////////////

void SkDrawing::resetMatrix() {
    fMatrix.reset();
}

void SkDrawing::getMatrix(SkMatrix* matrix) const {
    if (matrix) {
        *matrix = fMatrix;
    }
}

void SkDrawing::setMatrix(const SkMatrix& matrix) {
    if (fMatrix != matrix) {
        this->inval();
        fMatrix = matrix;
        this->inval();
    }
}

void SkDrawing::draw(SkCanvas* canvas) {
    SkAutoCanvasRestore ar(canvas, false);
    canvas->save(SkCanvas::kMatrix_SaveFlag);
    canvas->concat(fMatrix);

    this->onDraw(canvas);
    
    B2FIter iter(this);
    SkDrawing* child;
    while ((child = iter.next()) != NULL) {
        child->draw(canvas);
    }
}

///////////////////////////////////////////////////////////////////////////////

void SkDrawing::detachFromParent() {
	SkDrawing* parent = fParent;

	if (NULL == parent) {
		return;
    }
    
    this->inval();
    
	SkDrawing*	next = NULL;
    
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

SkDrawing* SkDrawing::attachChildToBack(SkDrawing* child) {
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

SkDrawing* SkDrawing::attachChildToFront(SkDrawing* child) {
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

void SkDrawing::detachAllChildren() {
	while (fFirstChild) {
		fFirstChild->detachFromParent();
    }
}

///////////////////////////////////////////////////////////////////////////////

SkDrawing::B2FIter::B2FIter(const SkDrawing* parent) {
	fFirstChild = parent ? parent->fFirstChild : NULL;
	fChild = fFirstChild;
}

SkDrawing*	SkDrawing::B2FIter::next() {
	SkDrawing* curr = fChild;
    
	if (fChild) {
		SkDrawing* next = fChild->fNextSibling;
		if (next == fFirstChild) {
			next = NULL;
        }
		fChild = next;
	}
	return curr;
}


