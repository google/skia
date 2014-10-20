// It is important _not_ to put header guards here.
// This file will be intentionally included three times.

// Useful reading:
//   http://clang.llvm.org/docs/LanguageExtensions.html#vectors-and-extended-vectors

#if defined(SK4X_PREAMBLE)

#elif defined(SK4X_PRIVATE)
    typedef T Vector __attribute__((ext_vector_type(4)));

    /*implicit*/ Sk4x(Vector vec) : fVec(vec) {}

    template <int m, int a, int s, int k>
    static Sk4x Shuffle(const Sk4x&, const Sk4x&);

    Vector fVec;

#else  // defined(SK4X_PRIVATE)

template <typename T>
Sk4x<T>::Sk4x() { }

template <typename T>
Sk4x<T>::Sk4x(T a, T b, T c, T d) { this->set(a,b,c,d); }

template <typename T>
Sk4x<T>::Sk4x(const T vals[4]) { this->set(vals[0], vals[1], vals[2], vals[3]); }

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
    #if __has_builtin(__builtin_convertvector)
        return Dst(__builtin_convertvector(fVec, typename Dst::Vector));
    #else
        return Dst(fVec[0], fVec[1], fVec[2], fVec[3]);
    #endif
}

template <typename T>
bool Sk4x<T>::allTrue() const { return fVec[0] & fVec[1] & fVec[2] & fVec[3]; }
template <typename T>
bool Sk4x<T>::anyTrue() const { return fVec[0] | fVec[1] | fVec[2] | fVec[3]; }

template <typename T> Sk4x<T> Sk4x<T>::bitNot() const { return ~fVec; }

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
    Sk4i less(a.lessThan(b));
    Sk4i val = a.reinterpret<Sk4i>().bitAnd(less).bitOr(
               b.reinterpret<Sk4i>().bitAnd(less.bitNot()));
    return val.reinterpret<Sk4x>();
}

template <typename T>
Sk4x<T> Sk4x<T>::Max(const Sk4x<T>& a, const Sk4x<T>& b) {
    Sk4i less(a.lessThan(b));
    Sk4i val = b.reinterpret<Sk4i>().bitAnd(less).bitOr(
               a.reinterpret<Sk4i>().bitAnd(less.bitNot()));
    return val.reinterpret<Sk4x>();
}

template <typename T>
template <int m, int a, int s, int k>
Sk4x<T> Sk4x<T>::Shuffle(const Sk4x<T>& x, const Sk4x<T>& y) {
    return __builtin_shufflevector(x.fVec, y.fVec, m,a,s,k);
}

template <typename T>
Sk4x<T> Sk4x<T>::zwxy() const { return fVec.zwxy; }

template <typename T>
Sk4x<T> Sk4x<T>::XYAB(const Sk4x& xyzw, const Sk4x& abcd) { return Shuffle<0,1,4,5>(xyzw, abcd); }

template <typename T>
Sk4x<T> Sk4x<T>::ZWCD(const Sk4x& xyzw, const Sk4x& abcd) { return Shuffle<2,3,6,7>(xyzw, abcd); }

#endif // defined(SK4X_PRIVATE)
