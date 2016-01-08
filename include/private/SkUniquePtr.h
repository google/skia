/*
 * Copyright 2015 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SkUniquePtr_DEFINED
#define SkUniquePtr_DEFINED

#include "SkTLogic.h"
#include <cstddef>
#include <utility>

namespace skstd {

template <typename T> struct default_delete {
    /*constexpr*/ default_delete() /*noexcept*/ = default;

    template <typename U, typename = enable_if_t<is_convertible<U*, T*>::value>>
    default_delete(const default_delete<U>&) /*noexcept*/ {}

    void operator()(T* obj) const {
        static_assert(sizeof(T) > 0, "Deleting pointer to incomplete type!");
        delete obj;
    }
};
template <typename T> struct default_delete<T[]> {
    /*constexpr*/ default_delete() /*noexcept*/ = default;

    void operator()(T* obj) const {
        static_assert(sizeof(T) > 0, "Deleting pointer to incomplete type!");
        delete [] obj;
    }
};

template <typename T, typename D = default_delete<T>> class unique_ptr {
    // remove_reference_t<D>::pointer if that type exists, otherwise T*.
    struct pointer_type_detector {
        template <typename U> static typename U::pointer detector(typename U::pointer*);
        template <typename U> static T* detector(...);
        using type = decltype(detector<remove_reference_t<D>>(0));
    };

public:
    using pointer = typename pointer_type_detector::type;
    using element_type = T;
    using deleter_type = D;

private:
    template <typename B, bool = std::is_empty<B>::value /*&& !is_final<B>::value*/>
    struct compressed_base : private B {
        /*constexpr*/ compressed_base() : B() {}
        /*constexpr*/ compressed_base(const B& b) : B(b) {}
        /*constexpr*/ compressed_base(B&& b) : B(std::move(b)) {}
        /*constexpr*/ B& get() /*noexcept*/ { return *this; }
        /*constexpr*/ B const& get() const /*noexcept*/ { return *this; }
        void swap(compressed_base&) /*noexcept*/ { }
    };

    template <typename B> struct compressed_base<B, false> {
        B fb;
        /*constexpr*/ compressed_base() : B() {}
        /*constexpr*/ compressed_base(const B& b) : fb(b) {}
        /*constexpr*/ compressed_base(B&& b) : fb(std::move(b)) {}
        /*constexpr*/ B& get() /*noexcept*/ { return fb; }
        /*constexpr*/ B const& get() const /*noexcept*/ { return fb; }
        void swap(compressed_base& that) /*noexcept*/ { SkTSwap(fb, that.fB); }
    };

    struct compressed_data : private compressed_base<deleter_type> {
        pointer fPtr;
        /*constexpr*/ compressed_data() : compressed_base<deleter_type>(), fPtr() {}
        /*constexpr*/ compressed_data(const pointer& ptr, const deleter_type& d)
            : compressed_base<deleter_type>(d), fPtr(ptr) {}
        template <typename U1, typename U2, typename = enable_if_t<
            is_convertible<U1, pointer>::value && is_convertible<U2, deleter_type>::value
        >> /*constexpr*/ compressed_data(U1&& ptr, U2&& d)
            : compressed_base<deleter_type>(std::forward<U2>(d)), fPtr(std::forward<U1>(ptr)) {}
        /*constexpr*/ pointer& getPointer() /*noexcept*/ { return fPtr; }
        /*constexpr*/ pointer const& getPointer() const /*noexcept*/ { return fPtr; }
        /*constexpr*/ deleter_type& getDeleter() /*noexcept*/ {
            return compressed_base<deleter_type>::get();
        }
        /*constexpr*/ deleter_type const& getDeleter() const /*noexcept*/ {
            return compressed_base<deleter_type>::get();
        }
        void swap(compressed_data& that) /*noexcept*/ {
            compressed_base<deleter_type>::swap(static_cast<compressed_base<deleter_type>>(that));
            SkTSwap(fPtr, that.fPtr);
        }
    };
    compressed_data data;

public:
    /*constexpr*/ unique_ptr() /*noexcept*/ : data() {
        static_assert(!std::is_pointer<deleter_type>::value, "Deleter nullptr function pointer!");
    }

    /*constexpr*/ unique_ptr(std::nullptr_t) /*noexcept*/ : unique_ptr() { }

    explicit unique_ptr(pointer ptr) /*noexcept*/ : data(ptr, deleter_type()) {
        static_assert(!std::is_pointer<deleter_type>::value, "Deleter nullptr function pointer!");
    }

    unique_ptr(pointer ptr,
               conditional_t<std::is_reference<deleter_type>::value,
                             deleter_type, const deleter_type&> d)
    /*noexcept*/ : data(ptr, d)
    {}

    unique_ptr(pointer ptr, remove_reference_t<deleter_type>&& d) /*noexcept*/
        : data(std::move(ptr), std::move(d))
    {
        static_assert(!std::is_reference<deleter_type>::value,
            "Binding an rvalue reference deleter as an lvalue reference deleter is not allowed.");
    }


    unique_ptr(unique_ptr&& that) /*noexcept*/
        : data(that.release(), std::forward<deleter_type>(that.get_deleter()))
    {}

    template <typename U, typename ThatD, typename = enable_if_t<
        is_convertible<typename unique_ptr<U, ThatD>::pointer, pointer>::value &&
        !std::is_array<U>::value &&
        conditional_t<std::is_reference<D>::value,
                      std::is_same<ThatD, D>,
                      is_convertible<ThatD, D>>::value>>
    unique_ptr(unique_ptr<U, ThatD>&& that) /*noexcept*/
        : data(that.release(), std::forward<ThatD>(that.get_deleter()))
    {}

    ~unique_ptr() /*noexcept*/ {
        pointer& ptr = data.getPointer();
        if (ptr != nullptr) {
            get_deleter()(ptr);
        }
        ptr = pointer();
    }

    unique_ptr& operator=(unique_ptr&& that) /*noexcept*/ {
        reset(that.release());
        get_deleter() = std::forward<deleter_type>(that.get_deleter());
        return *this;
    }

    template <typename U, typename ThatD> enable_if_t<
        is_convertible<typename unique_ptr<U, ThatD>::pointer, pointer>::value &&
        !std::is_array<U>::value,
    unique_ptr&> operator=(unique_ptr<U, ThatD>&& that) /*noexcept*/ {
        reset(that.release());
        get_deleter() = std::forward<ThatD>(that.get_deleter());
        return *this;
    }

    unique_ptr& operator=(std::nullptr_t) /*noexcept*/ {
        reset();
        return *this;
    }

    add_lvalue_reference_t<element_type> operator*() const {
        SkASSERT(get() != pointer());
        return *get();
    }

    pointer operator->() const /*noexcept*/ {
        SkASSERT(get() != pointer());
        return get();
    }

    pointer get() const /*noexcept*/ {
        return data.getPointer();
    }

    deleter_type& get_deleter() /*noexcept*/ {
        return data.getDeleter();
    }

    const deleter_type& get_deleter() const /*noexcept*/ {
        return data.getDeleter();
    }

    //explicit operator bool() const noexcept {
    bool is_attached() const /*noexcept*/ {
        return get() == pointer() ? false : true;
    }

    pointer release() /*noexcept*/ {
        pointer ptr = get();
        data.getPointer() = pointer();
        return ptr;
    }

    void reset(pointer ptr = pointer()) /*noexcept*/ {
        SkTSwap(data.getPointer(), ptr);
        if (ptr != pointer()) {
            get_deleter()(ptr);
        }
    }

    void swap(unique_ptr& that) /*noexcept*/ {
        SkTSwap(data, that.data);
    }

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;
};

template <typename T, typename D> class unique_ptr<T[], D> {
    // remove_reference_t<D>::pointer if that type exists, otherwise T*.
    struct pointer_type_detector {
        template <typename U> static typename U::pointer detector(typename U::pointer*);
        template <typename U> static T* detector(...);
        using type = decltype(detector<remove_reference_t<D>>(0));
    };

public:
    using pointer = typename pointer_type_detector::type;
    using element_type = T;
    using deleter_type = D;

private:
    template <typename B, bool = std::is_empty<B>::value /*&& !is_final<B>::value*/>
    struct compressed_base : private B {
        /*constexpr*/ compressed_base() : B() {}
        /*constexpr*/ compressed_base(const B& b) : B(b) {}
        /*constexpr*/ compressed_base(B&& b) : B(std::move(b)) {}
        /*constexpr*/ B& get() /*noexcept*/ { return *this; }
        /*constexpr*/ B const& get() const /*noexcept*/ { return *this; }
        void swap(compressed_base&) /*noexcept*/ { }
    };

    template <typename B> struct compressed_base<B, false> {
        B fb;
        /*constexpr*/ compressed_base() : B() {}
        /*constexpr*/ compressed_base(const B& b) : fb(b) {}
        /*constexpr*/ compressed_base(B&& b) : fb(std::move(b)) {}
        /*constexpr*/ B& get() /*noexcept*/ { return fb; }
        /*constexpr*/ B const& get() const /*noexcept*/ { return fb; }
        void swap(compressed_base& that) /*noexcept*/ { SkTSwap(fb, that.fB); }
    };

    struct compressed_data : private compressed_base<deleter_type> {
        pointer fPtr;
        /*constexpr*/ compressed_data() : compressed_base<deleter_type>(), fPtr() {}
        /*constexpr*/ compressed_data(const pointer& ptr, const deleter_type& d)
            : compressed_base<deleter_type>(d), fPtr(ptr) {}
        template <typename U1, typename U2, typename = enable_if_t<
            is_convertible<U1, pointer>::value && is_convertible<U2, deleter_type>::value
        >> /*constexpr*/ compressed_data(U1&& ptr, U2&& d)
            : compressed_base<deleter_type>(std::forward<U2>(d)), fPtr(std::forward<U1>(ptr)) {}
        /*constexpr*/ pointer& getPointer() /*noexcept*/ { return fPtr; }
        /*constexpr*/ pointer const& getPointer() const /*noexcept*/ { return fPtr; }
        /*constexpr*/ deleter_type& getDeleter() /*noexcept*/ {
            return compressed_base<deleter_type>::get();
        }
        /*constexpr*/ deleter_type const& getDeleter() const /*noexcept*/ {
            return compressed_base<deleter_type>::get();
        }
        void swap(compressed_data& that) /*noexcept*/ {
            compressed_base<deleter_type>::swap(static_cast<compressed_base<deleter_type>>(that));
            SkTSwap(fPtr, that.fPtr);
        }
    };
    compressed_data data;

public:
    /*constexpr*/ unique_ptr() /*noexcept*/ : data() {
        static_assert(!std::is_pointer<deleter_type>::value, "Deleter nullptr function pointer!");
    }

    /*constexpr*/ unique_ptr(std::nullptr_t) /*noexcept*/ : unique_ptr() { }

    explicit unique_ptr(pointer ptr) /*noexcept*/ : data(ptr, deleter_type()) {
        static_assert(!std::is_pointer<deleter_type>::value, "Deleter nullptr function pointer!");
    }

    unique_ptr(pointer ptr,
               conditional_t<std::is_reference<deleter_type>::value,
                             deleter_type, const deleter_type&> d)
    /*noexcept*/ : data(ptr, d)
    {}

    unique_ptr(pointer ptr, remove_reference_t<deleter_type>&& d) /*noexcept*/
        : data(std::move(ptr), std::move(d))
    {
        static_assert(!std::is_reference<deleter_type>::value,
            "Binding an rvalue reference deleter as an lvalue reference deleter is not allowed.");
    }

    unique_ptr(unique_ptr&& that) /*noexcept*/
        : data(that.release(), std::forward<deleter_type>(that.get_deleter()))
    {}

    ~unique_ptr() {
        pointer& ptr = data.getPointer();
        if (ptr != nullptr) {
          get_deleter()(ptr);
        }
        ptr = pointer();
    }

    unique_ptr& operator=(unique_ptr&& that) /*noexcept*/ {
        reset(that.release());
        get_deleter() = std::forward<deleter_type>(that.get_deleter());
        return *this;
    }

    unique_ptr& operator=(std::nullptr_t) /*noexcept*/ {
        reset();
        return *this;
    }

    add_lvalue_reference_t<element_type> operator[](size_t i) const {
        SkASSERT(get() != pointer());
        return get()[i];
    }

    pointer get() const /*noexcept*/ {
        return data.getPointer();
    }

    deleter_type& get_deleter() /*noexcept*/ {
        return data.getDeleter();
    }

    const deleter_type& get_deleter() const /*noexcept*/ {
        return data.getDeleter();
    }

    //explicit operator bool() const noexcept {
    bool is_attached() const /*noexcept*/ {
        return get() == pointer() ? false : true;
    }

    pointer release() /*noexcept*/ {
        pointer ptr = get();
        data.getPointer() = pointer();
        return ptr;
    }

    void reset(pointer ptr = pointer()) /*noexcept*/ {
        SkTSwap(data.getPointer(), ptr);
        if (ptr != pointer()) {
            get_deleter()(ptr);
        }
    }

    template <typename U> void reset(U*) = delete;

    void swap(unique_ptr& that) /*noexcept*/ {
        data.swap(that.data);
    }

    unique_ptr(const unique_ptr&) = delete;
    unique_ptr& operator=(const unique_ptr&) = delete;
};

template <typename T, typename D>
inline void swap(unique_ptr<T, D>& a, unique_ptr<T, D>& b) /*noexcept*/ {
    a.swap(b);
}

template <typename T, typename D, typename U, typename ThatD>
inline bool operator==(const unique_ptr<T, D>& a, const unique_ptr<U, ThatD>& b) {
    return a.get() == b.get();
}

template <typename T, typename D>
inline bool operator==(const unique_ptr<T, D>& a, std::nullptr_t) /*noexcept*/ {
    //return !a;
    return !a.is_attached();
}

template <typename T, typename D>
inline bool operator==(std::nullptr_t, const unique_ptr<T, D>& b) /*noexcept*/ {
    //return !b;
    return !b.is_attached();
}

template <typename T, typename D, typename U, typename ThatD>
inline bool operator!=(const unique_ptr<T, D>& a, const unique_ptr<U, ThatD>& b) {
    return a.get() != b.get();
}

template <typename T, typename D>
inline bool operator!=(const unique_ptr<T, D>& a, std::nullptr_t) /*noexcept*/ {
    //return (bool)a;
    return a.is_attached();
}

template <typename T, typename D>
inline bool operator!=(std::nullptr_t, const unique_ptr<T, D>& b) /*noexcept*/ {
    //return (bool)b;
    return b.is_attached();
}

}  // namespace skstd

#endif
