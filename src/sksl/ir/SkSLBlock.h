/*
 * Copyright 2016 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_BLOCK
#define SKSL_BLOCK

#include "src/sksl/ir/SkSLStatement.h"
#include "src/sksl/ir/SkSLSymbolTable.h"

namespace SkSL {

/**
 * A block of multiple statements functioning as a single statement.
 */
class Block : public Statement {
public:
    static constexpr Kind kStatementKind = Kind::kBlock;

    Block(int offset, std::vector<std::unique_ptr<Statement>> statements,
          const std::shared_ptr<SymbolTable> symbols = nullptr, bool isScope = true)
    : INHERITED(offset, kStatementKind, BlockData{std::move(symbols), isScope},
                std::move(statements)) {}

    class iterator {
    public:
        Statement& operator*() {
            return **fIter;
        }

        iterator& operator++() {
            ++fIter;
            return *this;
        }

        bool operator==(const iterator& other) const {
            return fIter == other.fIter;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        using inner = std::vector<std::unique_ptr<Statement>>::iterator;

        iterator(inner iter)
        : fIter(iter) {}

        inner fIter;

        friend class Block;
    };

    class const_iterator {
    public:
        Statement& operator*() {
            return **fIter;
        }

        const_iterator& operator++() {
            ++fIter;
            return *this;
        }

        bool operator==(const const_iterator& other) const {
            return fIter == other.fIter;
        }

        bool operator!=(const const_iterator& other) const {
            return !(*this == other);
        }

    private:
        using inner = std::vector<std::unique_ptr<Statement>>::const_iterator;

        const_iterator(inner iter)
        : fIter(iter) {}

        inner fIter;

        friend class Block;
    };

    iterator begin() {
        return iterator(fStatementChildren.begin());
    }

    iterator end() {
        return iterator(fStatementChildren.end());
    }

    const_iterator begin() const {
        return const_iterator(fStatementChildren.begin());
    }

    const_iterator end() const {
        return const_iterator(fStatementChildren.end());
    }

    int childCount() const {
        return this->statementChildCount();
    }

    Statement& child(int index) const {
        return this->statementChild(index);
    }

    std::unique_ptr<Statement>& childPointer(int index) {
        return this->statementPointer(index);
    }

    const std::unique_ptr<Statement>& childPointer(int index) const {
        return this->statementPointer(index);
    }

    void reserve(int count) {
        fStatementChildren.reserve(count);
    }

    void addChild(std::unique_ptr<Statement> child) {
        fStatementChildren.push_back(std::move(child));
    }

    bool isScope() const {
        return this->blockData().fIsScope;
    }

    void setIsScope(bool isScope) {
        this->blockData().fIsScope = isScope;
    }

    std::shared_ptr<SymbolTable> symbolTable() const {
        return this->blockData().fSymbolTable;
    }

    bool isEmpty() const override {
        for (int i = 0; i < this->statementChildCount(); ++i) {
            if (!this->statementChild(i).isEmpty()) {
                return false;
            }
        }
        return true;
    }

    std::unique_ptr<Statement> clone() const override {
        std::vector<std::unique_ptr<Statement>> cloned;
        for (int i = 0; i < this->statementChildCount(); ++i) {
            cloned.push_back(this->statementChild(i).clone());
        }
        BlockData data = this->blockData();
        return std::unique_ptr<Statement>(new Block(fOffset, std::move(cloned), data.fSymbolTable,
                                                    data.fIsScope));
    }

    String description() const override {
        String result("{");
        for (int i = 0; i < this->statementChildCount(); ++i) {
            result += "\n";
            result += this->statementChild(i).description();
        }
        result += "\n}\n";
        return result;
    }

private:
    using INHERITED = Statement;
};

}  // namespace SkSL

#endif
