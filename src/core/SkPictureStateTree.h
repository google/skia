
/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkPictureStateTree_DEFINED
#define SkPictureStateTree_DEFINED

#include "SkTDArray.h"
#include "SkChunkAlloc.h"
#include "SkDeque.h"
#include "SkMatrix.h"
#include "SkRefCnt.h"

class SkCanvas;

/**
 * Provides an interface that, given a sequence of draws into an SkPicture with corresponding
 * offsets, allows for playback of an arbitrary subset of the draws (note that Z-order is only
 * guaranteed if the draws are explicitly sorted).
 */
class SkPictureStateTree : public SkRefCnt {
private:
    struct Node;
public:
    SK_DECLARE_INST_COUNT(SkPictureStateTree)

    /**
     * A draw call, stores offset into command buffer, a pointer to the matrix, and a pointer to
     * the node in the tree that corresponds to its clip/layer state
     */
    struct Draw {
        SkMatrix* fMatrix;
        Node* fNode;
        uint32_t fOffset;
        bool operator<(const Draw& other) const { return fOffset < other.fOffset; }
    };

    class Iterator;

    SkPictureStateTree();
    ~SkPictureStateTree();

    /**
     * Creates and returns a struct representing a draw at the given offset.
     */
    Draw* appendDraw(size_t offset);

    /**
     * Given a list of draws, and a canvas, initialize an iterator that produces the correct 
     * sequence of offsets into the command buffer to carry out those calls with correct 
     * matrix/clip state. This handles saves/restores, and does all necessary matrix setup.
     */
    void initIterator(SkPictureStateTree::Iterator* iter, 
                      const SkTDArray<void*>& draws, 
                      SkCanvas* canvas);

    void appendSave();
    void appendSaveLayer(size_t offset);
    void appendRestore();
    void appendTransform(const SkMatrix& trans);
    void appendClip(size_t offset);

    /**
     * Call this immediately after an appendRestore call that is associated
     * a save or saveLayer that was removed from the command stream
     * due to a command pattern optimization in SkPicture.
     */
    void saveCollapsed();

    /**
     * Playback helper
     */
    class Iterator {
    public:
        /** Returns the next op offset needed to create the drawing state
            required by the queued up draw operation or the offset of the queued
            up draw operation itself. In the latter case, the next draw operation
            will move into the queued up slot.
            It retuns kDrawComplete when done.
            TODO: this might be better named nextOp
        */
        uint32_t nextDraw();
        static const uint32_t kDrawComplete = SK_MaxU32;
        Iterator() : fValid(false) { }
        bool isValid() const { return fValid; }

    private:
        void init(const SkTDArray<void*>& draws, SkCanvas* canvas, Node* root);

        void setCurrentMatrix(const SkMatrix*);

        // The draws this iterator is associated with
        const SkTDArray<void*>* fDraws;

        // canvas this is playing into (so we can insert saves/restores as necessary)
        SkCanvas* fCanvas;

        // current state node
        Node* fCurrentNode;

        // List of nodes whose state we need to apply to reach TargetNode
        SkTDArray<Node*> fNodes;

        // The matrix of the canvas we're playing back into
        SkMatrix fPlaybackMatrix;

        // Cache of current matrix, so we can avoid redundantly setting it
        const SkMatrix* fCurrentMatrix;

        // current position in the array of draws
        int fPlaybackIndex;
        // Whether or not we need to do a save next iteration
        bool fSave;

        // Whether or not this is a valid iterator (the default public constructor sets this false)
        bool fValid;

        uint32_t finish();

        friend class SkPictureStateTree;
    };

private:

    void appendNode(size_t offset);

    SkChunkAlloc fAlloc;
    // Needed by saveCollapsed() because nodes do not currently store
    // references to their children.  If they did, we could just retrieve the
    // last added child.
    Node* fLastRestoredNode;

    // The currently active state
    Draw fCurrentState;
    // A stack of states for tracking save/restores
    SkDeque fStateStack;

    // Represents a notable piece of state that requires an offset into the command buffer,
    // corresponding to a clip/saveLayer/etc call, to apply.
    struct Node {
        Node* fParent;
        uint32_t fOffset;
        uint16_t fLevel;
        uint16_t fFlags;
        SkMatrix* fMatrix;
        enum Flags {
            kSave_Flag      = 0x1,
            kSaveLayer_Flag = 0x2
        };
    };

    Node fRoot;
    SkMatrix fRootMatrix;

    typedef SkRefCnt INHERITED;
};

#endif
