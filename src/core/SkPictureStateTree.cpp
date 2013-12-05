
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkPictureStateTree.h"
#include "SkCanvas.h"

SkPictureStateTree::SkPictureStateTree()
    : fAlloc(2048)
    , fRoot(NULL)
    , fLastRestoredNode(NULL)
    , fStateStack(sizeof(Draw), 16) {
    SkMatrix* identity = static_cast<SkMatrix*>(fAlloc.allocThrow(sizeof(SkMatrix)));
    identity->reset();
    fRoot = static_cast<Node*>(fAlloc.allocThrow(sizeof(Node)));
    fRoot->fParent = NULL;
    fRoot->fMatrix = identity;
    fRoot->fFlags = Node::kSave_Flag;
    fRoot->fOffset = 0;
    fRoot->fLevel = 0;
    fCurrentState.fNode = fRoot;
    fCurrentState.fMatrix = identity;
    *static_cast<Draw*>(fStateStack.push_back()) = fCurrentState;
}

SkPictureStateTree::~SkPictureStateTree() {
}

SkPictureStateTree::Draw* SkPictureStateTree::appendDraw(uint32_t offset) {
    Draw* draw = static_cast<Draw*>(fAlloc.allocThrow(sizeof(Draw)));
    *draw = fCurrentState;
    draw->fOffset = offset;
    return draw;
}

void SkPictureStateTree::appendSave() {
    *static_cast<Draw*>(fStateStack.push_back()) = fCurrentState;
    fCurrentState.fNode->fFlags |= Node::kSave_Flag;
}

void SkPictureStateTree::appendSaveLayer(uint32_t offset) {
    *static_cast<Draw*>(fStateStack.push_back()) = fCurrentState;
    this->appendNode(offset);
    fCurrentState.fNode->fFlags |= Node::kSaveLayer_Flag;
}

void SkPictureStateTree::saveCollapsed() {
    SkASSERT(NULL != fLastRestoredNode);
    SkASSERT(SkToBool(fLastRestoredNode->fFlags & \
        (Node::kSaveLayer_Flag | Node::kSave_Flag)));
    SkASSERT(fLastRestoredNode->fParent == fCurrentState.fNode);
    // The structure of the tree is not modified here. We just turn off
    // the save or saveLayer flag to prevent the iterator from making state
    // changing calls on the playback canvas when traversing a save or
    // saveLayerNode node.
    fLastRestoredNode->fFlags = 0;
}

void SkPictureStateTree::appendRestore() {
    fLastRestoredNode = fCurrentState.fNode;
    fCurrentState = *static_cast<Draw*>(fStateStack.back());
    fStateStack.pop_back();
}

void SkPictureStateTree::appendTransform(const SkMatrix& trans) {
    SkMatrix* m = static_cast<SkMatrix*>(fAlloc.allocThrow(sizeof(SkMatrix)));
    *m = trans;
    fCurrentState.fMatrix = m;
}

void SkPictureStateTree::appendClip(uint32_t offset) {
    this->appendNode(offset);
}

SkPictureStateTree::Iterator SkPictureStateTree::getIterator(const SkTDArray<void*>& draws,
                                                             SkCanvas* canvas) {
    return Iterator(draws, canvas, fRoot);
}

void SkPictureStateTree::appendNode(uint32_t offset) {
    Node* n = static_cast<Node*>(fAlloc.allocThrow(sizeof(Node)));
    n->fOffset = offset;
    n->fFlags = 0;
    n->fParent = fCurrentState.fNode;
    n->fLevel = fCurrentState.fNode->fLevel + 1;
    n->fMatrix = fCurrentState.fMatrix;
    fCurrentState.fNode = n;
}

SkPictureStateTree::Iterator::Iterator(const SkTDArray<void*>& draws, SkCanvas* canvas, Node* root)
    : fDraws(&draws)
    , fCanvas(canvas)
    , fCurrentNode(root)
    , fPlaybackMatrix(canvas->getTotalMatrix())
    , fCurrentMatrix(NULL)
    , fPlaybackIndex(0)
    , fSave(false)
    , fValid(true) {
}

uint32_t SkPictureStateTree::Iterator::draw() {
    SkASSERT(this->isValid());
    if (fPlaybackIndex >= fDraws->count()) {
        // restore back to where we started
        if (fCurrentNode->fFlags & Node::kSaveLayer_Flag) { fCanvas->restore(); }
        fCurrentNode = fCurrentNode->fParent;
        while (NULL != fCurrentNode) {
            if (fCurrentNode->fFlags & Node::kSave_Flag) { fCanvas->restore(); }
            if (fCurrentNode->fFlags & Node::kSaveLayer_Flag) { fCanvas->restore(); }
            fCurrentNode = fCurrentNode->fParent;
        }
        fCanvas->setMatrix(fPlaybackMatrix);
        return kDrawComplete;
    }

    Draw* draw = static_cast<Draw*>((*fDraws)[fPlaybackIndex]);
    Node* targetNode = draw->fNode;

    if (fSave) {
        fCanvas->save(SkCanvas::kClip_SaveFlag);
        fSave = false;
    }

    if (fCurrentNode != targetNode) {
        // If we're not at the target and we don't have a list of nodes to get there, we need to
        // figure out the path from our current node, to the target
        if (fNodes.count() == 0) {
            // Trace back up to a common ancestor, restoring to get our current state to match that
            // of the ancestor, and saving a list of nodes whose state we need to apply to get to
            // the target (we can restore up to the ancestor immediately, but we'll need to return
            // an offset for each node on the way down to the target, to apply the desired clips and
            // saveLayers, so it may take several draw() calls before the next draw actually occurs)
            Node* tmp = fCurrentNode;
            Node* ancestor = targetNode;
            while (tmp != ancestor) {
                uint16_t currentLevel = tmp->fLevel;
                uint16_t targetLevel = ancestor->fLevel;
                if (currentLevel >= targetLevel) {
                    if (tmp != fCurrentNode && tmp->fFlags & Node::kSave_Flag) { fCanvas->restore(); }
                    if (tmp->fFlags & Node::kSaveLayer_Flag) { fCanvas->restore(); }
                    tmp = tmp->fParent;
                }
                if (currentLevel <= targetLevel) {
                    fNodes.push(ancestor);
                    ancestor = ancestor->fParent;
                }
            }

            if (ancestor->fFlags & Node::kSave_Flag) {
                if (fCurrentNode != ancestor) { fCanvas->restore(); }
                if (targetNode != ancestor) { fCanvas->save(SkCanvas::kClip_SaveFlag); }
            }
            fCurrentNode = ancestor;
        }

        // If we're not at the target node yet, we'll need to return an offset to make the caller
        // apply the next clip or saveLayer.
        if (fCurrentNode != targetNode) {
            if (fCurrentMatrix != fNodes.top()->fMatrix) {
                fCurrentMatrix = fNodes.top()->fMatrix;
                SkMatrix tmp = *fNodes.top()->fMatrix;
                tmp.postConcat(fPlaybackMatrix);
                fCanvas->setMatrix(tmp);
            }
            uint32_t offset = fNodes.top()->fOffset;
            fCurrentNode = fNodes.top();
            fSave = fCurrentNode != targetNode && fCurrentNode->fFlags & Node::kSave_Flag;
            fNodes.pop();
            return offset;
        }
    }

    // If we got this far, the clip/saveLayer state is all set, so we can proceed to set the matrix
    // for the draw, and return its offset.

    if (fCurrentMatrix != draw->fMatrix) {
        SkMatrix tmp = *draw->fMatrix;
        tmp.postConcat(fPlaybackMatrix);
        fCanvas->setMatrix(tmp);
        fCurrentMatrix = draw->fMatrix;
    }

    ++fPlaybackIndex;
    return draw->fOffset;
}
