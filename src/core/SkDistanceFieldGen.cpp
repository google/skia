/*
 * Copyright 2014 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkDistanceFieldGen.h"
#include "SkPoint.h"

struct DFData {
    float   fAlpha;      // alpha value of source texel
    float   fDistSq;     // distance squared to nearest (so far) edge texel
    SkPoint fDistVector; // distance vector to nearest (so far) edge texel
};

enum NeighborFlags {
    kLeft_NeighborFlag        = 0x01,
    kRight_NeighborFlag       = 0x02,
    kTopLeft_NeighborFlag     = 0x04,
    kTop_NeighborFlag         = 0x08,
    kTopRight_NeighborFlag    = 0x10,
    kBottomLeft_NeighborFlag  = 0x20,
    kBottom_NeighborFlag      = 0x40,
    kBottomRight_NeighborFlag = 0x80,
    kAll_NeighborFlags        = 0xff,

    kNeighborFlagCount        = 8
};

// We treat an "edge" as a place where we cross from black to non-black, or vice versa.
// 'neighborFlags' is used to limit the directions in which we test to avoid indexing
// outside of the image
static bool found_edge(const unsigned char* imagePtr, int width, int neighborFlags) {
    // the order of these should match the neighbor flags above
    const int kNum8ConnectedNeighbors = 8;
    const int offsets[8] = {-1, 1, -width-1, -width, -width+1, width-1, width, width+1 };
    SkASSERT(kNum8ConnectedNeighbors == kNeighborFlagCount);

    // search for an edge
    bool currVal = (*imagePtr != 0);
    for (int i = 0; i < kNum8ConnectedNeighbors; ++i) {
        bool checkVal;
        if ((1 << i) & neighborFlags) {
            const unsigned char* checkPtr = imagePtr + offsets[i];
            checkVal = (*checkPtr != 0);
        } else {
            checkVal = false;
        }
        SkASSERT(checkVal == 0 || checkVal == 1);
        SkASSERT(currVal == 0 || currVal == 1);
        if (checkVal != currVal) {
            return true;
        }
    }

    return false;
}

static void init_glyph_data(DFData* data, unsigned char* edges, const unsigned char* image,
                            int dataWidth, int dataHeight,
                            int imageWidth, int imageHeight,
                            int pad) {
    data += pad*dataWidth;
    data += pad;
    edges += (pad*dataWidth + pad);

    for (int j = 0; j < imageHeight; ++j) {
        for (int i = 0; i < imageWidth; ++i) {
            if (255 == *image) {
                data->fAlpha = 1.0f;
            } else {
                data->fAlpha = (*image)*0.00392156862f;  // 1/255
            }
            int checkMask = kAll_NeighborFlags;
            if (i == 0) {
                checkMask &= ~(kLeft_NeighborFlag|kTopLeft_NeighborFlag|kBottomLeft_NeighborFlag);
            }
            if (i == imageWidth-1) {
                checkMask &= ~(kRight_NeighborFlag|kTopRight_NeighborFlag|kBottomRight_NeighborFlag);
            }
            if (j == 0) {
                checkMask &= ~(kTopLeft_NeighborFlag|kTop_NeighborFlag|kTopRight_NeighborFlag);
            }
            if (j == imageHeight-1) {
                checkMask &= ~(kBottomLeft_NeighborFlag|kBottom_NeighborFlag|kBottomRight_NeighborFlag);
            }
            if (found_edge(image, imageWidth, checkMask)) {
                *edges = 255;  // using 255 makes for convenient debug rendering
            }
            ++data;
            ++image;
            ++edges;
        }
        data += 2*pad;
        edges += 2*pad;
    }
}

// from Gustavson (2011)
// computes the distance to an edge given an edge normal vector and a pixel's alpha value
// assumes that direction has been pre-normalized
static float edge_distance(const SkPoint& direction, float alpha) {
    float dx = direction.fX;
    float dy = direction.fY;
    float distance;
    if (SkScalarNearlyZero(dx) || SkScalarNearlyZero(dy)) {
        distance = 0.5f - alpha;
    } else {
        // this is easier if we treat the direction as being in the first octant
        // (other octants are symmetrical)
        dx = SkScalarAbs(dx);
        dy = SkScalarAbs(dy);
        if (dx < dy) {
            SkTSwap(dx, dy);
        }

        // a1 = 0.5*dy/dx is the smaller fractional area chopped off by the edge
        // to avoid the divide, we just consider the numerator
        float a1num = 0.5f*dy;

        // we now compute the approximate distance, depending where the alpha falls
        // relative to the edge fractional area

        // if 0 <= alpha < a1
        if (alpha*dx < a1num) {
            // TODO: find a way to do this without square roots?
            distance = 0.5f*(dx + dy) - SkScalarSqrt(2.0f*dx*dy*alpha);
        // if a1 <= alpha <= 1 - a1
        } else if (alpha*dx < (dx - a1num)) {
            distance = (0.5f - alpha)*dx;
        // if 1 - a1 < alpha <= 1
        } else {
            // TODO: find a way to do this without square roots?
            distance = -0.5f*(dx + dy) + SkScalarSqrt(2.0f*dx*dy*(1.0f - alpha));
        }
    }

    return distance;
}

static void init_distances(DFData* data, unsigned char* edges, int width, int height) {
    // skip one pixel border
    DFData* currData = data;
    DFData* prevData = data - width;
    DFData* nextData = data + width;

    for (int j = 0; j < height; ++j) {
        for (int i = 0; i < width; ++i) {
            if (*edges) {
                // we should not be in the one-pixel outside band
                SkASSERT(i > 0 && i < width-1 && j > 0 && j < height-1);
                // gradient will point from low to high
                // +y is down in this case
                // i.e., if you're outside, gradient points towards edge
                // if you're inside, gradient points away from edge
                SkPoint currGrad;
                currGrad.fX = (prevData+1)->fAlpha - (prevData-1)->fAlpha
                             + SK_ScalarSqrt2*(currData+1)->fAlpha
                             - SK_ScalarSqrt2*(currData-1)->fAlpha
                             + (nextData+1)->fAlpha - (nextData-1)->fAlpha;
                currGrad.fY = (nextData-1)->fAlpha - (prevData-1)->fAlpha
                             + SK_ScalarSqrt2*nextData->fAlpha
                             - SK_ScalarSqrt2*prevData->fAlpha
                             + (nextData+1)->fAlpha - (prevData+1)->fAlpha;
                currGrad.setLengthFast(1.0f);

                // init squared distance to edge and distance vector
                float dist = edge_distance(currGrad, currData->fAlpha);
                currGrad.scale(dist, &currData->fDistVector);
                currData->fDistSq = dist*dist;
            } else {
                // init distance to "far away"
                currData->fDistSq = 2000000.f;
                currData->fDistVector.fX = 1000.f;
                currData->fDistVector.fY = 1000.f;
            }
            ++currData;
            ++prevData;
            ++nextData;
            ++edges;
        }
    }
}

// Danielsson's 8SSEDT

// first stage forward pass
// (forward in Y, forward in X)
static void F1(DFData* curr, int width) {
    // upper left
    DFData* check = curr - width-1;
    SkPoint distVec = check->fDistVector;
    float distSq = check->fDistSq - 2.0f*(distVec.fX + distVec.fY - 1.0f);
    if (distSq < curr->fDistSq) {
        distVec.fX -= 1.0f;
        distVec.fY -= 1.0f;
        curr->fDistSq = distSq;
        curr->fDistVector = distVec;
    }

    // up
    check = curr - width;
    distVec = check->fDistVector;
    distSq = check->fDistSq - 2.0f*distVec.fY + 1.0f;
    if (distSq < curr->fDistSq) {
        distVec.fY -= 1.0f;
        curr->fDistSq = distSq;
        curr->fDistVector = distVec;
    }

    // upper right
    check = curr - width+1;
    distVec = check->fDistVector;
    distSq = check->fDistSq + 2.0f*(distVec.fX - distVec.fY + 1.0f);
    if (distSq < curr->fDistSq) {
        distVec.fX += 1.0f;
        distVec.fY -= 1.0f;
        curr->fDistSq = distSq;
        curr->fDistVector = distVec;
    }

    // left
    check = curr - 1;
    distVec = check->fDistVector;
    distSq = check->fDistSq - 2.0f*distVec.fX + 1.0f;
    if (distSq < curr->fDistSq) {
        distVec.fX -= 1.0f;
        curr->fDistSq = distSq;
        curr->fDistVector = distVec;
    }
}

// second stage forward pass
// (forward in Y, backward in X)
static void F2(DFData* curr, int width) {
    // right
    DFData* check = curr + 1;
    float distSq = check->fDistSq;
    SkPoint distVec = check->fDistVector;
    distSq = check->fDistSq + 2.0f*distVec.fX + 1.0f;
    if (distSq < curr->fDistSq) {
        distVec.fX += 1.0f;
        curr->fDistSq = distSq;
        curr->fDistVector = distVec;
    }
}

// first stage backward pass
// (backward in Y, forward in X)
static void B1(DFData* curr, int width) {
    // left
    DFData* check = curr - 1;
    SkPoint distVec = check->fDistVector;
    float distSq = check->fDistSq - 2.0f*distVec.fX + 1.0f;
    if (distSq < curr->fDistSq) {
        distVec.fX -= 1.0f;
        curr->fDistSq = distSq;
        curr->fDistVector = distVec;
    }
}

// second stage backward pass
// (backward in Y, backwards in X)
static void B2(DFData* curr, int width) {
    // right
    DFData* check = curr + 1;
    SkPoint distVec = check->fDistVector;
    float distSq = check->fDistSq + 2.0f*distVec.fX + 1.0f;
    if (distSq < curr->fDistSq) {
        distVec.fX += 1.0f;
        curr->fDistSq = distSq;
        curr->fDistVector = distVec;
    }

    // bottom left
    check = curr + width-1;
    distVec = check->fDistVector;
    distSq = check->fDistSq - 2.0f*(distVec.fX - distVec.fY - 1.0f);
    if (distSq < curr->fDistSq) {
        distVec.fX -= 1.0f;
        distVec.fY += 1.0f;
        curr->fDistSq = distSq;
        curr->fDistVector = distVec;
    }

    // bottom
    check = curr + width;
    distVec = check->fDistVector;
    distSq = check->fDistSq + 2.0f*distVec.fY + 1.0f;
    if (distSq < curr->fDistSq) {
        distVec.fY += 1.0f;
        curr->fDistSq = distSq;
        curr->fDistVector = distVec;
    }

    // bottom right
    check = curr + width+1;
    distVec = check->fDistVector;
    distSq = check->fDistSq + 2.0f*(distVec.fX + distVec.fY + 1.0f);
    if (distSq < curr->fDistSq) {
        distVec.fX += 1.0f;
        distVec.fY += 1.0f;
        curr->fDistSq = distSq;
        curr->fDistVector = distVec;
    }
}

// enable this to output edge data rather than the distance field
#define DUMP_EDGE 0

#if !DUMP_EDGE
static unsigned char pack_distance_field_val(float dist, float distanceMagnitude) {
    if (dist <= -distanceMagnitude) {
        return 255;
    } else if (dist > distanceMagnitude) {
        return 0;
    } else {
        return (unsigned char)((distanceMagnitude-dist)*128.0f/distanceMagnitude);
    }
}
#endif

// assumes an 8-bit image and distance field
bool SkGenerateDistanceFieldFromImage(unsigned char* distanceField,
                                      const unsigned char* image,
                                      int width, int height,
                                      int distanceMagnitude) {
    SkASSERT(NULL != distanceField);
    SkASSERT(NULL != image);

    // the final distance field will have additional texels on each side to handle
    // the maximum distance
    // we expand our temp data by one more on each side to simplify
    // the scanning code -- will always be treated as infinitely far away
    int pad = distanceMagnitude+1;

    // set params for distance field data
    int dataWidth = width + 2*pad;
    int dataHeight = height + 2*pad;

    // create temp data
    size_t dataSize = dataWidth*dataHeight*sizeof(DFData);
    SkAutoSMalloc<1024> dfStorage(dataSize);
    DFData* dataPtr = (DFData*) dfStorage.get();
    sk_bzero(dataPtr, dataSize);

    SkAutoSMalloc<1024> edgeStorage(dataWidth*dataHeight*sizeof(char));
    unsigned char* edgePtr = (unsigned char*) edgeStorage.get();
    sk_bzero(edgePtr, dataWidth*dataHeight*sizeof(char));

    // copy glyph into distance field storage
    init_glyph_data(dataPtr, edgePtr, image,
                    dataWidth, dataHeight,
                    width, height, pad);

    // create initial distance data, particularly at edges
    init_distances(dataPtr, edgePtr, dataWidth, dataHeight);

    // now perform Euclidean distance transform to propagate distances

    // forwards in y
    DFData* currData = dataPtr+dataWidth+1; // skip outer buffer
    unsigned char* currEdge = edgePtr+dataWidth+1;
    for (int j = 1; j < dataHeight-1; ++j) {
        // forwards in x
        for (int i = 1; i < dataWidth-1; ++i) {
            // don't need to calculate distance for edge pixels
            if (!*currEdge) {
                F1(currData, dataWidth);
            }
            ++currData;
            ++currEdge;
        }

        // backwards in x
        --currData; // reset to end
        --currEdge;
        for (int i = 1; i < dataWidth-1; ++i) {
            // don't need to calculate distance for edge pixels
            if (!*currEdge) {
                F2(currData, dataWidth);
            }
            --currData;
            --currEdge;
        }

        currData += dataWidth+1;
        currEdge += dataWidth+1;
    }

    // backwards in y
    currData = dataPtr+dataWidth*(dataHeight-2) - 1; // skip outer buffer
    currEdge = edgePtr+dataWidth*(dataHeight-2) - 1;
    for (int j = 1; j < dataHeight-1; ++j) {
        // forwards in x
        for (int i = 1; i < dataWidth-1; ++i) {
            // don't need to calculate distance for edge pixels
            if (!*currEdge) {
                B1(currData, dataWidth);
            }
            ++currData;
            ++currEdge;
        }

        // backwards in x
        --currData; // reset to end
        --currEdge;
        for (int i = 1; i < dataWidth-1; ++i) {
            // don't need to calculate distance for edge pixels
            if (!*currEdge) {
                B2(currData, dataWidth);
            }
            --currData;
            --currEdge;
        }

        currData -= dataWidth-1;
        currEdge -= dataWidth-1;
    }

    // copy results to final distance field data
    currData = dataPtr + dataWidth+1;
    currEdge = edgePtr + dataWidth+1;
    unsigned char *dfPtr = distanceField;
    for (int j = 1; j < dataHeight-1; ++j) {
        for (int i = 1; i < dataWidth-1; ++i) {
#if DUMP_EDGE
            unsigned char val = sk_float_round2int(255*currData->fAlpha);
            if (*currEdge) {
                val = 128;
            }
            *dfPtr++ = val;
#else
            float dist;
            if (currData->fAlpha > 0.5f) {
                dist = -SkScalarSqrt(currData->fDistSq);
            } else {
                dist = SkScalarSqrt(currData->fDistSq);
            }
            *dfPtr++ = pack_distance_field_val(dist, (float)distanceMagnitude);
#endif
            ++currData;
            ++currEdge;
        }
        currData += 2;
        currEdge += 2;
    }

    return true;
}
