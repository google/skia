// Copyright 2019 Google LLC.
#ifndef TypedUnion_DEFINED
#define TypedUnion_DEFINED

template <class T>
class SkStorageFor {
public:
    const T& get() const { return *reinterpret_cast<const T*>(&fStore); }
    T& get() { return *reinterpret_cast<T*>(&fStore); }
    // Up to caller to keep track of status.
    template<class... Args> void init(Args&&... args) {
        new (&this->get()) T(std::forward<Args>(args)...);
    }
    void destroy() { this->get().~T(); }
private:
    typename std::aligned_storage<sizeof(T), alignof(T)>::type fStore;
};

struct A {/* complete def*/};
struct B {/* complete def */};

template <typename A, typename B>
class TypedUnion {
public:
      enum Type { kNone, kFirstType, kSecondType } fType;

    TypedUnion() : fType(kNone) {}

    TypedUnion(TypedUnion&& o) : fType(o.fType), fUnion(o.fUnion) { o.fType = kNone; }

    TypedUnion& operator=(TypedUnion&& o) {
        if (this != &o) {
            this->~TypedUnion();
            new (this) TypedUnion(std::move(o));
        }
        return *this;
    }

    TypedUnion(const TypedUnion& other) {
        switch (fType) {
            case kFirstType:  fUnion.fA.destroy(); break;
            case kSecondType: fUnion.fB.destroy(); break;
            default: break;
        }
        fType = other.fType;
        switch (fType) {
            case kFirstType: fUnion.fA.init(*other.getFirst()); break;
            case kSecondType: fUnion.fB.init(*other.getSecond()); break;
            default: break;
        };
    }

    TypedUnion& operator=(const TypedUnion& other) {
        fType = other.fType;
        switch (fType) {
            case kFirstType: fUnion.fA.init(*other.getFirst()); break;
            case kSecondType: fUnion.fB.init(*other.getSecond()); break;
            default: break;
        };
        return *this;
    }

    ~TypedUnion() {
        switch (fType) {
            case kFirstType:  fUnion.fA.destroy(); break;
            case kSecondType: fUnion.fB.destroy(); break;
            default: break;
        }
    }

    static TypedUnion Make(A value) {
        TypedUnion s(kFirstType);
        s.fUnion.fA.init(std::move(value));
        return s;
    }

    static TypedUnion Make(B value) {
        TypedUnion s(kSecondType);
        s.fUnion.fB.init(std::move(value));
        return s;
    }

    Type type() const { return fType; }

    bool operator==(const TypedUnion& other) const {
        if (fType != other.fType) return false;
        switch (fType) {
            case kFirstType:  return fUnion.fA.get() == other.fUnion.fA.get();
            case kSecondType: return fUnion.fB.get() == other.fUnion.fB.get();
            default: return true;
        }
    }

    bool operator==(const A& other) const {
        if (fType != kFirstType) return false;
        return fUnion.fA.get() == other;
    }

    bool operator==(const B& other) const {
        if (fType != kSecondType) return false;
        return fUnion.fB.get() == other;
    }

    //A* getFirst() { return fType == kFirstType ? &fUnion.fA : nullptr; }
    //B* getSecond() { return fType == kSecondType ? &fUnion.fB : nullptr; }

    const A* getFirst() const { return fType == kFirstType ? &fUnion.fA.get() : nullptr; }
    const B* getSecond() const { return fType == kSecondType ? &fUnion.fB.get() : nullptr; }

private:

    union {
        SkStorageFor<A> fA;
        SkStorageFor<B> fB;
    } fUnion;

    TypedUnion(Type t) : fType(t) {}
};

#endif // TypedUnion_DEFINED
