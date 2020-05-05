/*
 * Copyright 2020 Google LLC
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#ifndef SKSL_ASTBUILDER
#define SKSL_ASTBUILDER

#include <initializer_list>
#include <memory>
#include <vector>
#include "src/sksl/SkSLASTFile.h"

namespace SkSL {

class ASTBuilder {
public:
    ASTBuilder()
            : bool_ (type("bool"))
            , bool2_(type("bool2"))
            , bool3_(type("bool3"))
            , bool4_(type("bool4"))
            , float_ (type("float"))
            , float2_(type("float2"))
            , float3_(type("float3"))
            , float4_(type("float4"))
            , half_ (type("half"))
            , half2_(type("half2"))
            , half3_(type("half3"))
            , half4_(type("half4"))
            , int_ (type("int"))
            , int2_(type("int2"))
            , int3_(type("int3"))
            , int4_(type("int4"))
            , void_(type("void"))
            , fFile(new ASTFile()) {
        fFile->fRoot = this->create(ASTNode::Kind::kFile);
    }

    // TODO: Ternary (as static function in ASTBuilder?)
    // TODO: Unary operators, etc.
    // TODO: Ensure (bool/float/int) literals work (almost) everywhere

    struct Node {
        Node(ASTBuilder* owner) : fOwner(owner) {}
        virtual ~Node() {}

        virtual ASTNode::ID id() const = 0;
        ASTBuilder* fOwner;
    };

    // Placeholder for "default" arguments, to signal no arguments were passed
    struct MissingNode : public Node {
        MissingNode() : Node(nullptr) {}
        ASTNode::ID id() const override { return ASTNode::ID::Invalid(); }
    };

    struct Expression : public Node {
        Expression(ASTBuilder* owner, ASTNode::ID id) : Node(owner), fID(id) {}

        Expression& operator=(const Expression&) = delete;
//        Expression operator=(const Node& y) {
//            return owner->binary(Token::Kind::TK_EQ, id, y.id);
//        }

        ASTNode::ID id() const override { return fID; }
        ASTNode::ID fID;
    };

    // Variable just stores the name. Each reference needs a new node in the AST.
    struct Variable : public Node {
        Variable(ASTBuilder* owner, const StringFragment& name) : Node(owner), fName(name) {}

        ASTNode::ID id() const override {
            return fOwner->create(ASTNode::Kind::kIdentifier, fName);
        }

        Expression operator=(const Node& y) {
            return fOwner->binary(Token::Kind::TK_EQ, id(), y.id());
        }

        Expression operator=(bool y) { return *this = fOwner->operator[](y); }
        Expression operator=(float y) { return *this = fOwner->operator[](y); }
        Expression operator=(int y) { return *this = fOwner->operator[](y); }

        StringFragment fName;
    };

    // Type isn't a Node by itself (it can't participate in expressions). It may need to generate
    // either a kType node, or a kIdentifier node (for constructor calls).
    struct Type {
        Type(ASTBuilder* owner, const StringFragment& name) : fOwner(owner), fName(name) {}

        template <typename... Args>
        Expression operator()(Args&&... args) const {
            static_assert(sizeof...(args) > 0, "Type cast requires at least one argument");
            ASTNode::ID call = fOwner->create(ASTNode::Kind::kCall);
            ASTNode::ID id = fOwner->create(ASTNode::Kind::kIdentifier, fName);
            fOwner->getNode(call).addChild(id);
            this->addChildren(call, args...);
            return Expression(fOwner, call);
        }

        ASTNode::ID typeNode() const {
            ASTNode::ID id = fOwner->create(ASTNode::Kind::kType);
            fOwner->getNode(id).setTypeData({fName, false, false});
            return id;
        }

    private:
        void addChildren(ASTNode::ID call) const {}

        // TODO: Add literal overrides
        template <typename... Args>
        void addChildren(ASTNode::ID call, const Node& node, Args&&... remainder) const {
            fOwner->getNode(call).addChild(node.id());
            this->addChildren(call, remainder...);
        }

        template <typename... Args>
        void addChildren(ASTNode::ID call, float x, Args&&... remainder) const {
            fOwner->getNode(call).addChild(fOwner->operator[](x).id());
            this->addChildren(call, remainder...);
        }

        ASTBuilder*    fOwner;
        StringFragment fName;
    };

    // TODO: Arrays, structs, etc. Check for valid type names (Parser::fTypes)
    Type type(const char* name) { return Type(this, this->fragment(name)); }

    // Not a Node by itself, until it's invoked with the call operator.
    struct Function {
        Function(ASTBuilder* owner, ASTNode::ID id, ASTNode::ID body)
                : fOwner(owner), fID(id), fBody(body) {}

        Function& operator,(const Expression& e) {
            fOwner->getNode(fBody).addChild(e.id());
            return *this;
        }

        // TODO: Version with no argument
        // TODO: Other statements (going to need fBody to be a stack of blocks for scopes)
        void return_(const Node& e) {
            ASTNode::ID id = fOwner->create(ASTNode::Kind::kReturn);
            fOwner->getNode(id).addChild(e.id());
            fOwner->getNode(fBody).addChild(id);
        }

        Variable local(const Type& type, const char* name, const Node& init = MissingNode()) {
            SkSL::StringFragment nameFragment = fOwner->fragment(name);

            ASTNode::ID typeId = type.typeNode();
            ASTNode::ID decls = fOwner->create(ASTNode::Kind::kVarDeclarations);
            ASTNode::ID mods = fOwner->create(ASTNode::Kind::kModifiers, Modifiers());
            fOwner->getNode(decls).addChild(mods);
            fOwner->getNode(decls).addChild(typeId);

            ASTNode::ID var = fOwner->create(ASTNode::Kind::kVarDeclaration);
            fOwner->getNode(var).setVarData({nameFragment, 0});
            fOwner->getNode(decls).addChild(var);
            if (ASTNode::ID initVal = init.id()) {
                fOwner->getNode(var).addChild(initVal);
            }

            fOwner->getNode(fBody).addChild(decls);
            return Variable(fOwner, nameFragment);
        }

        Variable local(const Type& type, const char* name, float init) {
            return this->local(type, name, fOwner->operator[](init));
        }
        Variable local(const Type& type, const char* name, int init) {
            return this->local(type, name, fOwner->operator[](init));
        }
        Variable local(const Type& type, const char* name, bool init) {
            return this->local(type, name, fOwner->operator[](init));
        }

        ASTBuilder* fOwner;
        ASTNode::ID fID;
        ASTNode::ID fBody;
    };

    // TODO: Declaration without body?
    Function function(const Type& returnType, const char* name) {
        ASTNode::ID typeId = returnType.typeNode();
        ASTNode::ID id = this->create(ASTNode::Kind::kFunction);
        this->getNode(id).setFunctionData({Modifiers(), this->fragment(name), 0});
        this->getNode(id).addChild(typeId);

        ASTNode::ID body = this->create(ASTNode::Kind::kBlock);
        this->getNode(id).addChild(body);

        this->getNode(fFile->fRoot).addChild(id);

        // Scope everything to function, so it goes in the right block?
        // Keep a reference (or stack of references) to in-progress function, so statements can
        // automatically be appended?
        return Function(this, id, body);
    }

    template <typename... Args>
    Function function(const Type& returnType, const char* name, Args&&... args) {
        ASTNode::ID typeId = returnType.typeNode();
        ASTNode::ID id = this->create(ASTNode::Kind::kFunction);
        this->getNode(id).setFunctionData({Modifiers(), this->fragment(name), sizeof...(args)/2});
        this->getNode(id).addChild(typeId);

        this->addParameters(id, args...);

        ASTNode::ID body = this->create(ASTNode::Kind::kBlock);
        this->getNode(id).addChild(body);

        this->getNode(fFile->fRoot).addChild(id);

        return Function(this, id, body);
    }

    void addParameters(ASTNode::ID func) {}

    template <typename... Args>
    void addParameters(ASTNode::ID func, const Type& type, const char* name, Args&&... remainder) {
        ASTNode::ID typeId = type.typeNode();
        ASTNode::ID id = this->create(ASTNode::Kind::kParameter);
        this->getNode(id).setParameterData({Modifiers(), this->fragment(name), 0});
        this->getNode(id).addChild(typeId);

        this->getNode(func).addChild(id);

        this->addParameters(func, remainder...);
    }

    // TODO: Is there a better name for this? Function named '_'? Or slightly longer 'term'?
    // operator[] converts the given argument to a Node for use in expressions, of the appropriate
    // type. It handles:
    // strings        -> Identifier
    // int/float/bool -> Literals

    // For now, assume identifiers are variables
    Variable operator[](const char* name) {
        return Variable(this, this->fragment(name));
    }
    Expression operator[](int i) {
        return Expression(this, this->create(ASTNode::Kind::kInt, i));
    }
    Expression operator[](float f) {
        return Expression(this, this->create(ASTNode::Kind::kFloat, f));
    }
    Expression operator[](bool b) {
        return Expression(this, this->create(ASTNode::Kind::kBool, b));
    }

    std::unique_ptr<ASTFile> file() { return std::move(fFile); }

    std::vector<std::unique_ptr<char[]>> fStrings;

    const Type bool_;
    const Type bool2_;
    const Type bool3_;
    const Type bool4_;
    const Type float_;
    const Type float2_;
    const Type float3_;
    const Type float4_;
    const Type half_;
    const Type half2_;
    const Type half3_;
    const Type half4_;
    const Type int_;
    const Type int2_;
    const Type int3_;
    const Type int4_;
    const Type void_;

    Expression binary(Token::Kind kind, ASTNode::ID lhs, ASTNode::ID rhs) {
        ASTNode::ID id = this->create(ASTNode::Kind::kBinary, Token(kind, -1, -1));
        this->getNode(id).addChild(lhs);
        this->getNode(id).addChild(rhs);
        return Expression(this, id);
    }

private:
    template <typename... Args>
    ASTNode::ID create(ASTNode::Kind kind, Args&&... args) {
        ASTNode::ID id = fFile->fNodes.size();
        fFile->fNodes.emplace_back(&fFile->fNodes, -1, kind, args...);
        return id;
    }

    ASTNode& getNode(ASTNode::ID id) {
        SkASSERT(id.fValue >= 0 && id.fValue < (int)fFile->fNodes.size());
        return fFile->fNodes[id.fValue];
    }

    StringFragment fragment(const char* s) {
        size_t len = strlen(s);
        std::unique_ptr<char[]> data(new char[len]);
        memcpy(data.get(), s, len);
        fStrings.push_back(std::move(data));
        return StringFragment(fStrings.back().get(), len);
    }

    std::unique_ptr<ASTFile> fFile;
};

#define BINARY_OP(op, tk)                                                          \
    static inline ASTBuilder::Expression operator op (const ASTBuilder::Node& x,   \
                                                      const ASTBuilder::Node& y) { \
        return x.fOwner->binary(Token::Kind::tk, x.id(), y.id());                  \
    }

#define BINARY_OP_LITERAL(op, type)                                                \
    static inline ASTBuilder::Expression operator op (const ASTBuilder::Node& x,   \
                                                      type y) {                    \
        return x op x.fOwner->operator[](y);                                       \
    }                                                                              \
    static inline ASTBuilder::Expression operator op (type x,                      \
                                                      const ASTBuilder::Node& y) { \
        return y.fOwner->operator[](x) op y;                                       \
    }

// Define binary operators that operate on (Node, Node), as well as various kinds of literals:
// B = bool
// F = float
// I = int
#define BINARY_OP_B(op, tk)      \
    BINARY_OP(op, tk)            \
    BINARY_OP_LITERAL(op, bool)

#define BINARY_OP_BFI(op, tk)    \
    BINARY_OP(op, tk)            \
    BINARY_OP_LITERAL(op, bool)  \
    BINARY_OP_LITERAL(op, float) \
    BINARY_OP_LITERAL(op, int)

#define BINARY_OP_FI(op, tk)     \
    BINARY_OP(op, tk)            \
    BINARY_OP_LITERAL(op, float) \
    BINARY_OP_LITERAL(op, int)

#define BINARY_OP_I(op, tk)      \
    BINARY_OP(op, tk)            \
    BINARY_OP_LITERAL(op, int)

// Like above, but the LHS must be a Variable (LValue)
#define COMPOUND_OP(op, tk)                                                         \
    static inline ASTBuilder::Expression operator op(const ASTBuilder::Variable& x, \
                                                     const ASTBuilder::Node& y) {   \
        return x.fOwner->binary(Token::Kind::tk, x.id(), y.id());                   \
    }

#define COMPOUND_OP_LITERAL(op, type)                                                \
    static inline ASTBuilder::Expression operator op (const ASTBuilder::Variable& x, \
                                                      type y) {                      \
        return x op x.fOwner->operator[](y);                                         \
    }

#define COMPOUND_OP_I(op, tk)      \
    COMPOUND_OP(op, tk)            \
    COMPOUND_OP_LITERAL(op, int)

#define COMPOUND_OP_FI(op, tk)     \
    COMPOUND_OP(op, tk)            \
    COMPOUND_OP_LITERAL(op, float) \
    COMPOUND_OP_LITERAL(op, int)

BINARY_OP_B(||, TK_LOGICALOR)
BINARY_OP_B(&&, TK_LOGICALAND)

BINARY_OP_I(|, TK_BITWISEOR)
BINARY_OP_I(^, TK_BITWISEXOR)
BINARY_OP_I(&, TK_BITWISEAND)

COMPOUND_OP_I(|=, TK_BITWISEOREQ)
COMPOUND_OP_I(^=, TK_BITWISEXOREQ)
COMPOUND_OP_I(&=, TK_BITWISEANDEQ)

BINARY_OP_BFI(==, TK_EQEQ)
BINARY_OP_BFI(!=, TK_NEQ)
BINARY_OP_FI(< , TK_LT)
BINARY_OP_FI(> , TK_GT)
BINARY_OP_FI(<=, TK_LTEQ)
BINARY_OP_FI(>=, TK_GTEQ)

BINARY_OP_I(<<, TK_SHL)
BINARY_OP_I(>>, TK_SHR)

COMPOUND_OP_I(<<=, TK_SHLEQ)
COMPOUND_OP_I(>>=, TK_SHREQ)

BINARY_OP_FI(+, TK_PLUS)
BINARY_OP_FI(-, TK_MINUS)
BINARY_OP_FI(*, TK_STAR)
BINARY_OP_FI(/, TK_SLASH)
BINARY_OP_FI(%, TK_PERCENT)

COMPOUND_OP_FI(+=, TK_PLUSEQ)
COMPOUND_OP_FI(-=, TK_MINUSEQ)
COMPOUND_OP_FI(*=, TK_STAREQ)
COMPOUND_OP_FI(/=, TK_SLASHEQ)
COMPOUND_OP_FI(%=, TK_PERCENTEQ)

} // namespace

#endif
