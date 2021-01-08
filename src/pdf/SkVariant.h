// Copyright 2021 Hal W Canary III
// Use of this source code is governed by a BSD-style license that can be found in the LICENSE file.

#ifndef SkVariant_DEFINED
#define SkVariant_DEFINED

static_assert(__cplusplus >= 201703L);

#include <new>
#include <type_traits>

namespace skstd {

namespace variant_details {
template <unsigned I, typename F, typename... REST> struct Helper {
    static_assert(I > 0, "");
    using Next = Helper<I + 1, REST...>;
    template <typename U>
    static unsigned constexpr kIndex = std::is_same<U, F>::value ? I : Next::template kIndex<U>;
    static bool constexpr kAllCopyConstructable =
            std::is_copy_constructible_v<F> && Next::kAllCopyConstructable;
    template <typename FN> static auto Visit(unsigned id, void* data, FN f) {
        return id == I ? f(*reinterpret_cast<F*>(data)) : Next::Visit(id, data, std::move(f));
    }
    template <typename FN> static auto CVisit(unsigned id, const void* data, FN f) {
        return id == I ? f(*reinterpret_cast<const F*>(data))
                       : Next::CVisit(id, data, std::move(f));
    }
};

template <unsigned I, typename F> struct Helper<I, F> {
    static_assert(I > 0, "");
    template <typename U> static unsigned constexpr kIndex = std::is_same<U, F>::value ? I : 0;
    static bool constexpr kAllCopyConstructable = std::is_copy_constructible_v<F>;
    template <typename FN> static auto Visit(unsigned id, void* data, FN f) {
        if (id == I) {
            return f(*reinterpret_cast<F*>(data));
        }
    }
    template <typename FN> static auto CVisit(unsigned id, const void* data, FN f) {
        if (id == I) {
            return f(*reinterpret_cast<const F*>(data));
        }
    }
};
}  // namespace variant_details

template <typename... Types> class variant {
    typename std::aligned_union<0, Types...>::type fStorage;
    unsigned fTypeIndex = 0;

    using Helper = variant_details::Helper<1, Types...>;
    template <typename T> static unsigned constexpr kIndex = Helper::template kIndex<T>;
    static constexpr bool kCopyConstructable = Helper::kAllCopyConstructable;

public:
    template <typename T> variant(T&& value) : fTypeIndex(kIndex<std::decay_t<T>>) {
        using U = std::decay_t<T>;
        static_assert(kIndex<U> != 0, "type not in union");
        new ((U*)&fStorage) U(std::move(value));
    }

    ~variant() {
        this->visit([](auto& o) {
            using T = std::decay_t<decltype(o)>;
            o.~T();
        });
    }

    variant(variant&& that) : fTypeIndex(that.fTypeIndex) {
        this->visit([&that](auto& o) {
            using T = std::decay_t<decltype(o)>;
            new (&o) T(std::move(*reinterpret_cast<T*>(&that.fStorage)));
        });
    }

    variant& operator=(variant&& that) {
        if (this != &that) {
            this->~variant();
            new (this) variant(std::move(that));
        }
        return *this;
    }

    template <typename> variant(const variant& that) : fTypeIndex(that.fTypeIndex) {
        static_assert(kCopyConstructable);
        this->visit([&that](auto& o) {
            using T = std::decay_t<decltype(o)>;
            new (&o) T(*reinterpret_cast<const T*>(&that.fStorage));
        });
    }

    template <typename> variant& operator=(const variant& that) {
        static_assert(kCopyConstructable);
        if (this != &that) {
            this->~variant();
            new (this) variant(that);
        }
        return *this;
    }

    template <typename T> bool holds_alternative() const { return kIndex<T> == fTypeIndex; }

    template <typename Fn> auto visit(Fn f) {
        return Helper::Visit(fTypeIndex, &fStorage, std::move(f));
    }
    template <typename Fn> auto visit(Fn f) const {
        return Helper::CVisit(fTypeIndex, &fStorage, std::move(f));
    }

    template <typename T> T* get_if() {
        static_assert(kIndex<T> != 0, "type not in union");
        return kIndex<T> == fTypeIndex ? (T*)(&fStorage) : nullptr;
    }

    template <typename T> const T* get_if() const {
        static_assert(kIndex<T> != 0, "type not in union");
        return kIndex<T> == fTypeIndex ? reinterpret_cast<const T*>(&fStorage) : nullptr;
    }
};

template <typename T, typename... Types>
bool holds_alternative(const skstd::variant<Types...>& value) {
    return value.template holds_alternative<T>();
}

template <typename Fn, typename... Types>
auto visit(Fn&& fn, const skstd::variant<Types...>& value) {
    return value.visit(std::move(fn));
}

template <typename T, typename... Types> T* get_if(skstd::variant<Types...>* pv) {
    return pv ? pv->template get_if<T>() : nullptr;
}

template <typename T, typename... Types> const T* get_if(const skstd::variant<Types...>* pv) {
    return pv ? pv->template get_if<T>() : nullptr;
}

}  // namespace skstd
#endif  // SkVariant_DEFINED
