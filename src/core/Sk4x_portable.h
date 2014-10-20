// It is important _not_ to put header guards here.
// This file will be intentionally included three times.

#if defined(SK4X_PREAMBLE)

#elif defined(SK4X_PRIVATE)
    typedef T Vector[4];

    Vector fVec;

    template <int m, int a, int s, int k>
    static Sk4x Shuffle(const Sk4x&, const Sk4x&);

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
Sk4x<T>& Sk4x<T>::operator=(const Sk4x<T>& other) {
    this->set(other.fVec[0], other.fVec[1], other.fVec[2], other.fVec[3]);
    return *this;
}

template <typename T>
void Sk4x<T>::set(T a, T b, T c, T d) {
    fVec[0] = a;
    fVec[1] = b;
    fVec[2] = c;
    fVec[3] = d;
}

template <typename T>
void Sk4x<T>::store(T vals[4]) const {
    vals[0] = fVec[0];
    vals[1] = fVec[1];
    vals[2] = fVec[2];
    vals[3] = fVec[3];
}

template <typename T>
template <typename Dst> Dst Sk4x<T>::reinterpret() const {
    return Dst(reinterpret_cast<const typename Dst::Vector*>(fVec));
}

template <typename T>
template <typename Dst> Dst Sk4x<T>::cast() const {
    return Dst(fVec[0], fVec[1], fVec[2], fVec[3]);
}

template <typename T>
bool Sk4x<T>::allTrue() const { return fVec[0] & fVec[1] & fVec[2] & fVec[3]; }
template <typename T>
bool Sk4x<T>::anyTrue() const { return fVec[0] | fVec[1] | fVec[2] | fVec[3]; }

template <typename T>
Sk4x<T> Sk4x<T>::bitNot() const { return Sk4x(~fVec[0], ~fVec[1], ~fVec[2], ~fVec[3]); }

#define BINOP(op) fVec[0] op other.fVec[0], \
                  fVec[1] op other.fVec[1], \
                  fVec[2] op other.fVec[2], \
                  fVec[3] op other.fVec[3]

template <typename T> Sk4x<T> Sk4x<T>::bitAnd(const Sk4x& other) const { return Sk4x(BINOP(&)); }
template <typename T> Sk4x<T> Sk4x<T>::bitOr (const Sk4x& other) const { return Sk4x(BINOP(|)); }

template <typename T>
Sk4i Sk4x<T>::           equal(const Sk4x<T>& other) const { return Sk4i(BINOP(==)); }
template <typename T>
Sk4i Sk4x<T>::        notEqual(const Sk4x<T>& other) const { return Sk4i(BINOP(!=)); }
template <typename T>
Sk4i Sk4x<T>::        lessThan(const Sk4x<T>& other) const { return Sk4i(BINOP( <)); }
template <typename T>
Sk4i Sk4x<T>::     greaterThan(const Sk4x<T>& other) const { return Sk4i(BINOP( >)); }
template <typename T>
Sk4i Sk4x<T>::   lessThanEqual(const Sk4x<T>& other) const { return Sk4i(BINOP(<=)); }
template <typename T>
Sk4i Sk4x<T>::greaterThanEqual(const Sk4x<T>& other) const { return Sk4i(BINOP(>=)); }

template <typename T>
Sk4x<T> Sk4x<T>::     add(const Sk4x<T>& other) const { return Sk4x(BINOP(+)); }
template <typename T>
Sk4x<T> Sk4x<T>::subtract(const Sk4x<T>& other) const { return Sk4x(BINOP(-)); }
template <typename T>
Sk4x<T> Sk4x<T>::multiply(const Sk4x<T>& other) const { return Sk4x(BINOP(*)); }
template <typename T>
Sk4x<T> Sk4x<T>::  divide(const Sk4x<T>& other) const { return Sk4x(BINOP(/)); }

#undef BINOP

template <typename T>
Sk4x<T> Sk4x<T>::Min(const Sk4x<T>& a, const Sk4x<T>& b) {
    return Sk4x(SkTMin(a.fVec[0], b.fVec[0]),
                SkTMin(a.fVec[1], b.fVec[1]),
                SkTMin(a.fVec[2], b.fVec[2]),
                SkTMin(a.fVec[3], b.fVec[3]));
}

template <typename T>
Sk4x<T> Sk4x<T>::Max(const Sk4x<T>& a, const Sk4x<T>& b) {
    return Sk4x(SkTMax(a.fVec[0], b.fVec[0]),
                SkTMax(a.fVec[1], b.fVec[1]),
                SkTMax(a.fVec[2], b.fVec[2]),
                SkTMax(a.fVec[3], b.fVec[3]));
}

template <typename T>
template <int m, int a, int s, int k>
Sk4x<T> Sk4x<T>::Shuffle(const Sk4x<T>& x, const Sk4x<T>& y) {
    return Sk4x(m < 4 ? x.fVec[m] : y.fVec[m-4],
                a < 4 ? x.fVec[a] : y.fVec[a-4],
                s < 4 ? x.fVec[s] : y.fVec[s-4],
                k < 4 ? x.fVec[k] : y.fVec[k-4]);
}

template <typename T>
Sk4x<T> Sk4x<T>::zwxy() const { return Shuffle<2,3,0,1>(*this, *this); }

template <typename T>
Sk4x<T> Sk4x<T>::XYAB(const Sk4x& xyzw, const Sk4x& abcd) { return Shuffle<0,1,4,5>(xyzw, abcd); }

template <typename T>
Sk4x<T> Sk4x<T>::ZWCD(const Sk4x& xyzw, const Sk4x& abcd) { return Shuffle<2,3,6,7>(xyzw, abcd); }

#endif // defined(SK4X_PRIVATE)
