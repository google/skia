/*
 * Copyright 2020 Google LLC.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkTArray.h"

namespace SkSL {

class IRNode;

// Once the rearchitecture is complete, Base will be locked to IRNode, but right now we have to
// worry about both ExpressionArray and StatementArray
template<int N, typename T, typename Base>
class NodeArray {
public:
    class iterator {
    public:
        T& operator*() {
            return (T&) **fBase;
        }

        T* operator->() {
            return (T*) fBase->get();
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

        friend class NodeArray;
    };

    class const_iterator {
    public:
        const T& operator*() {
            return (const T&) **fBase;
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

        friend class NodeArray;
    };

    NodeArray(SkSTArray<N, std::unique_ptr<Base>>* contents)
        : fContents(*contents) {}

    NodeArray(const NodeArray& other)
        : fContents(other.fContents) {}

    NodeArray(NodeArray&& other)
        : fContents(std::move(other.fContents)) {}

    explicit NodeArray(int reserveCount)
        : fContents(reserveCount) {}

    NodeArray& operator=(const NodeArray& other) {
        fContents = other.fContents;
        return *this;
    }

    NodeArray& operator=(NodeArray&& other) {
        fContents = std::move(other.fContents);
        return *this;
    }

    void reset() {
        fContents.reset();
    }

    void reserve_back(int n) {
        fContents.reserve_back(n);
    }

    void removeShuffle(int n) {
        fContents.removeShuffle(n);
    }

    int count() const {
        return fContents.count();
    }

    bool empty() const {
        return fContents.empty();
    }

    T& push_back() {
        return (T&) *fContents.push_back();
    }

    T& push_back(const T& t) {
        return (T&) *fContents.push_back(t);
    }

    T& push_back(T&& t) {
        return (T&) *fContents.push_back(t);
    }

    template<class... Args> T& emplace_back(Args&&... args) {
        return (T&) *fContents.push_back(std::forward<Args>(args)...);
    }

    void push_back_n(int n) {
        fContents.push_back_n(n);
    }

    void push_back_n(int n, const T& t) {
        this->reserve_back(n);
        for (int i = 0; i < n; ++i) {
            this->push_back(t);
        }
    }

    void push_back_n(int n, const T t[]) {
        this->reserve_back(n);
        for (int i = 0; i < n; ++i) {
            this->push_back(t[i]);
        }
    }

    T* move_back_n(int n, T* t) {
        this->reserve_back(n);
        for (int i = 0; i < n; ++i) {
            this->move_back(t[i]);
        }
    }

    void pop_back() {
        fContents.pop_back();
    }

    void pop_back_n(int n) {
        fContents.pop_back_n(n);
    }

    void resize(int newCount) {
        fContents.resize_back(newCount);
    }

    iterator begin() {
        return iterator(fContents.begin());
    }

    iterator end() {
        return iterator(fContents.end());
    }

    const_iterator begin() const {
        return const_iterator(fContents.begin());
    }

    const_iterator end() const {
        return const_iterator(fContents.end());
    }

    T& operator[](int i) {
        return fContents[i]->template as<T>();
    }

    const T& operator[](int i) const {
        return fContents[i]->template as<T>();
    }

    T& front() { return (*this)[0]; }

    const T& front() const { return (*this)[0]; }

    T& back() { return fContents.back()->template as<T>(); }

    const T& back() const { return fContents.back()->template as<T>(); }

    int capacity() const {
        return fContents.capacity();
    }

private:
    SkSTArray<N, std::unique_ptr<Base>>& fContents;
};

template<int N, typename T, typename Base>
class ConstNodeArray {
public:
    class iterator {
    public:
        const T& operator*() {
            return (T&) **fBase;
        }

        const T* operator->() {
            return (T*) fBase->get();
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

        friend class ConstNodeArray;
    };

    ConstNodeArray(SkSTArray<N, std::unique_ptr<Base>>* contents)
        : fContents(*contents) {}

    ConstNodeArray(const ConstNodeArray& other)
        : fContents(other.fContents) {}

    ConstNodeArray(ConstNodeArray&& other)
        : fContents(std::move(other.fContents)) {}

    explicit ConstNodeArray(int reserveCount)
        : fContents(reserveCount) {}

    ConstNodeArray& operator=(const ConstNodeArray& other) {
        fContents = other.fContents;
        return *this;
    }

    ConstNodeArray& operator=(ConstNodeArray&& other) {
        fContents = std::move(other.fContents);
        return *this;
    }

    int count() const {
        return fContents.count();
    }

    bool empty() const {
        return fContents.empty();
    }

    iterator begin() const {
        return iterator(fContents.begin());
    }

    iterator end() const {
        return iterator(fContents.end());
    }

    T& operator[](int i) {
        return fContents[i]->template as<T>();
    }

    const T& operator[](int i) const {
        return fContents[i]->template as<T>();
    }

    T& front() { return (*this)[0]; }

    const T& front() const { return (*this)[0]; }

    T& back() { return fContents.back()->template as<T>(); }

    const T& back() const { return fContents.back()->template as<T>(); }

private:
    const SkSTArray<N, std::unique_ptr<Base>>& fContents;
};

} // namespace SkSL
