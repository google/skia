#ifndef SkTArray_DEFINED
#define SkTArray_DEFINED

template <typename T, bool MEM_MOVE = false> class SkTArray {
public:
    SkTArray();
    explicit SkTArray(int reserveCount);
    T* begin();
    T* end();
    T& operator[] (int i);
    const T& operator[] (int i) const;
    int count() const;
    T& push_back();
    T& push_back(const T& t);
    void reset();
};

template <int N, typename T, bool MEM_MOVE= false>
class SkSTArray : public SkTArray<T, MEM_MOVE> {
private:
    typedef SkTArray<T, MEM_MOVE> INHERITED;
public:
    SkSTArray();
    explicit SkSTArray(int reserveCount)
        : INHERITED(reserveCount) {
    }
};

#endif
