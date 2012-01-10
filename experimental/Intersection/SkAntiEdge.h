/*
 *  SkAntiEdge.h
 *  core
 *
 *  Created by Cary Clark on 5/6/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef SkAntiEdge_DEFINED
#define SkAntiEdge_DEFINED

#include "SkFixed.h"
#include "SkTDArray.h"

struct SkBitmap;
struct SkPoint;

struct SkAntiEdge {
    SkAntiEdge* fNext; // list in walking order (y, then x, then diag)
    SkAntiEdge* fPrev; // reverse in walking order
    SkAntiEdge* fLink; // list in connected order, top to bottom

    SkFixed     fFirstX; // starting X
    SkFixed     fFirstY; // starting Y
    SkFixed     fLastX; // ending X
    SkFixed     fLastY; // ending Y
    SkFixed     fX0; // computed edge current value (may be off end)
    SkFixed     fY0;
    SkFixed     fX; // edge current value (always on edge)
    SkFixed     fY;
    SkFixed     fDX; // change in X per unit step in Y
    SkFixed     fDY; // change in Y per unit step in X
    SkFixed     fWalkX; // unit step position (integer after initial step)
    SkFixed     fWalkY;
    uint16_t    fPartialY; // initial partial coverage in Y (0 .. SkFixed1]
    int16_t     fWindingSum; // winding including contributions to the left
    int8_t      fWinding; // 1 or -1 (could be 2 bits)
    bool        fFinished : 1;
    unsigned    fDXFlipped : 1; // used as bool and to adjust calculations (0/1)
    bool        fLinkSet : 1; // set if edge has been attached to another edge

    void calcLine();
    bool setLine(const SkPoint& p0, const SkPoint& p1);
    uint16_t advanceX(SkFixed left);
    uint16_t advanceFlippedX(SkFixed left);
    void advanceY(SkFixed top);
// FIXME: mark DEBUG
    void pointInLine(SkFixed x, SkFixed y);
    void pointOnLine(SkFixed x, SkFixed y);
    void validate();
};

class SkAntiEdgeBuilder {
public:
void process(const SkPoint* points, int ptCount,
        uint8_t* result, int pixelCol, int pixelRow);
private:
    int build(const SkPoint pts[], int count);
    void calc();
    void link();
    void sort();
    void sort(SkTDArray<SkAntiEdge*>&);
    void split();
    void split(SkAntiEdge* edge, SkFixed y);
    void walk(uint8_t* result, int rowBytes, int height);
    SkAntiEdge fHeadEdge;
    SkAntiEdge fTailEdge;
    SkTDArray<SkAntiEdge> fEdges;
    SkTDArray<SkAntiEdge*> fList;
};

void SkAntiEdge_Test();
void CreateSweep(SkBitmap* , float width);
void CreateHorz(SkBitmap* );
void CreateVert(SkBitmap* );
void CreateAngle(SkBitmap* sweep, float angle);

#endif
