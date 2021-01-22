/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_NODEARRAYWRAPPER
#define SKSL_NODEARRAYWRAPPER

#include "include/private/SkTArray.h"

namespace SkSL {

class IRNode;

// Wraps an SkTArray<std::unique_ptr<Base>> and presents it as a collection of <T>
// Once the rearchitecture is complete, Base will always be IRNode and thus we can remove that
// parameter, but right now we have to worry about wrapping both ExpressionArray and StatementArray
template<typename T, typename Base>
class NodeArrayWrapper {
public:
    class iterator {
    public:
        T& operator*() {
            return static_cast<T&>(**fBase);
        }

        T* operator->() {
            return static_cast<T*>(fBase->get());
        }

        iterator& operator++() {
            ++fBase;
            return *this;
        }

        bool operator==(const iterator& other) const {
            return fBase == other.fBase;
        }

        bool operator!=(const iterator& other) const {
            return fBase != other.fBase;
        }

    private:
        iterator(const std::unique_ptr<Base>* base)
            : fBase(base) {}

        const std::unique_ptr<Base>* fBase;

        friend class NodeArrayWrapper;
    };

    class const_iterator {
    public:
        const T& operator*() {
            return static_cast<const T&>(**fBase);
        }

        const T* operator->() {
            return static_cast<const T*>(fBase->get());
        }

        const_iterator& operator++() {
            ++fBase;
            return *this;
        }

        bool operator==(const const_iterator& other) const {
            return fBase == other.fBase;
        }

        bool operator!=(const const_iterator& other) const {
            return fBase != other.fBase;
        }

    private:
        const_iterator(const std::unique_ptr<Base>* base)
            : fBase(base) {}

        const std::unique_ptr<Base>* fBase;

        friend class NodeArrayWrapper;
    };

    NodeArrayWrapper(SkTArray<std::unique_ptr<Base>>* contents)
        : fContents(contents) {}

    NodeArrayWrapper(const NodeArrayWrapper& other)
        : fContents(other.fContents) {}

    NodeArrayWrapper& operator=(const NodeArrayWrapper& other) {
        fContents = other.fContents;
    }

    void reset() {
        fContents->reset();
    }

    void reserve_back(int n) {
        fContents->reserve_back(n);
    }

    int count() const {
        return fContents->count();
    }

    bool empty() const {
        return fContents->empty();
    }


    T& push_back(T* t) {
        return static_cast<T&>(*fContents->emplace_back(t));
    }

    template<class... Args> T& emplace_back(Args&&... args) {
        return static_cast<T&>(*fContents->emplace_back(new T(std::forward<Args>(args)...)));
    }

    void pop_back() {
        fContents->pop_back();
    }

    iterator begin() {
        return iterator(fContents->begin());
    }

    iterator end() {
        return iterator(fContents->end());
    }

    const_iterator begin() const {
        return const_iterator(fContents->begin());
    }

    const_iterator end() const {
        return const_iterator(fContents->end());
    }

    T& operator[](int i) {
        SkASSERT(fContents->at(i));
        return fContents->at(i)->template as<T>();
    }

    const T& operator[](int i) const {
        SkASSERT(fContents->at(i));
        return fContents->at(i)->template as<T>();
    }

    T& front() {
        return fContents->front()->template as<T>();
    }

    const T& front() const {
        return fContents->front()->template as<T>();
    }

    T& back() {
        return fContents->back()->template as<T>();
    }

    const T& back() const {
        return fContents->back()->template as<T>();
    }

    int capacity() const {
        return fContents->capacity();
    }

private:
    SkTArray<std::unique_ptr<Base>>* fContents;
};

template<typename T, typename Base>
class ConstNodeArrayWrapper {
public:
    class iterator {
    public:
        const T& operator*() {
            return static_cast<T&>(**fBase);
        }

        const T* operator->() {
            return static_cast<T*>(fBase->get());
        }

        iterator& operator++() {
            ++fBase;
            return *this;
        }

        bool operator==(const iterator& other) const {
            return fBase == other.fBase;
        }

        bool operator!=(const iterator& other) const {
            return fBase != other.fBase;
        }

    private:
        iterator(const std::unique_ptr<Base>* base)
            : fBase(base) {}

        const std::unique_ptr<Base>* fBase;

        friend class ConstNodeArrayWrapper;
    };

    ConstNodeArrayWrapper(const SkTArray<std::unique_ptr<Base>>* contents)
        : fContents(contents) {}

    ConstNodeArrayWrapper(const ConstNodeArrayWrapper& other)
        : fContents(other.fContents) {}

    int count() const {
        return fContents->count();
    }

    bool empty() const {
        return fContents->empty();
    }

    iterator begin() const {
        return iterator(fContents->begin());
    }

    iterator end() const {
        return iterator(fContents->end());
    }

    T& operator[](int i) {
        return fContents->at(i)->template as<T>();
    }

    const T& operator[](int i) const {
        return fContents->at(i)->template as<T>();
    }

    T& front() {
        return fContents->front()->template as<T>();
    }

    const T& front() const {
        return fContents->front()->template as<T>();
    }

    T& back() { return fContents->back()->template as<T>(); }

    const T& back() const { return fContents->back()->template as<T>(); }

private:
    const SkTArray<std::unique_ptr<Base>>* fContents;
};

} // namespace SkSL

#endif
