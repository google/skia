/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SkMatrix_DEFINED
#define SkMatrix_DEFINED

#include "SkRect.h"

struct SkRSXform;
struct SkPoint3;
class SkString;

/** \class SkMatrix
    SkMatrix holds a 3x3 matrix for transforming coordinates. This allows mapping
    points and vectors with translation, scaling, skewing, rotation, and
    perspective.

    SkMatrix elements are in row major order. SkMatrix does not have a constructor,
    so it must be explicitly initialized. setIdentity() initializes SkMatrix
    so it has no effect. setTranslate(), setScale(), setSkew(), setRotate(), set9 and setAll()
    initializes all SkMatrix elements with the corresponding mapping.

    SkMatrix includes a hidden variable that classifies the type of matrix to
    improve performance. SkMatrix is not thread safe unless getType() is called first.
*/
SK_BEGIN_REQUIRE_DENSE
class SK_API SkMatrix {
public:

    /** Sets SkMatrix to scale by (sx, sy). Returned matrix is:

            | sx  0  0 |
            |  0 sy  0 |
            |  0  0  1 |

        @param sx  horizontal scale factor
        @param sy  vertical scale factor
        @return    SkMatrix with scale
    */
    static SkMatrix SK_WARN_UNUSED_RESULT MakeScale(SkScalar sx, SkScalar sy) {
        SkMatrix m;
        m.setScale(sx, sy);
        return m;
    }

    /** Sets SkMatrix to scale by (scale, scale). Returned matrix is:

            | scale   0   0 |
            |   0   scale 0 |
            |   0     0   1 |

        @param scale  horizontal and vertical scale factor
        @return       SkMatrix with scale
    */
    static SkMatrix SK_WARN_UNUSED_RESULT MakeScale(SkScalar scale) {
        SkMatrix m;
        m.setScale(scale, scale);
        return m;
    }

    /** Sets SkMatrix to translate by (dx, dy). Returned matrix is:

            | 1 0 dx |
            | 0 1 dy |
            | 0 0  1 |

        @param dx  horizontal translation
        @param dy  vertical translation
        @return    SkMatrix with translation
    */
    static SkMatrix SK_WARN_UNUSED_RESULT MakeTrans(SkScalar dx, SkScalar dy) {
        SkMatrix m;
        m.setTranslate(dx, dy);
        return m;
    }

    /** Sets SkMatrix to:

            | scaleX  skewX transX |
            |  skewY scaleY transY |
            |  pers0  pers1  pers2 |

        @param scaleX  horizontal scale factor
        @param skewX   horizontal skew factor
        @param transX  horizontal translation
        @param skewY   vertical skew factor
        @param scaleY  vertical scale factor
        @param transY  vertical translation
        @param pers0   input x perspective factor
        @param pers1   input y perspective factor
        @param pers2   perspective scale factor
        @return        SkMatrix constructed from parameters
    */
    static SkMatrix SK_WARN_UNUSED_RESULT MakeAll(SkScalar scaleX, SkScalar skewX,  SkScalar transX,
                                                  SkScalar skewY,  SkScalar scaleY, SkScalar transY,
                                                  SkScalar pers0, SkScalar pers1, SkScalar pers2) {
        SkMatrix m;
        m.setAll(scaleX, skewX, transX, skewY, scaleY, transY, pers0, pers1, pers2);
        return m;
    }

    /** \enum SkMatrix::TypeMask
        Enum of bit fields for mask returned by getType().
        Used to identify the complexity of SkMatrix, to optimize performance.
    */
    enum TypeMask {
        kIdentity_Mask    = 0,    //!< all bits clear if SkMatrix is identity
        kTranslate_Mask   = 0x01, //!< set if SkMatrix has translation
        kScale_Mask       = 0x02, //!< set if SkMatrix has x or y scale
        kAffine_Mask      = 0x04, //!< set if SkMatrix skews or rotates
        kPerspective_Mask = 0x08, //!< set if SkMatrix has perspective
    };

    /** Returns a bit field describing the transformations the matrix may
        perform. The bit field is computed conservatively, so it may include
        false positives. For example, when kPerspective_Mask is set, all
        other bits are set.

        @return  kIdentity_Mask, or combinations of: kTranslate_Mask, kScale_Mask,
                 kAffine_Mask, kPerspective_Mask
    */
    TypeMask getType() const {
        if (fTypeMask & kUnknown_Mask) {
            fTypeMask = this->computeTypeMask();
        }
        // only return the public masks
        return (TypeMask)(fTypeMask & 0xF);
    }

    /** Returns true if SkMatrix is identity.  Identity matrix is:

            | 1 0 0 |
            | 0 1 0 |
            | 0 0 1 |

        @return  true if SkMatrix has no effect
    */
    bool isIdentity() const {
        return this->getType() == 0;
    }

    /** Returns true if SkMatrix at most scales and translates. SkMatrix may be identity,
        contain only scale elements, only translate elements, or both. SkMatrix form is:

            | scale-x    0    translate-x |
            |    0    scale-y translate-y |
            |    0       0         1      |

        @return  true if SkMatrix is identity; or scales, translates, or both
    */
    bool isScaleTranslate() const {
        return !(this->getType() & ~(kScale_Mask | kTranslate_Mask));
    }

    /** Returns true if SkMatrix is identity, or translates. SkMatrix form is:

            | 1 0 translate-x |
            | 0 1 translate-y |
            | 0 0      1      |

        @return  true if SkMatrix is identity, or translates
    */
    bool isTranslate() const { return !(this->getType() & ~(kTranslate_Mask)); }

    /** Returns true SkMatrix maps SkRect to another SkRect. If true, SkMatrix is identity,
        or scales, or rotates a multiple of 90 degrees, or mirrors in x or y. In all
        cases, SkMatrix may also have translation. SkMatrix form is either:

            | scale-x    0    translate-x |
            |    0    scale-y translate-y |
            |    0       0         1      |

        or

            |    0     rotate-x translate-x |
            | rotate-y    0     translate-y |
            |    0        0          1      |

        for non-zero values of scale-x, scale-y, rotate-x, and rotate-y.

        Also called preservesAxisAlignment(); use the one that provides better inline
        documentation.

        @return  true if SkMatrix maps one SkRect into another
    */
    bool rectStaysRect() const {
        if (fTypeMask & kUnknown_Mask) {
            fTypeMask = this->computeTypeMask();
        }
        return (fTypeMask & kRectStaysRect_Mask) != 0;
    }

    /** Returns true SkMatrix maps SkRect to another SkRect. If true, SkMatrix is identity,
        or scales, or rotates a multiple of 90 degrees, or mirrors in x or y. In all
        cases, SkMatrix may also have translation. SkMatrix form is either:

            | scale-x    0    translate-x |
            |    0    scale-y translate-y |
            |    0       0         1      |

        or

            |    0     rotate-x translate-x |
            | rotate-y    0     translate-y |
            |    0        0          1      |

        for non-zero values of scale-x, scale-y, rotate-x, and rotate-y.

        Also called rectStaysRect(); use the one that provides better inline
        documentation.

        @return  true if SkMatrix maps one SkRect into another
    */
    bool preservesAxisAlignment() const { return this->rectStaysRect(); }

    /** Returns true if the matrix contains perspective elements. SkMatrix form is:

            |       --            --              --          |
            |       --            --              --          |
            | perspective-x  perspective-y  perspective-scale |

        where perspective-x or perspective-y is non-zero, or perspective-scale is
        not one. All other elements may have any value.

        @return  true if SkMatrix is in most general form
    */
    bool hasPerspective() const {
        return SkToBool(this->getPerspectiveTypeMaskOnly() &
                        kPerspective_Mask);
    }

    /** Returns true if SkMatrix contains only translation, rotation, reflection, and
        uniform scale.
        Returns false if SkMatrix contains different scales, skewing, perspective, or
        degenerate forms that collapse to a line or point.

        Describes that the SkMatrix makes rendering with and without the matrix are
        visually alike; a transformed circle remains a circle. Mathematically, this is
        referred to as similarity of a Euclidean_Space, or a similarity transformation.

        Preserves right angles, keeping the arms of the angle equal lengths.

        @param tol  to be deprecated
        @return     true if SkMatrix only rotates, uniformly scales, translates
    */
    bool isSimilarity(SkScalar tol = SK_ScalarNearlyZero) const;

    /** Returns true if SkMatrix contains only translation, rotation, reflection, and
        scale. Scale may differ along rotated axes.
        Returns false if SkMatrix skewing, perspective, or degenerate forms that collapse
        to a line or point.

        Preserves right angles, but not requiring that the arms of the angle
        retain equal lengths.

        @param tol  to be deprecated
        @return     true if SkMatrix only rotates, scales, translates
    */
    bool preservesRightAngles(SkScalar tol = SK_ScalarNearlyZero) const;

    /** \enum
        SkMatrix organizes its values in row order. These members correspond to
        each value in SkMatrix.
    */
    enum {
        kMScaleX, //!< horizontal scale factor
        kMSkewX,  //!< horizontal skew factor
        kMTransX, //!< horizontal translation
        kMSkewY,  //!< vertical skew factor
        kMScaleY, //!< vertical scale factor
        kMTransY, //!< vertical translation
        kMPersp0, //!< input x perspective factor
        kMPersp1, //!< input y perspective factor
        kMPersp2, //!< perspective bias
    };

    /** \enum
        Affine arrays are in column major order to match the matrix used by
        PDF and XPS.
    */
    enum {
        kAScaleX, //!< horizontal scale factor
        kASkewY,  //!< vertical skew factor
        kASkewX,  //!< horizontal skew factor
        kAScaleY, //!< vertical scale factor
        kATransX, //!< horizontal translation
        kATransY, //!< vertical translation
    };

    /** Returns one matrix value. Asserts if index is out of range and SK_DEBUG is
        defined.

        @param index  one of: kMScaleX, kMSkewX, kMTransX, kMSkewY, kMScaleY, kMTransY,
                      kMPersp0, kMPersp1, kMPersp2
        @return       value corresponding to index
    */
    SkScalar operator[](int index) const {
        SkASSERT((unsigned)index < 9);
        return fMat[index];
    }

    /** Returns one matrix value. Asserts if index is out of range and SK_DEBUG is
        defined.

        @param index  one of: kMScaleX, kMSkewX, kMTransX, kMSkewY, kMScaleY, kMTransY,
                      kMPersp0, kMPersp1, kMPersp2
        @return       value corresponding to index
    */
    SkScalar get(int index) const {
        SkASSERT((unsigned)index < 9);
        return fMat[index];
    }

    /** Returns scale factor multiplied by x input, contributing to x output.
        With mapPoints(), scales points along the x-axis.

        @return  horizontal scale factor
    */
    SkScalar getScaleX() const { return fMat[kMScaleX]; }

    /** Returns scale factor multiplied by y input, contributing to y output.
        With mapPoints(), scales points along the y-axis.

        @return  vertical scale factor
    */
    SkScalar getScaleY() const { return fMat[kMScaleY]; }

    /** Returns scale factor multiplied by x input, contributing to y output.
        With mapPoints(), skews points along the y-axis.
        Skew x and y together can rotate points.

        @return  vertical skew factor
    */
    SkScalar getSkewY() const { return fMat[kMSkewY]; }

    /** Returns scale factor multiplied by y input, contributing to x output.
        With mapPoints(), skews points along the x-axis.
        Skew x and y together can rotate points.

        @return  horizontal scale factor
    */
    SkScalar getSkewX() const { return fMat[kMSkewX]; }

    /** Returns translation contributing to x output.
        With mapPoints(), moves points along the x-axis.

        @return  horizontal translation factor
    */
    SkScalar getTranslateX() const { return fMat[kMTransX]; }

    /** Returns translation contributing to y output.
        With mapPoints(), moves points along the y-axis.

        @return  vertical translation factor
    */
    SkScalar getTranslateY() const { return fMat[kMTransY]; }

    /** Returns factor scaling input x relative to input y.

        @return  input x perspective factor
    */
    SkScalar getPerspX() const { return fMat[kMPersp0]; }

    /** Returns factor scaling input y relative to input x.

        @return  input y perspective factor
    */
    SkScalar getPerspY() const { return fMat[kMPersp1]; }

    /** Returns writable SkMatrix value. Asserts if index is out of range and SK_DEBUG is
        defined. Clears internal cache anticipating that caller will change SkMatrix value.

        Next call to read SkMatrix state may recompute cache; subsequent writes to SkMatrix
        value must be followed by dirtyMatrixTypeCache().

        @param index  one of: kMScaleX, kMSkewX, kMTransX, kMSkewY, kMScaleY, kMTransY,
                      kMPersp0, kMPersp1, kMPersp2
        @return       writable value corresponding to index
    */
    SkScalar& operator[](int index) {
        SkASSERT((unsigned)index < 9);
        this->setTypeMask(kUnknown_Mask);
        return fMat[index];
    }

    /** Sets SkMatrix value. Asserts if index is out of range and SK_DEBUG is
        defined. Safer than operator[]; internal cache is always maintained.

        @param index  one of: kMScaleX, kMSkewX, kMTransX, kMSkewY, kMScaleY, kMTransY,
                      kMPersp0, kMPersp1, kMPersp2
        @param value  scalar to store in SkMatrix
    */
    void set(int index, SkScalar value) {
        SkASSERT((unsigned)index < 9);
        fMat[index] = value;
        this->setTypeMask(kUnknown_Mask);
    }

    /** Sets horizontal scale factor.

        @param v  horizontal scale factor to store
    */
    void setScaleX(SkScalar v) { this->set(kMScaleX, v); }

    /** Sets vertical scale factor.

        @param v  vertical scale factor to store
    */
    void setScaleY(SkScalar v) { this->set(kMScaleY, v); }

    /** Sets vertical skew factor.

        @param v  vertical skew factor to store
    */
    void setSkewY(SkScalar v) { this->set(kMSkewY, v); }

    /** Sets horizontal skew factor.

        @param v  horizontal skew factor to store
    */
    void setSkewX(SkScalar v) { this->set(kMSkewX, v); }

    /** Sets horizontal translation.

        @param v  horizontal translation to store
    */
    void setTranslateX(SkScalar v) { this->set(kMTransX, v); }

    /** Sets vertical translation.

        @param v  vertical translation to store
    */
    void setTranslateY(SkScalar v) { this->set(kMTransY, v); }

    /** Sets input x perspective factor, which causes mapXY() to vary input x inversely
        proportional to input y.

        @param v  perspective factor
    */
    void setPerspX(SkScalar v) { this->set(kMPersp0, v); }

    /** Sets input y perspective factor, which causes mapXY() to vary input y inversely
        proportional to input x.

        @param v  perspective factor
    */
    void setPerspY(SkScalar v) { this->set(kMPersp1, v); }

    /** Sets all values from parameters. Sets matrix to:

            | scaleX  skewX transX |
            |  skewY scaleY transY |
            | persp0 persp1 persp2 |

        @param scaleX  horizontal scale factor to store
        @param skewX   horizontal skew factor to store
        @param transX  horizontal translation to store
        @param skewY   vertical skew factor to store
        @param scaleY  vertical scale factor to store
        @param transY  vertical translation to store
        @param persp0  input x perspective factor to store
        @param persp1  input y perspective factor to store
        @param persp2  perspective scale factor to store
    */
    void setAll(SkScalar scaleX, SkScalar skewX,  SkScalar transX,
                SkScalar skewY,  SkScalar scaleY, SkScalar transY,
                SkScalar persp0, SkScalar persp1, SkScalar persp2) {
        fMat[kMScaleX] = scaleX;
        fMat[kMSkewX]  = skewX;
        fMat[kMTransX] = transX;
        fMat[kMSkewY]  = skewY;
        fMat[kMScaleY] = scaleY;
        fMat[kMTransY] = transY;
        fMat[kMPersp0] = persp0;
        fMat[kMPersp1] = persp1;
        fMat[kMPersp2] = persp2;
        this->setTypeMask(kUnknown_Mask);
    }

    /** Copies nine scalar values contained by SkMatrix into buffer, in member value
        ascending order: kMScaleX, kMSkewX, kMTransX, kMSkewY, kMScaleY, kMTransY,
        kMPersp0, kMPersp1, kMPersp2.

        @param buffer  storage for nine scalar values
    */
    void get9(SkScalar buffer[9]) const {
        memcpy(buffer, fMat, 9 * sizeof(SkScalar));
    }

    /** Sets SkMatrix to nine scalar values in buffer, in member value ascending order:
        kMScaleX, kMSkewX, kMTransX, kMSkewY, kMScaleY, kMTransY, kMPersp0, kMPersp1,
        kMPersp2.

        Sets matrix to:

            | buffer[0] buffer[1] buffer[2] |
            | buffer[3] buffer[4] buffer[5] |
            | buffer[6] buffer[7] buffer[8] |

        In the future, set9 followed by get9 may not return the same values. Since SkMatrix
        maps non-homogeneous coordinates, scaling all nine values produces an equivalent
        transformation, possibly improving precision.

        @param buffer  nine scalar values
    */
    void set9(const SkScalar buffer[9]);

    /** Sets SkMatrix to identity; which has no effect on mapped points. Sets SkMatrix to:

            | 1 0 0 |
            | 0 1 0 |
            | 0 0 1 |

        Also called setIdentity(); use the one that provides better inline
        documentation.
    */
    void reset();

    /** Sets SkMatrix to identity; which has no effect on mapped points. Sets SkMatrix to:

            | 1 0 0 |
            | 0 1 0 |
            | 0 0 1 |

        Also called reset(); use the one that provides better inline
        documentation.
    */
    void setIdentity() { this->reset(); }

    /** Sets SkMatrix to translate by (dx, dy).

        @param dx  horizontal translation
        @param dy  vertical translation
    */
    void setTranslate(SkScalar dx, SkScalar dy);

    /** Sets SkMatrix to translate by (v.fX, v.fY).

        @param v  vector containing horizontal and vertical translation
    */
    void setTranslate(const SkVector& v) { this->setTranslate(v.fX, v.fY); }

    /** Sets SkMatrix to scale by sx and sy, about a pivot point at (px, py).
        The pivot point is unchanged when mapped with SkMatrix.

        @param sx  horizontal scale factor
        @param sy  vertical scale factor
        @param px  pivot x
        @param py  pivot y
    */
    void setScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py);

    /** Sets SkMatrix to scale by sx and sy about at pivot point at (0, 0).

        @param sx  horizontal scale factor
        @param sy  vertical scale factor
    */
    void setScale(SkScalar sx, SkScalar sy);

    /** Sets SkMatrix to rotate by degrees about a pivot point at (px, py).
        The pivot point is unchanged when mapped with SkMatrix.

        Positive degrees rotates clockwise.

        @param degrees  angle of axes relative to upright axes
        @param px       pivot x
        @param py       pivot y
    */
    void setRotate(SkScalar degrees, SkScalar px, SkScalar py);

    /** Sets SkMatrix to rotate by degrees about a pivot point at (0, 0).
        Positive degrees rotates clockwise.

        @param degrees  angle of axes relative to upright axes
    */
    void setRotate(SkScalar degrees);

    /** Sets SkMatrix to rotate by sinValue and cosValue, about a pivot point at (px, py).
        The pivot point is unchanged when mapped with SkMatrix.

        Vector (sinValue, cosValue) describes the angle of rotation relative to (0, 1).
        Vector length specifies scale.

        @param sinValue  rotation vector x component
        @param cosValue  rotation vector y component
        @param px        pivot x
        @param py        pivot y
    */
    void setSinCos(SkScalar sinValue, SkScalar cosValue,
                   SkScalar px, SkScalar py);

    /** Sets SkMatrix to rotate by sinValue and cosValue, about a pivot point at (0, 0).

        Vector (sinValue, cosValue) describes the angle of rotation relative to (0, 1).
        Vector length specifies scale.

        @param sinValue  rotation vector x component
        @param cosValue  rotation vector y component
    */
    void setSinCos(SkScalar sinValue, SkScalar cosValue);

    /** Sets SkMatrix to rotate, scale, and translate using a compressed matrix form.

        Vector (rsxForm.fSSin, rsxForm.fSCos) describes the angle of rotation relative
        to (0, 1). Vector length specifies scale. Mapped point is rotated and scaled
        by vector, then translated by (rsxForm.fTx, rsxForm.fTy).

        @param rsxForm  compressed SkRSXform matrix
        @return         reference to SkMatrix
    */
    SkMatrix& setRSXform(const SkRSXform& rsxForm);

    /** Sets SkMatrix to skew by kx and ky, about a pivot point at (px, py).
        The pivot point is unchanged when mapped with SkMatrix.

        @param kx  horizontal skew factor
        @param ky  vertical skew factor
        @param px  pivot x
        @param py  pivot y
    */
    void setSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py);

    /** Sets SkMatrix to skew by kx and ky, about a pivot point at (0, 0).

        @param kx  horizontal skew factor
        @param ky  vertical skew factor
    */
    void setSkew(SkScalar kx, SkScalar ky);

    /** Sets SkMatrix to SkMatrix a multiplied by SkMatrix b. Either a or b may be this.

        Given:

                | A B C |      | J K L |
            a = | D E F |, b = | M N O |
                | G H I |      | P Q R |

        sets SkMatrix to:

                    | A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
            a * b = | D E F | * | M N O | = | DJ+EM+FP DK+EN+FQ DL+EO+FR |
                    | G H I |   | P Q R |   | GJ+HM+IP GK+HN+IQ GL+HO+IR |

        @param a  SkMatrix on left side of multiply expression
        @param b  SkMatrix on right side of multiply expression
    */
    void setConcat(const SkMatrix& a, const SkMatrix& b);

    /** Sets SkMatrix to SkMatrix multiplied by SkMatrix constructed from translation (dx, dy).
        This can be thought of as moving the point to be mapped before applying SkMatrix.

        Given:

                     | A B C |               | 1 0 dx |
            Matrix = | D E F |,  T(dx, dy) = | 0 1 dy |
                     | G H I |               | 0 0  1 |

        sets SkMatrix to:

                                 | A B C | | 1 0 dx |   | A B A*dx+B*dy+C |
            Matrix * T(dx, dy) = | D E F | | 0 1 dy | = | D E D*dx+E*dy+F |
                                 | G H I | | 0 0  1 |   | G H G*dx+H*dy+I |

        @param dx  x translation before applying SkMatrix
        @param dy  y translation before applying SkMatrix
    */
    void preTranslate(SkScalar dx, SkScalar dy);

    /** Sets SkMatrix to SkMatrix multiplied by SkMatrix constructed from scaling by (sx, sy)
        about pivot point (px, py).
        This can be thought of as scaling about a pivot point before applying SkMatrix.

        Given:

                     | A B C |                       | sx  0 dx |
            Matrix = | D E F |,  S(sx, sy, px, py) = |  0 sy dy |
                     | G H I |                       |  0  0  1 |

        where

            dx = px - sx * px
            dy = py - sy * py

        sets SkMatrix to:

                                         | A B C | | sx  0 dx |   | A*sx B*sy A*dx+B*dy+C |
            Matrix * S(sx, sy, px, py) = | D E F | |  0 sy dy | = | D*sx E*sy D*dx+E*dy+F |
                                         | G H I | |  0  0  1 |   | G*sx H*sy G*dx+H*dy+I |

        @param sx  horizontal scale factor
        @param sy  vertical scale factor
        @param px  pivot x
        @param py  pivot y
    */
    void preScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py);

    /** Sets SkMatrix to SkMatrix multiplied by SkMatrix constructed from scaling by (sx, sy)
        about pivot point (0, 0).
        This can be thought of as scaling about the origin before applying SkMatrix.

        Given:

                     | A B C |               | sx  0  0 |
            Matrix = | D E F |,  S(sx, sy) = |  0 sy  0 |
                     | G H I |               |  0  0  1 |

        sets SkMatrix to:

                                 | A B C | | sx  0  0 |   | A*sx B*sy C |
            Matrix * S(sx, sy) = | D E F | |  0 sy  0 | = | D*sx E*sy F |
                                 | G H I | |  0  0  1 |   | G*sx H*sy I |

        @param sx  horizontal scale factor
        @param sy  vertical scale factor
    */
    void preScale(SkScalar sx, SkScalar sy);

    /** Sets SkMatrix to SkMatrix multiplied by SkMatrix constructed from rotating by degrees
        about pivot point (px, py).
        This can be thought of as rotating about a pivot point before applying SkMatrix.

        Positive degrees rotates clockwise.

        Given:

                     | A B C |                        | c -s dx |
            Matrix = | D E F |,  R(degrees, px, py) = | s  c dy |
                     | G H I |                        | 0  0  1 |

        where

            c  = cos(degrees)
            s  = sin(degrees)
            dx =  s * py + (1 - c) * px
            dy = -s * px + (1 - c) * py

        sets SkMatrix to:

                                          | A B C | | c -s dx |   | Ac+Bs -As+Bc A*dx+B*dy+C |
            Matrix * R(degrees, px, py) = | D E F | | s  c dy | = | Dc+Es -Ds+Ec D*dx+E*dy+F |
                                          | G H I | | 0  0  1 |   | Gc+Hs -Gs+Hc G*dx+H*dy+I |

        @param degrees  angle of axes relative to upright axes
        @param px       pivot x
        @param py       pivot y
    */
    void preRotate(SkScalar degrees, SkScalar px, SkScalar py);

    /** Sets SkMatrix to SkMatrix multiplied by SkMatrix constructed from rotating by degrees
        about pivot point (0, 0).
        This can be thought of as rotating about the origin before applying SkMatrix.

        Positive degrees rotates clockwise.

        Given:

                     | A B C |                        | c -s 0 |
            Matrix = | D E F |,  R(degrees, px, py) = | s  c 0 |
                     | G H I |                        | 0  0 1 |

        where

            c  = cos(degrees)
            s  = sin(degrees)

        sets SkMatrix to:

                                          | A B C | | c -s 0 |   | Ac+Bs -As+Bc C |
            Matrix * R(degrees, px, py) = | D E F | | s  c 0 | = | Dc+Es -Ds+Ec F |
                                          | G H I | | 0  0 1 |   | Gc+Hs -Gs+Hc I |

        @param degrees  angle of axes relative to upright axes
    */
    void preRotate(SkScalar degrees);

    /** Sets SkMatrix to SkMatrix multiplied by SkMatrix constructed from skewing by (kx, ky)
        about pivot point (px, py).
        This can be thought of as skewing about a pivot point before applying SkMatrix.

        Given:

                     | A B C |                       |  1 kx dx |
            Matrix = | D E F |,  K(kx, ky, px, py) = | ky  1 dy |
                     | G H I |                       |  0  0  1 |

        where

            dx = -kx * py
            dy = -ky * px

        sets SkMatrix to:

                                         | A B C | |  1 kx dx |   | A+B*ky A*kx+B A*dx+B*dy+C |
            Matrix * K(kx, ky, px, py) = | D E F | | ky  1 dy | = | D+E*ky D*kx+E D*dx+E*dy+F |
                                         | G H I | |  0  0  1 |   | G+H*ky G*kx+H G*dx+H*dy+I |

        @param kx  horizontal skew factor
        @param ky  vertical skew factor
        @param px  pivot x
        @param py  pivot y
    */
    void preSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py);

    /** Sets SkMatrix to SkMatrix multiplied by SkMatrix constructed from skewing by (kx, ky)
        about pivot point (0, 0).
        This can be thought of as skewing about the origin before applying SkMatrix.

        Given:

                     | A B C |               |  1 kx 0 |
            Matrix = | D E F |,  K(kx, ky) = | ky  1 0 |
                     | G H I |               |  0  0 1 |

        sets SkMatrix to:

                                 | A B C | |  1 kx 0 |   | A+B*ky A*kx+B C |
            Matrix * K(kx, ky) = | D E F | | ky  1 0 | = | D+E*ky D*kx+E F |
                                 | G H I | |  0  0 1 |   | G+H*ky G*kx+H I |

        @param kx  horizontal skew factor
        @param ky  vertical skew factor
    */
    void preSkew(SkScalar kx, SkScalar ky);

    /** Sets SkMatrix to SkMatrix multiplied by SkMatrix other.
        This can be thought of mapping by other before applying SkMatrix.

        Given:

                     | A B C |          | J K L |
            Matrix = | D E F |, other = | M N O |
                     | G H I |          | P Q R |

        sets SkMatrix to:

                             | A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
            Matrix * other = | D E F | * | M N O | = | DJ+EM+FP DK+EN+FQ DL+EO+FR |
                             | G H I |   | P Q R |   | GJ+HM+IP GK+HN+IQ GL+HO+IR |

        @param other  SkMatrix on right side of multiply expression
    */
    void preConcat(const SkMatrix& other);

    /** Sets SkMatrix to SkMatrix constructed from translation (dx, dy) multiplied by SkMatrix.
        This can be thought of as moving the point to be mapped after applying SkMatrix.

        Given:

                     | J K L |               | 1 0 dx |
            Matrix = | M N O |,  T(dx, dy) = | 0 1 dy |
                     | P Q R |               | 0 0  1 |

        sets SkMatrix to:

                                 | 1 0 dx | | J K L |   | J+dx*P K+dx*Q L+dx*R |
            T(dx, dy) * Matrix = | 0 1 dy | | M N O | = | M+dy*P N+dy*Q O+dy*R |
                                 | 0 0  1 | | P Q R |   |      P      Q      R |

        @param dx  x translation after applying SkMatrix
        @param dy  y translation after applying SkMatrix
    */
    void postTranslate(SkScalar dx, SkScalar dy);

    /** Sets SkMatrix to SkMatrix constructed from scaling by (sx, sy) about pivot point
        (px, py), multiplied by SkMatrix.
        This can be thought of as scaling about a pivot point after applying SkMatrix.

        Given:

                     | J K L |                       | sx  0 dx |
            Matrix = | M N O |,  S(sx, sy, px, py) = |  0 sy dy |
                     | P Q R |                       |  0  0  1 |

        where

            dx = px - sx * px
            dy = py - sy * py

        sets SkMatrix to:

                                         | sx  0 dx | | J K L |   | sx*J+dx*P sx*K+dx*Q sx*L+dx+R |
            S(sx, sy, px, py) * Matrix = |  0 sy dy | | M N O | = | sy*M+dy*P sy*N+dy*Q sy*O+dy*R |
                                         |  0  0  1 | | P Q R |   |         P         Q         R |

        @param sx  horizontal scale factor
        @param sy  vertical scale factor
        @param px  pivot x
        @param py  pivot y
    */
    void postScale(SkScalar sx, SkScalar sy, SkScalar px, SkScalar py);

    /** Sets SkMatrix to SkMatrix constructed from scaling by (sx, sy) about pivot point
        (0, 0), multiplied by SkMatrix.
        This can be thought of as scaling about the origin after applying SkMatrix.

        Given:

                     | J K L |               | sx  0  0 |
            Matrix = | M N O |,  S(sx, sy) = |  0 sy  0 |
                     | P Q R |               |  0  0  1 |

        sets SkMatrix to:

                                 | sx  0  0 | | J K L |   | sx*J sx*K sx*L |
            S(sx, sy) * Matrix = |  0 sy  0 | | M N O | = | sy*M sy*N sy*O |
                                 |  0  0  1 | | P Q R |   |    P    Q    R |

        @param sx  horizontal scale factor
        @param sy  vertical scale factor
    */
    void postScale(SkScalar sx, SkScalar sy);

    /** Sets SkMatrix to SkMatrix constructed from scaling by (1/divx, 1/divy) about pivot point (px, py), multiplied by SkMatrix.

        Returns false if either divx or divy is zero.

        Given:

                     | J K L |                   | sx  0  0 |
            Matrix = | M N O |,  I(divx, divy) = |  0 sy  0 |
                     | P Q R |                   |  0  0  1 |

        where

            sx = 1 / divx
            sy = 1 / divy

        sets SkMatrix to:

                                     | sx  0  0 | | J K L |   | sx*J sx*K sx*L |
            I(divx, divy) * Matrix = |  0 sy  0 | | M N O | = | sy*M sy*N sy*O |
                                     |  0  0  1 | | P Q R |   |    P    Q    R |

        @param divx  integer divisor for inverse scale in x
        @param divy  integer divisor for inverse scale in y
        @return      true on successful scale
    */
    bool postIDiv(int divx, int divy);

    /** Sets SkMatrix to SkMatrix constructed from rotating by degrees about pivot point
        (px, py), multiplied by SkMatrix.
        This can be thought of as rotating about a pivot point after applying SkMatrix.

        Positive degrees rotates clockwise.

        Given:

                     | J K L |                        | c -s dx |
            Matrix = | M N O |,  R(degrees, px, py) = | s  c dy |
                     | P Q R |                        | 0  0  1 |

        where

            c  = cos(degrees)
            s  = sin(degrees)
            dx =  s * py + (1 - c) * px
            dy = -s * px + (1 - c) * py

        sets SkMatrix to:

                                          |c -s dx| |J K L|   |cJ-sM+dx*P cK-sN+dx*Q cL-sO+dx+R|
            R(degrees, px, py) * Matrix = |s  c dy| |M N O| = |sJ+cM+dy*P sK+cN+dy*Q sL+cO+dy*R|
                                          |0  0  1| |P Q R|   |         P          Q          R|

        @param degrees  angle of axes relative to upright axes
        @param px       pivot x
        @param py       pivot y
    */
    void postRotate(SkScalar degrees, SkScalar px, SkScalar py);

    /** Sets SkMatrix to SkMatrix constructed from rotating by degrees about pivot point
        (0, 0), multiplied by SkMatrix.
        This can be thought of as rotating about the origin after applying SkMatrix.

        Positive degrees rotates clockwise.

        Given:

                     | J K L |                        | c -s 0 |
            Matrix = | M N O |,  R(degrees, px, py) = | s  c 0 |
                     | P Q R |                        | 0  0 1 |

        where

            c  = cos(degrees)
            s  = sin(degrees)

        sets SkMatrix to:

                                          | c -s dx | | J K L |   | cJ-sM cK-sN cL-sO |
            R(degrees, px, py) * Matrix = | s  c dy | | M N O | = | sJ+cM sK+cN sL+cO |
                                          | 0  0  1 | | P Q R |   |     P     Q     R |

        @param degrees  angle of axes relative to upright axes
    */
    void postRotate(SkScalar degrees);

    /** Sets SkMatrix to SkMatrix constructed from skewing by (kx, ky) about pivot point
        (px, py), multiplied by SkMatrix.
        This can be thought of as skewing about a pivot point after applying SkMatrix.

        Given:

                     | J K L |                       |  1 kx dx |
            Matrix = | M N O |,  K(kx, ky, px, py) = | ky  1 dy |
                     | P Q R |                       |  0  0  1 |

        where

            dx = -kx * py
            dy = -ky * px

        sets SkMatrix to:

                                         | 1 kx dx| |J K L|   |J+kx*M+dx*P K+kx*N+dx*Q L+kx*O+dx+R|
            K(kx, ky, px, py) * Matrix = |ky  1 dy| |M N O| = |ky*J+M+dy*P ky*K+N+dy*Q ky*L+O+dy*R|
                                         | 0  0  1| |P Q R|   |          P           Q           R|

        @param kx  horizontal skew factor
        @param ky  vertical skew factor
        @param px  pivot x
        @param py  pivot y
    */
    void postSkew(SkScalar kx, SkScalar ky, SkScalar px, SkScalar py);

    /** Sets SkMatrix to SkMatrix constructed from skewing by (kx, ky) about pivot point
        (0, 0), multiplied by SkMatrix.
        This can be thought of as skewing about the origin after applying SkMatrix.

        Given:

                     | J K L |               |  1 kx 0 |
            Matrix = | M N O |,  K(kx, ky) = | ky  1 0 |
                     | P Q R |               |  0  0 1 |

        sets SkMatrix to:

                                 |  1 kx 0 | | J K L |   | J+kx*M K+kx*N L+kx*O |
            K(kx, ky) * Matrix = | ky  1 0 | | M N O | = | ky*J+M ky*K+N ky*L+O |
                                 |  0  0 1 | | P Q R |   |      P      Q      R |

        @param kx  horizontal skew factor
        @param ky  vertical skew factor
    */
    void postSkew(SkScalar kx, SkScalar ky);

    /** Sets SkMatrix to SkMatrix other multiplied by SkMatrix.
        This can be thought of mapping by other after applying SkMatrix.

        Given:

                     | J K L |           | A B C |
            Matrix = | M N O |,  other = | D E F |
                     | P Q R |           | G H I |

        sets SkMatrix to:

                             | A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
            other * Matrix = | D E F | * | M N O | = | DJ+EM+FP DK+EN+FQ DL+EO+FR |
                             | G H I |   | P Q R |   | GJ+HM+IP GK+HN+IQ GL+HO+IR |

        @param other  SkMatrix on left side of multiply expression
    */
    void postConcat(const SkMatrix& other);

    /** \enum SkMatrix::ScaleToFit
        ScaleToFit describes how SkMatrix is constructed to map one SkRect to another.
        ScaleToFit may allow SkMatrix to have unequal horizontal and vertical scaling,
        or may restrict SkMatrix to square scaling. If restricted, ScaleToFit specifies
        how SkMatrix maps to the side or center of the destination SkRect.
    */
    enum ScaleToFit {
        /** Computes SkMatrix that scales in x and y independently, so that source SkRect is
            mapped to completely fill destination SkRect. The aspect ratio of source SkRect
            may change.
        */
        kFill_ScaleToFit,

        /** Computes SkMatrix that maintains source SkRect aspect ratio, mapping source SkRect
            width or height to destination SkRect. Aligns mapping to left and top edges
            of destination SkRect.
        */
        kStart_ScaleToFit,

        /** Computes SkMatrix that maintains source SkRect aspect ratio, mapping source SkRect
            width or height to destination SkRect. Aligns mapping to center of destination
            SkRect.
        */
        kCenter_ScaleToFit,

        /** Computes SkMatrix that maintains source SkRect aspect ratio, mapping source SkRect
            width or height to destination SkRect. Aligns mapping to right and bottom
            edges of destination SkRect.
        */
        kEnd_ScaleToFit,
    };

    /** Sets SkMatrix to scale and translate src SkRect to dst SkRect. stf selects whether
        mapping completely fills dst or preserves the aspect ratio, and how to align
        src within dst. Returns false if src is empty, and sets SkMatrix to identity.
        Returns true if dst is empty, and sets SkMatrix to:

            | 0 0 0 |
            | 0 0 0 |
            | 0 0 1 |

        @param src  SkRect to map from
        @param dst  SkRect to map to
        @param stf  one of: kFill_ScaleToFit, kStart_ScaleToFit,
                    kCenter_ScaleToFit, kEnd_ScaleToFit
        @return     true if SkMatrix can represent SkRect mapping
    */
    bool setRectToRect(const SkRect& src, const SkRect& dst, ScaleToFit stf);

    /** Returns SkMatrix set to scale and translate src SkRect to dst SkRect. stf selects
        whether mapping completely fills dst or preserves the aspect ratio, and how to
        align src within dst. Returns the identity SkMatrix if src is empty. If dst is
        empty, returns SkMatrix set to:

            | 0 0 0 |
            | 0 0 0 |
            | 0 0 1 |

        @param src  SkRect to map from
        @param dst  SkRect to map to
        @param stf  one of: kFill_ScaleToFit, kStart_ScaleToFit,
                    kCenter_ScaleToFit, kEnd_ScaleToFit
        @return     SkMatrix mapping src to dst
    */
    static SkMatrix MakeRectToRect(const SkRect& src, const SkRect& dst, ScaleToFit stf) {
        SkMatrix m;
        m.setRectToRect(src, dst, stf);
        return m;
    }

    /** Sets SkMatrix to map src to dst. count must be zero or greater, and four or less.

        If count is zero, sets SkMatrix to identity and returns true.
        If count is one, sets SkMatrix to translate and returns true.
        If count is two or more, sets SkMatrix to map points if possible; returns false
        if SkMatrix cannot be constructed. If count is four, SkMatrix may include
        perspective.

        @param src    points to map from
        @param dst    points to map to
        @param count  number of points in src and dst
        @return       true if SkMatrix was constructed successfully
    */
    bool setPolyToPoly(const SkPoint src[], const SkPoint dst[], int count);

    /** Sets inverse to reciprocal matrix, returning true if SkMatrix can be inverted.
        Geometrically, if SkMatrix maps from source to destination, inverse SkMatrix
        maps from destination to source. If SkMatrix can not be inverted, inverse is
        unchanged.

        @param inverse  storage for inverted SkMatrix; may be nullptr
        @return         true if SkMatrix can be inverted
    */
    bool SK_WARN_UNUSED_RESULT invert(SkMatrix* inverse) const {
        // Allow the trivial case to be inlined.
        if (this->isIdentity()) {
            if (inverse) {
                inverse->reset();
            }
            return true;
        }
        return this->invertNonIdentity(inverse);
    }

    /** Fills affine with identity values in column major order.
        Sets affine to:

            | 1 0 0 |
            | 0 1 0 |

        Affine 3x2 matrices in column major order are used by OpenGL and XPS.

        @param affine  storage for 3x2 affine matrix
    */
    static void SetAffineIdentity(SkScalar affine[6]);

    /** Fills affine in column major order. Sets affine to:

            | scale-x  skew-x translate-x |
            | skew-y  scale-y translate-y |

        If SkMatrix contains perspective, returns false and leaves affine unchanged.

        @param affine  storage for 3x2 affine matrix; may be nullptr
        @return        true if SkMatrix does not contain perspective
    */
    bool SK_WARN_UNUSED_RESULT asAffine(SkScalar affine[6]) const;

    /** Sets SkMatrix to affine values, passed in column major order. Given affine,
        column, then row, as:

            | scale-x  skew-x translate-x |
            |  skew-y scale-y translate-y |

        SkMatrix is set, row, then column, to:

            | scale-x  skew-x translate-x |
            |  skew-y scale-y translate-y |
            |       0       0           1 |

        @param affine  3x2 affine matrix
    */
    void setAffine(const SkScalar affine[6]);

    /** Maps src SkPoint array of length count to dst SkPoint array of equal or greater
        length. Points are mapped by multiplying each SkPoint by SkMatrix. Given:

                     | A B C |        | x |
            Matrix = | D E F |,  pt = | y |
                     | G H I |        | 1 |

        where

            for (i = 0; i < count; ++i) {
                x = src[i].fX
                y = src[i].fY
            }

        each dst SkPoint is computed as:

                          |A B C| |x|                               Ax+By+C   Dx+Ey+F
            Matrix * pt = |D E F| |y| = |Ax+By+C Dx+Ey+F Gx+Hy+I| = ------- , -------
                          |G H I| |1|                               Gx+Hy+I   Gx+Hy+I

        src and dst may point to the same storage.

        @param dst    storage for mapped points
        @param src    points to transform
        @param count  number of points to transform
    */
    void mapPoints(SkPoint dst[], const SkPoint src[], int count) const {
        SkASSERT((dst && src && count > 0) || 0 == count);
        // no partial overlap
        SkASSERT(src == dst || &dst[count] <= &src[0] || &src[count] <= &dst[0]);
        this->getMapPtsProc()(*this, dst, src, count);
    }

    /** Maps pts SkPoint array of length count in place. Points are mapped by multiplying
        each SkPoint by SkMatrix. Given:

                     | A B C |        | x |
            Matrix = | D E F |,  pt = | y |
                     | G H I |        | 1 |

        where

            for (i = 0; i < count; ++i) {
                x = pts[i].fX
                y = pts[i].fY
            }

        each resulting pts SkPoint is computed as:

                          |A B C| |x|                               Ax+By+C   Dx+Ey+F
            Matrix * pt = |D E F| |y| = |Ax+By+C Dx+Ey+F Gx+Hy+I| = ------- , -------
                          |G H I| |1|                               Gx+Hy+I   Gx+Hy+I

        @param pts    storage for mapped points
        @param count  number of points to transform
    */
    void mapPoints(SkPoint pts[], int count) const {
        this->mapPoints(pts, pts, count);
    }

    /** Maps src SkPoint3 array of length count to dst SkPoint3 array, which must of length count or
        greater. SkPoint3 array is mapped by multiplying each SkPoint3 by SkMatrix. Given:

                     | A B C |         | x |
            Matrix = | D E F |,  src = | y |
                     | G H I |         | z |

        each resulting dst SkPoint is computed as:

                           |A B C| |x|
            Matrix * src = |D E F| |y| = |Ax+By+Cz Dx+Ey+Fz Gx+Hy+Iz|
                           |G H I| |z|

        @param dst    storage for mapped SkPoint3 array
        @param src    SkPoint3 array to transform
        @param count  items in SkPoint3 array to transform
    */
    void mapHomogeneousPoints(SkPoint3 dst[], const SkPoint3 src[], int count) const;

    /** Maps SkPoint (x, y) to result. SkPoint is mapped by multiplying by SkMatrix. Given:

                     | A B C |        | x |
            Matrix = | D E F |,  pt = | y |
                     | G H I |        | 1 |

        result is computed as:

                          |A B C| |x|                               Ax+By+C   Dx+Ey+F
            Matrix * pt = |D E F| |y| = |Ax+By+C Dx+Ey+F Gx+Hy+I| = ------- , -------
                          |G H I| |1|                               Gx+Hy+I   Gx+Hy+I

        @param x       x-coordinate of SkPoint to map
        @param y       y-coordinate of SkPoint to map
        @param result  storage for mapped SkPoint
    */
    void mapXY(SkScalar x, SkScalar y, SkPoint* result) const {
        SkASSERT(result);
        this->getMapXYProc()(*this, x, y, result);
    }

    /** Returns SkPoint (x, y) multiplied by SkMatrix. Given:

                     | A B C |        | x |
            Matrix = | D E F |,  pt = | y |
                     | G H I |        | 1 |

        result is computed as:

                          |A B C| |x|                               Ax+By+C   Dx+Ey+F
            Matrix * pt = |D E F| |y| = |Ax+By+C Dx+Ey+F Gx+Hy+I| = ------- , -------
                          |G H I| |1|                               Gx+Hy+I   Gx+Hy+I

        @param x  x-coordinate of SkPoint to map
        @param y  y-coordinate of SkPoint to map
        @return   mapped SkPoint
    */
    SkPoint mapXY(SkScalar x, SkScalar y) const {
        SkPoint result;
        this->getMapXYProc()(*this, x, y, &result);
        return result;
    }

    /** Maps src vector array of length count to vector SkPoint array of equal or greater
        length. Vectors are mapped by multiplying each vector by SkMatrix, treating
        SkMatrix translation as zero. Given:

                     | A B 0 |         | x |
            Matrix = | D E 0 |,  src = | y |
                     | G H I |         | 1 |

        where

            for (i = 0; i < count; ++i) {
                x = src[i].fX
                y = src[i].fY
            }

        each dst vector is computed as:

                           |A B 0| |x|                            Ax+By     Dx+Ey
            Matrix * src = |D E 0| |y| = |Ax+By Dx+Ey Gx+Hy+I| = ------- , -------
                           |G H I| |1|                           Gx+Hy+I   Gx+Hy+I

        src and dst may point to the same storage.

        @param dst    storage for mapped vectors
        @param src    vectors to transform
        @param count  number of vectors to transform
    */
    void mapVectors(SkVector dst[], const SkVector src[], int count) const;

    /** Maps vecs vector array of length count in place, multiplying each vector by
        SkMatrix, treating SkMatrix translation as zero. Given:

                     | A B 0 |         | x |
            Matrix = | D E 0 |,  vec = | y |
                     | G H I |         | 1 |

        where

            for (i = 0; i < count; ++i) {
                x = vecs[i].fX
                y = vecs[i].fY
            }

        each result vector is computed as:

                           |A B 0| |x|                            Ax+By     Dx+Ey
            Matrix * vec = |D E 0| |y| = |Ax+By Dx+Ey Gx+Hy+I| = ------- , -------
                           |G H I| |1|                           Gx+Hy+I   Gx+Hy+I

        @param vecs   vectors to transform, and storage for mapped vectors
        @param count  number of vectors to transform
    */
    void mapVectors(SkVector vecs[], int count) const {
        this->mapVectors(vecs, vecs, count);
    }

    /** Maps vector (x, y) to result. Vector is mapped by multiplying by SkMatrix,
        treating SkMatrix translation as zero. Given:

                     | A B 0 |         | dx |
            Matrix = | D E 0 |,  vec = | dy |
                     | G H I |         |  1 |

        each result vector is computed as:

                       |A B 0| |dx|                                        A*dx+B*dy     D*dx+E*dy
        Matrix * vec = |D E 0| |dy| = |A*dx+B*dy D*dx+E*dy G*dx+H*dy+I| = ----------- , -----------
                       |G H I| | 1|                                       G*dx+H*dy+I   G*dx+*dHy+I

        @param dx      x-coordinate of vector to map
        @param dy      y-coordinate of vector to map
        @param result  storage for mapped vector
    */
    void mapVector(SkScalar dx, SkScalar dy, SkVector* result) const {
        SkVector vec = { dx, dy };
        this->mapVectors(result, &vec, 1);
    }

    /** Returns vector (x, y) multiplied by SkMatrix, treating SkMatrix translation as zero.
        Given:

                     | A B 0 |         | dx |
            Matrix = | D E 0 |,  vec = | dy |
                     | G H I |         |  1 |

        each result vector is computed as:

                       |A B 0| |dx|                                        A*dx+B*dy     D*dx+E*dy
        Matrix * vec = |D E 0| |dy| = |A*dx+B*dy D*dx+E*dy G*dx+H*dy+I| = ----------- , -----------
                       |G H I| | 1|                                       G*dx+H*dy+I   G*dx+*dHy+I

        @param dx  x-coordinate of vector to map
        @param dy  y-coordinate of vector to map
        @return    mapped vector
    */
    SkVector mapVector(SkScalar dx, SkScalar dy) const {
        SkVector vec = { dx, dy };
        this->mapVectors(&vec, &vec, 1);
        return vec;
    }

    /** Sets dst to bounds of src corners mapped by SkMatrix.
        Returns true if mapped corners are dst corners.

        Returned value is the same as calling rectStaysRect().

        @param dst  storage for bounds of mapped points
        @param src  SkRect to map
        @return     true if dst is equivalent to mapped src
    */
    bool mapRect(SkRect* dst, const SkRect& src) const;

    /** Sets rect to bounds of rect corners mapped by SkMatrix.
        Returns true if mapped corners are computed rect corners.

        Returned value is the same as calling rectStaysRect().

        @param rect  rectangle to map, and storage for bounds of mapped corners
        @return      true if result is equivalent to mapped src
    */
    bool mapRect(SkRect* rect) const {
        return this->mapRect(rect, *rect);
    }

    /** Maps four corners of rect to dst. Points are mapped by multiplying each
        rect corner by SkMatrix. rect corner is processed in this order:
        (rect.fLeft, rect.fTop), (rect.fRight, rect.fTop), (rect.fRight, rect.fBottom),
        (rect.fLeft, rect.fBottom).

        rect may be empty: rect.fLeft may be greater than or equal to rect.fRight;
        rect.fTop may be greater than or equal to rect.fBottom.

        Given:

                     | A B C |        | x |
            Matrix = | D E F |,  pt = | y |
                     | G H I |        | 1 |

        where pt is initialized from each of (rect.fLeft, rect.fTop),
        (rect.fRight, rect.fTop), (rect.fRight, rect.fBottom), (rect.fLeft, rect.fBottom),
        each dst SkPoint is computed as:

                          |A B C| |x|                               Ax+By+C   Dx+Ey+F
            Matrix * pt = |D E F| |y| = |Ax+By+C Dx+Ey+F Gx+Hy+I| = ------- , -------
                          |G H I| |1|                               Gx+Hy+I   Gx+Hy+I

        @param dst   storage for mapped corner points
        @param rect  SkRect to map
    */
    void mapRectToQuad(SkPoint dst[4], const SkRect& rect) const {
        // This could potentially be faster if we only transformed each x and y of the rect once.
        rect.toQuad(dst);
        this->mapPoints(dst, 4);
    }

    /** Sets dst to bounds of src corners mapped by SkMatrix. If matrix contains
        elements other than scale or translate: asserts if SK_DEBUG is defined;
        otherwise, results are undefined.

        @param dst  storage for bounds of mapped points
        @param src  SkRect to map
    */
    void mapRectScaleTranslate(SkRect* dst, const SkRect& src) const;

    /** Returns geometric mean radius of ellipse formed by constructing circle of
        size radius, and mapping constructed circle with SkMatrix. The result squared is
        equal to the major axis length times the minor axis length.
        Result is not meaningful if SkMatrix contains perspective elements.

        @param radius  circle size to map
        @return        average mapped radius
    */
    SkScalar mapRadius(SkScalar radius) const;

    /** Returns true if a unit step in x at some y mapped through SkMatrix can be
        represented by a constant vector. Returns true if getType() returns kIdentity_Mask,
        or combinations of: kTranslate_Mask, kScale_Mask, and kAffine_Mask.

        May return true if getType() returns kPerspective_Mask, but only when SkMatrix
        does not include rotation or skewing along the y-axis.

        @return  true if SkMatrix does not have complex perspective
    */
    bool isFixedStepInX() const;

    /** Returns vector representing a unit step in x at y mapped through SkMatrix.
        If isFixedStepInX() is false, returned value is undefined.

        @param y  position of line parallel to x-axis
        @return   vector advance of mapped unit step in x
    */
    SkVector fixedStepInX(SkScalar y) const;

    /** Returns true if SkMatrix equals m, using an efficient comparison.

        Returns false when the sign of zero values is the different; when one
        matrix has positive zero value and the other has negative zero value.

        Returns true even when both matrices contain NaN.

        NaN never equals any value, including itself. To improve performance, NaN values
        are treated as bit patterns that are equal if their bit patterns are equal.

        @param m  SkMatrix to compare
        @return   true if m and SkMatrix are represented by identical bit patterns
    */
    bool cheapEqualTo(const SkMatrix& m) const {
        return 0 == memcmp(fMat, m.fMat, sizeof(fMat));
    }

    /** Compares a and b; returns true if a and b are numerically equal. Returns true
        even if sign of zero values are different. Returns false if either SkMatrix
        contains NaN, even if the other SkMatrix also contains NaN.

        @param a  SkMatrix to compare
        @param b  SkMatrix to compare
        @return   true if m and SkMatrix are numerically equal
    */
    friend SK_API bool operator==(const SkMatrix& a, const SkMatrix& b);

    /** Compares a and b; returns true if a and b are not numerically equal. Returns false
        even if sign of zero values are different. Returns true if either SkMatrix
        contains NaN, even if the other SkMatrix also contains NaN.

        @param a  SkMatrix to compare
        @param b  SkMatrix to compare
        @return   true if m and SkMatrix are numerically not equal
    */
    friend SK_API bool operator!=(const SkMatrix& a, const SkMatrix& b) {
        return !(a == b);
    }

    /** Writes text representation of SkMatrix to standard output. Floating point values
        are written with limited precision; it may not be possible to reconstruct
        original SkMatrix from output.
    */
    void dump() const;

    /** Creates string representation of SkMatrix. Floating point values
        are written with limited precision; it may not be possible to reconstruct
        original SkMatrix from output.

        @param str  storage for string representation of SkMatrix
    */
    void toString(SkString* str) const;

    /** Returns the minimum scaling factor of SkMatrix by decomposing the scaling and
        skewing elements.
        Returns -1 if scale factor overflows or SkMatrix contains perspective.

        @return  minimum scale factor
    */
    SkScalar getMinScale() const;

    /** Returns the maximum scaling factor of SkMatrix by decomposing the scaling and
        skewing elements.
        Returns -1 if scale factor overflows or SkMatrix contains perspective.

        @return  maximum scale factor
    */
    SkScalar getMaxScale() const;

    /** Sets scaleFactors[0] to the minimum scaling factor, and scaleFactors[1] to the
        maximum scaling factor. Scaling factors are computed by decomposing
        the SkMatrix scaling and skewing elements.

        Returns true if scaleFactors are found; otherwise, returns false and sets
        scaleFactors to undefined values.

        @param scaleFactors  storage for minimum and maximum scale factors
        @return              true if scale factors were computed correctly
    */
    bool SK_WARN_UNUSED_RESULT getMinMaxScales(SkScalar scaleFactors[2]) const;

    /** Decomposes SkMatrix into scale components and whatever remains. Returns false if
        SkMatrix could not be decomposed.

        Sets scale to portion of SkMatrix that scales in x and y. Sets remaining to SkMatrix
        with x and y scaling factored out. remaining may be passed as nullptr
        to determine if SkMatrix can be decomposed without computing remainder.

        Returns true if scale components are found. scale and remaining are
        unchanged if SkMatrix contains perspective; scale factors are not finite, or
        are nearly zero.

        On success
        Matrix = scale * Remaining

        @param scale      x and y scaling factors; may be nullptr
        @param remaining  SkMatrix without scaling; may be nullptr
        @return           true if scale can be computed
    */
    bool decomposeScale(SkSize* scale, SkMatrix* remaining = nullptr) const;

    /** Returns reference to const identity SkMatrix. Returned SkMatrix is set to:

            | 1 0 0 |
            | 0 1 0 |
            | 0 0 1 |

        @return  const identity SkMatrix
    */
    static const SkMatrix& I();

    /** Returns reference to a const SkMatrix with invalid values. Returned SkMatrix is set
        to:

            | SK_ScalarMax SK_ScalarMax SK_ScalarMax |
            | SK_ScalarMax SK_ScalarMax SK_ScalarMax |
            | SK_ScalarMax SK_ScalarMax SK_ScalarMax |

        @return  const invalid SkMatrix
    */
    static const SkMatrix& InvalidMatrix();

    /** Returns SkMatrix a multiplied by SkMatrix b.

        Given:

                | A B C |      | J K L |
            a = | D E F |, b = | M N O |
                | G H I |      | P Q R |

        sets SkMatrix to:

                    | A B C |   | J K L |   | AJ+BM+CP AK+BN+CQ AL+BO+CR |
            a * b = | D E F | * | M N O | = | DJ+EM+FP DK+EN+FQ DL+EO+FR |
                    | G H I |   | P Q R |   | GJ+HM+IP GK+HN+IQ GL+HO+IR |

        @param a  SkMatrix on left side of multiply expression
        @param b  SkMatrix on right side of multiply expression
        @return   SkMatrix computed from a times b
    */
    static SkMatrix Concat(const SkMatrix& a, const SkMatrix& b) {
        SkMatrix result;
        result.setConcat(a, b);
        return result;
    }

    /** Sets internal cache to unknown state. Use to force update after repeated
        modifications to SkMatrix element reference returned by operator[](int index).
    */
    void dirtyMatrixTypeCache() {
        this->setTypeMask(kUnknown_Mask);
    }

    /** Initializes SkMatrix with scale and translate elements.

            | sx  0 tx |
            |  0 sy ty |
            |  0  0  1 |

        @param sx  horizontal scale factor to store
        @param sy  vertical scale factor to store
        @param tx  horizontal translation to store
        @param ty  vertical translation to store
    */
    void setScaleTranslate(SkScalar sx, SkScalar sy, SkScalar tx, SkScalar ty) {
        fMat[kMScaleX] = sx;
        fMat[kMSkewX]  = 0;
        fMat[kMTransX] = tx;

        fMat[kMSkewY]  = 0;
        fMat[kMScaleY] = sy;
        fMat[kMTransY] = ty;

        fMat[kMPersp0] = 0;
        fMat[kMPersp1] = 0;
        fMat[kMPersp2] = 1;

        unsigned mask = 0;
        if (sx != 1 || sy != 1) {
            mask |= kScale_Mask;
        }
        if (tx || ty) {
            mask |= kTranslate_Mask;
        }
        this->setTypeMask(mask | kRectStaysRect_Mask);
    }

    /** Returns true if all elements of the matrix are finite. Returns false if any
        element is infinity, or NaN.

        @return  true if matrix has only finite elements
    */
    bool isFinite() const { return SkScalarsAreFinite(fMat, 9); }

private:
    enum {
        /** Set if the matrix will map a rectangle to another rectangle. This
            can be true if the matrix is scale-only, or rotates a multiple of
            90 degrees.

            This bit will be set on identity matrices
        */
        kRectStaysRect_Mask = 0x10,

        /** Set if the perspective bit is valid even though the rest of
            the matrix is Unknown.
        */
        kOnlyPerspectiveValid_Mask = 0x40,

        kUnknown_Mask = 0x80,

        kORableMasks =  kTranslate_Mask |
                        kScale_Mask |
                        kAffine_Mask |
                        kPerspective_Mask,

        kAllMasks = kTranslate_Mask |
                    kScale_Mask |
                    kAffine_Mask |
                    kPerspective_Mask |
                    kRectStaysRect_Mask,
    };

    SkScalar         fMat[9];
    mutable uint32_t fTypeMask;

    static void ComputeInv(SkScalar dst[9], const SkScalar src[9], double invDet, bool isPersp);

    uint8_t computeTypeMask() const;
    uint8_t computePerspectiveTypeMask() const;

    void setTypeMask(int mask) {
        // allow kUnknown or a valid mask
        SkASSERT(kUnknown_Mask == mask || (mask & kAllMasks) == mask ||
                 ((kUnknown_Mask | kOnlyPerspectiveValid_Mask) & mask)
                 == (kUnknown_Mask | kOnlyPerspectiveValid_Mask));
        fTypeMask = SkToU8(mask);
    }

    void orTypeMask(int mask) {
        SkASSERT((mask & kORableMasks) == mask);
        fTypeMask = SkToU8(fTypeMask | mask);
    }

    void clearTypeMask(int mask) {
        // only allow a valid mask
        SkASSERT((mask & kAllMasks) == mask);
        fTypeMask = fTypeMask & ~mask;
    }

    TypeMask getPerspectiveTypeMaskOnly() const {
        if ((fTypeMask & kUnknown_Mask) &&
            !(fTypeMask & kOnlyPerspectiveValid_Mask)) {
            fTypeMask = this->computePerspectiveTypeMask();
        }
        return (TypeMask)(fTypeMask & 0xF);
    }

    /** Returns true if we already know that the matrix is identity;
        false otherwise.
    */
    bool isTriviallyIdentity() const {
        if (fTypeMask & kUnknown_Mask) {
            return false;
        }
        return ((fTypeMask & 0xF) == 0);
    }

    inline void updateTranslateMask() {
        if ((fMat[kMTransX] != 0) | (fMat[kMTransY] != 0)) {
            fTypeMask |= kTranslate_Mask;
        } else {
            fTypeMask &= ~kTranslate_Mask;
        }
    }

    typedef void (*MapXYProc)(const SkMatrix& mat, SkScalar x, SkScalar y,
                                 SkPoint* result);

    static MapXYProc GetMapXYProc(TypeMask mask) {
        SkASSERT((mask & ~kAllMasks) == 0);
        return gMapXYProcs[mask & kAllMasks];
    }

    MapXYProc getMapXYProc() const {
        return GetMapXYProc(this->getType());
    }

    typedef void (*MapPtsProc)(const SkMatrix& mat, SkPoint dst[],
                                  const SkPoint src[], int count);

    static MapPtsProc GetMapPtsProc(TypeMask mask) {
        SkASSERT((mask & ~kAllMasks) == 0);
        return gMapPtsProcs[mask & kAllMasks];
    }

    MapPtsProc getMapPtsProc() const {
        return GetMapPtsProc(this->getType());
    }

    bool SK_WARN_UNUSED_RESULT invertNonIdentity(SkMatrix* inverse) const;

    static bool Poly2Proc(const SkPoint[], SkMatrix*, const SkPoint& scale);
    static bool Poly3Proc(const SkPoint[], SkMatrix*, const SkPoint& scale);
    static bool Poly4Proc(const SkPoint[], SkMatrix*, const SkPoint& scale);

    static void Identity_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
    static void Trans_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
    static void Scale_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
    static void ScaleTrans_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
    static void Rot_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
    static void RotTrans_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);
    static void Persp_xy(const SkMatrix&, SkScalar, SkScalar, SkPoint*);

    static const MapXYProc gMapXYProcs[];

    static void Identity_pts(const SkMatrix&, SkPoint[], const SkPoint[], int);
    static void Trans_pts(const SkMatrix&, SkPoint dst[], const SkPoint[], int);
    static void Scale_pts(const SkMatrix&, SkPoint dst[], const SkPoint[], int);
    static void ScaleTrans_pts(const SkMatrix&, SkPoint dst[], const SkPoint[],
                               int count);
    static void Persp_pts(const SkMatrix&, SkPoint dst[], const SkPoint[], int);

    static void Affine_vpts(const SkMatrix&, SkPoint dst[], const SkPoint[], int);

    static const MapPtsProc gMapPtsProcs[];

    // return the number of bytes written, whether or not buffer is null
    size_t writeToMemory(void* buffer) const;
    /**
     * Reads data from the buffer parameter
     *
     * @param buffer Memory to read from
     * @param length Amount of memory available in the buffer
     * @return number of bytes read (must be a multiple of 4) or
     *         0 if there was not enough memory available
     */
    size_t readFromMemory(const void* buffer, size_t length);

    friend class SkPerspIter;
    friend class SkMatrixPriv;
    friend class SkReader32;
    friend class SerializationTest;
};
SK_END_REQUIRE_DENSE

#endif
