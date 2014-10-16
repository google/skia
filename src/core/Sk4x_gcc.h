// It is important _not_ to put header guards here.
// This file will be intentionally included three times.

// Useful reading:
//   https://gcc.gnu.org/onlinedocs/gcc/Vector-Extensions.html

#if defined(SK4X_PREAMBLE)

#elif defined(SK4X_PRIVATE)
    typedef T Vector __attribute__((vector_size(16)));

    /*implicit*/ Sk4x(Vector vec) : fVec(vec) {}
    static inline Vector ShuffleImpl(Vector a, Vector b, int __attribute__((vector_size(16))) mask);
    template <int m, int a, int s, int k>
    static Sk4x Shuffle(const Sk4x&, const Sk4x&);

    Vector fVec;

#else  // defined(SK4X_PRIVATE)

template <typename T>
Sk4x<T>::Sk4x() { }

template <typename T>
Sk4x<T>::Sk4x(T a, T b, T c, T d) { this->set(a,b,c,d); }

template <typename T>
Sk4x<T>::Sk4x(const T vals[4]) {
    fVec = *reinterpret_cast<const Vector*>(vals);  // Should compile to moveaps or moveups.
}

template <typename T>
Sk4x<T>::Sk4x(const Sk4x<T>& other) { *this = other; }

template <typename T>
Sk4x<T>& Sk4x<T>::operator=(const Sk4x<T>& other) { fVec = other.fVec; return *this; }

template <typename T>
void Sk4x<T>::set(T a, T b, T c, T d) {
    Vector v = { a, b, c, d };
    fVec = v;
}

template <typename T>
void Sk4x<T>::store(T vals[4]) const {
    SkASSERT(SkIsAlign16((uintptr_t)vals));
    *reinterpret_cast<Vector*>(vals) = fVec;
}

template <typename T>
template <typename Dst> Dst Sk4x<T>::reinterpret() const {
    return Dst((typename Dst::Vector)fVec);
}

template <typename T>
template <typename Dst> Dst Sk4x<T>::cast() const {
    return Dst(fVec[0], fVec[1], fVec[2], fVec[3]);
}

template <typename T>
bool Sk4x<T>::allTrue() const { return fVec[0] & fVec[1] & fVec[2] & fVec[3]; }
template <typename T>
bool Sk4x<T>::anyTrue() const { return fVec[0] | fVec[1] | fVec[2] | fVec[3]; }

template <typename T> Sk4x<T> Sk4x<T>::bitNot() const { return Sk4i(~fVec); }

template <typename T> Sk4x<T> Sk4x<T>::bitAnd(const Sk4x& other) const { return fVec & other.fVec; }
template <typename T> Sk4x<T> Sk4x<T>::bitOr (const Sk4x& other) const { return fVec | other.fVec; }

template <typename T>
Sk4i Sk4x<T>::           equal(const Sk4x<T>& other) const { return fVec == other.fVec; }
template <typename T>
Sk4i Sk4x<T>::        notEqual(const Sk4x<T>& other) const { return fVec != other.fVec; }
template <typename T>
Sk4i Sk4x<T>::        lessThan(const Sk4x<T>& other) const { return fVec  < other.fVec; }
template <typename T>
Sk4i Sk4x<T>::     greaterThan(const Sk4x<T>& other) const { return fVec  > other.fVec; }
template <typename T>
Sk4i Sk4x<T>::   lessThanEqual(const Sk4x<T>& other) const { return fVec <= other.fVec; }
template <typename T>
Sk4i Sk4x<T>::greaterThanEqual(const Sk4x<T>& other) const { return fVec >= other.fVec; }

template <typename T>
Sk4x<T> Sk4x<T>::     add(const Sk4x<T>& other) const { return fVec + other.fVec; }
template <typename T>
Sk4x<T> Sk4x<T>::subtract(const Sk4x<T>& other) const { return fVec - other.fVec; }
template <typename T>
Sk4x<T> Sk4x<T>::multiply(const Sk4x<T>& other) const { return fVec * other.fVec; }
template <typename T>
Sk4x<T> Sk4x<T>::  divide(const Sk4x<T>& other) const { return fVec / other.fVec; }

template <typename T>
Sk4x<T> Sk4x<T>::Min(const Sk4x<T>& a, const Sk4x<T>& b) {
    return a.fVec < b.fVec ? a.fVec : b.fVec;  // This makes great SSE code (1 minps op)...
}

template <typename T>
Sk4x<T> Sk4x<T>::Max(const Sk4x<T>& a, const Sk4x<T>& b) {
    return a.fVec < b.fVec ? b.fVec : a.fVec;  // ...but this doesn't look so good (7 ops?).
}

// GCC 4.8 has a bug that leads it to segfault when presented with the obvious code for Shuffle:
//   Sk4i::Vector mask = { m,a,s,k };
//   return __builtin_shuffle(x.fVec, y.fVec, mask);
//
// This roundabout implementation via ShuffleImpl works around that bug,
//   https://gcc.gnu.org/bugzilla/show_bug.cgi?id=57509

template <>
inline Sk4i::Vector Sk4i::ShuffleImpl(Sk4i::Vector x, Sk4i::Vector y, Sk4i::Vector mask) {
    return __builtin_shuffle(x,y, mask);
}

template <>
inline Sk4f::Vector Sk4f::ShuffleImpl(Sk4f::Vector x, Sk4f::Vector y, Sk4i::Vector mask) {
    return __builtin_shuffle(x,y, mask);
}

template <typename T>
template <int m, int a, int s, int k>
Sk4x<T> Sk4x<T>::Shuffle(const Sk4x<T>& x, const Sk4x<T>& y) {
    Sk4i::Vector mask = { m,a,s,k };
    return ShuffleImpl(x.fVec, y.fVec, mask);
}

template <typename T>
Sk4x<T> Sk4x<T>::zwxy() const { return Shuffle<2,3,0,1>(*this, *this); }

template <typename T>
Sk4x<T> Sk4x<T>::XYAB(const Sk4x& xyzw, const Sk4x& abcd) { return Shuffle<0,1,4,5>(xyzw, abcd); }

template <typename T>
Sk4x<T> Sk4x<T>::ZWCD(const Sk4x& xyzw, const Sk4x& abcd) { return Shuffle<2,3,6,7>(xyzw, abcd); }

#endif // defined(SK4X_PRIVATE)
