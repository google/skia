
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
        bool operator<(const Draw& other) { return fOffset < other.fOffset; }
    };

    class Iterator;

    SkPictureStateTree();
    ~SkPictureStateTree();

    /**
     * Creates and returns a struct representing a draw at the given offset.
     */
    Draw* appendDraw(uint32_t offset);

    /**
     * Given a list of draws, and a canvas, returns an iterator that produces the correct sequence
     * of offsets into the command buffer to carry out those calls with correct matrix/clip state.
     * This handles saves/restores, and does all necessary matrix setup.
     */
    Iterator getIterator(const SkTDArray<void*>& draws, SkCanvas* canvas);

    void appendSave();
    void appendSaveLayer(uint32_t offset);
    void appendRestore();
    void appendTransform(const SkMatrix& trans);
    void appendClip(uint32_t offset);

    /**
     * Playback helper
     */
    class Iterator {
    public:
        /** Returns the next offset into the picture stream, or kDrawComplete if complete. */
        uint32_t draw();
        static const uint32_t kDrawComplete = SK_MaxU32;
        Iterator() : fPlaybackMatrix(), fValid(false) { }
        bool isValid() { return fValid; }
    private:
        Iterator(const SkTDArray<void*>& draws, SkCanvas* canvas, Node* root);
        // The draws this iterator is associated with
        const SkTDArray<void*>* fDraws;

        // canvas this is playing into (so we can insert saves/restores as necessary)
        SkCanvas* fCanvas;

        // current state node
        Node* fCurrentNode;

        // List of nodes whose state we need to apply to reach TargetNode
        SkTDArray<Node*> fNodes;

        // The matrix of the canvas we're playing back into
        const SkMatrix fPlaybackMatrix;

        // Cache of current matrix, so we can avoid redundantly setting it
        SkMatrix* fCurrentMatrix;

        // current position in the array of draws
        int fPlaybackIndex;
        // Whether or not we need to do a save next iteration
        bool fSave;

        // Whether or not this is a valid iterator (the default public constructor sets this false)
        bool fValid;

        friend class SkPictureStateTree;
    };

private:

    void appendNode(uint32_t offset);

    SkChunkAlloc fAlloc;
    Node* fRoot;

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

    typedef SkRefCnt INHERITED;
};

#endif

