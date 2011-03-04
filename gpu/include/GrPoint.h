/*
    Copyright 2010 Google Inc.

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

         http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
 */


#ifndef GrPoint_DEFINED
#define GrPoint_DEFINED

#include "GrTypes.h"
#include "GrScalar.h"

/**
 *  2D Point struct
 */
struct GrPoint {
public:
    GrScalar fX, fY;

    GrPoint() {}
    GrPoint(GrScalar x, GrScalar y) { fX = x; fY = y; }
    
    GrScalar x() const { return fX; }
    GrScalar y() const { return fY; }

    void set(GrScalar x, GrScalar y) {
        fX = x;
        fY = y;
    }
    
    void setAsMidPoint(const GrPoint& a, const GrPoint& b) {
        fX = GrScalarAve(a.fX, b.fX);
        fY = GrScalarAve(a.fY, b.fY);
    }

    void offset(GrScalar dx, GrScalar dy) {
        fX += dx;
        fY += dy;
    }

    GrScalar distanceToSqd(const GrPoint& p) const {
        GrScalar dx = (p.fX - fX);
        GrScalar dy = (p.fY - fY);
        return GrMul(dx, dx) + GrMul(dy, dy);
    }
    
    GrScalar distanceTo(const GrPoint& p) const {
        // TODO: fixed point sqrt
        return GrFloatToScalar(sqrtf(GrScalarToFloat(distanceToSqd(p))));
    }
    
    GrScalar distanceToOriginSqd() const {
        return GrMul(fX, fX) + GrMul(fY, fY);
    }

    GrScalar distanceToOrigin() const {
        return GrFloatToScalar(sqrtf(GrScalarToFloat(distanceToOriginSqd())));
    }
    
    inline GrScalar distanceToLineBetweenSqd(const GrPoint& a, 
                                             const GrPoint& b) const;

    inline GrScalar distanceToLineBetween(const GrPoint& a, 
                                          const GrPoint& b) const;
    
    inline GrScalar distanceToLineSegmentBetweenSqd(const GrPoint& a, 
                                                    const GrPoint& b) const;
    
    inline GrScalar distanceToLineSegmentBetween(const GrPoint& a, 
                                                 const GrPoint& b) const;
    
    // counter-clockwise fan
    void setRectFan(GrScalar l, GrScalar t, GrScalar r, GrScalar b) {
        GrPoint* v = this;
        v[0].set(l, t);
        v[1].set(l, b);
        v[2].set(r, b);
        v[3].set(r, t);
    }
    
    void setRectFan(GrScalar l, GrScalar t, GrScalar r, GrScalar b, size_t stride) {
        GrAssert(stride >= sizeof(GrPoint));
        ((GrPoint*)((intptr_t)this + 0 * stride))->set(l, t);
        ((GrPoint*)((intptr_t)this + 1 * stride))->set(l, b);
        ((GrPoint*)((intptr_t)this + 2 * stride))->set(r, b);
        ((GrPoint*)((intptr_t)this + 3 * stride))->set(r, t);
    }
    
    // counter-clockwise fan
    void setIRectFan(int l, int t, int r, int b) {
        GrPoint* v = this;
        v[0].set(GrIntToScalar(l), GrIntToScalar(t));
        v[1].set(GrIntToScalar(l), GrIntToScalar(b));
        v[2].set(GrIntToScalar(r), GrIntToScalar(b));
        v[3].set(GrIntToScalar(r), GrIntToScalar(t));
    }
    
    void setIRectFan(int l, int t, int r, int b, size_t stride) {
        GrAssert(stride >= sizeof(GrPoint));
        ((GrPoint*)((intptr_t)this + 0 * stride))->set(GrIntToScalar(l), 
                                                      GrIntToScalar(t));
        ((GrPoint*)((intptr_t)this + 1 * stride))->set(GrIntToScalar(l), 
                                                      GrIntToScalar(b));
        ((GrPoint*)((intptr_t)this + 2 * stride))->set(GrIntToScalar(r), 
                                                      GrIntToScalar(b));
        ((GrPoint*)((intptr_t)this + 3 * stride))->set(GrIntToScalar(r), 
                                                      GrIntToScalar(t));
    }
    
    bool operator ==(const GrPoint& p) const {
        return fX == p.fX && fY == p.fY;
    }
    
    bool operator !=(const GrPoint& p) const {
        return fX != p.fX || fY != p.fY;
    }
};

struct GrIPoint16 {
    int16_t fX, fY;
    
    void set(intptr_t x, intptr_t y) {
        fX = GrToS16(x);
        fY = GrToS16(y);
    }
};

struct GrVec {
public:
    GrScalar fX, fY;
    
    GrVec() {}
    GrVec(GrScalar x, GrScalar y) { fX = x; fY = y; }
    
    GrScalar x() const { return fX; }
    GrScalar y() const { return fY; }
    
    /**
     * set x and y length of the vector.
     */
    void set(GrScalar x, GrScalar y) {
        fX = x;
        fY = y;
    }
    
    /**
     * set vector to point from a to b.
     */
    void setBetween(const GrPoint& a, const GrPoint& b) {
        fX = b.fX - a.fX;
        fY = b.fY - a.fY;
    }
    
    /**
     * Make this vector be orthogonal to vec. Looking down vec the
     * new vector will point left.
     */
    void setOrthogLeft(const GrVec& vec) {
        // vec could be this
        GrVec v = vec;
        fX = -v.fY;
        fY = v.fX;
    }

    /**
     * Make this vector be orthogonal to vec. Looking down vec the
     * new vector will point right.
     */
    void setOrthogRight(const GrVec& vec) {
        // vec could be this
        GrVec v = vec;
        fX = v.fY;
        fY = -v.fX;
    }

    /**
     * set orthogonal to vec from a to b. Will be facing left relative to a,b
     * vec
     */
    void setOrthogLeftToVecBetween(const GrPoint& a, const GrPoint& b) {
        fX = a.fY - b.fY;
        fY = b.fX - a.fX;
    }

    /**
     * set orthogonal to vec from a to b. Will be facing right relative to a,b
     * vec.
     */
    void setOrthogRightToVecBetween(const GrPoint& a, const GrPoint& b) {
        fX = b.fY - a.fY;
        fY = a.fX - b.fX;
    }

    /**
     * length of the vector squared.
     */
    GrScalar lengthSqd() const {
        return GrMul(fX, fX) + GrMul(fY, fY);
    }
    
    /**
     * length of the vector.
     */
    GrScalar length() const {
        // TODO: fixed point sqrt
        return GrFloatToScalar(sqrtf(GrScalarToFloat(lengthSqd())));
    }
    
    /**
     * normalizes the vector if it's length is not 0.
     * @return true if normalized, otherwise false.
     */
    bool normalize() {
        GrScalar l = lengthSqd();
        if (l) {
            // TODO: fixed point sqrt and invert
            l = 1 / sqrtf(l);
            fX *= l;
            fY *= l;
            return true;
        }
        return false;
    }
    
    /**
     * Dot product of this with vec.
     */
    GrScalar dot(const GrVec& vec) const {
        return GrMul(vec.fX, fX) + GrMul(vec.fY, fY);
    }
   
    /**
     * Dot product of this vec with vector from (0,0) to a pt.
     */
    GrScalar dotWithVecToPt(const GrPoint& pt) const {
        return GrMul(pt.fX, fX) + GrMul(pt.fY, fY);
    }

    /**
     * z-value of this cross vec.
     */
    GrScalar cross(const GrVec& vec) const {
        return GrMul(fX, vec.fY) - GrMul(fY, vec.fX);
    }
    
    bool operator ==(const GrPoint& p) const {
        return fX == p.fX && fY == p.fY;
    }
    
    bool operator !=(const GrPoint& p) const {
        return fX != p.fX || fY != p.fY;
    }
};

GrScalar GrPoint::distanceToLineBetweenSqd(const GrPoint& a, 
                                           const GrPoint& b) const {
    // Let d be the distance between c (this) and line ab.
    // The area of the triangle defined by a, b, and c is 
    // A = |b-a|*d/2. Let u = b-a and v = c-a. The cross product of
    // u and v is aligned with the z axis and its magnitude is 2A. 
    // So d = |u x v| / |u|.
    GrVec u, v;
    u.setBetween(a,b);
    v.setBetween(a,*this);
    
    GrScalar det = u.cross(v);
    return (GrMul(det, det)) / u.lengthSqd();
}

GrScalar GrPoint::distanceToLineBetween(const GrPoint& a, 
                                        const GrPoint& b) const {
    GrVec u, v;
    u.setBetween(a,b);
    v.setBetween(a,*this);
    
    GrScalar det = u.cross(v);
    return (GrScalarAbs(det)) / u.length();
}

GrScalar GrPoint::distanceToLineSegmentBetweenSqd(const GrPoint& a, 
                                                  const GrPoint& b) const {
    // See comments to distanceToLineBetweenSqd. If the projection of c onto
    // u is between a and b then this returns the same result as that 
    // function. Otherwise, it returns the distance to the closer of a and
    // b. Let the projection of v onto u be v'.  There are three cases:
    //    1. v' points opposite to u. c is not between a and b and is closer
    //       to a than b.
    //    2. v' points along u and has magnitude less than y. c is between
    //       a and b and the distance to the segment is the same as distance
    //       to the line ab.
    //    3. v' points along u and has greater magnitude than u. c is not
    //       not between a and b and is closer to b than a.
    // v' = (u dot v) * u / |u|. So if (u dot v)/|u| is less than zero we're 
    // in case 1. If (u dot v)/|u| is > |u| we are in case 3. Otherwise
    // we're in case 2. We actually compare (u dot v) to 0 and |u|^2 to 
    // avoid a sqrt to compute |u|.
    
    GrVec u, v;
    u.setBetween(a,b);
    v.setBetween(a,*this);
    
    GrScalar uLengthSqd = u.lengthSqd();
    GrScalar uDotV = u.dot(v);
    
    if (uDotV <= 0) {
        return v.lengthSqd();
    } else if (uDotV > uLengthSqd) {
        return b.distanceToSqd(*this);
    } else {
        GrScalar det = u.cross(v);
        return (GrMul(det, det)) / uLengthSqd;
    }
}

GrScalar GrPoint::distanceToLineSegmentBetween(const GrPoint& a, 
                                               const GrPoint& b) const {
    // TODO: fixed point sqrt
    return GrFloatToScalar(sqrtf(GrScalarToFloat(distanceToLineSegmentBetweenSqd(a,b))));
}


#endif

