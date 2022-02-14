SkSL Binary Format
==================

Background
----------

The SkSL binary format was originally created for internal use, to store our built-in include files
in a format which required less processing time than the original text versions.

This improved compiler startup time and memory footprint, but as the format was purpose-built for
encoding our include files, it could not encode a normal SkSL program. It only supported the subset
of SkSL required by the include files, had no means of referring to external symbols, and had not
been tested outside of being used to encode our small handful of include files.

To both improve testability and to support the needs of clients requiring a binary SkSL format, this
feature has been extended to support arbitrary SkSL files. Full test coverage is achieved by
encoding our entire corpus of SkSL tests to binary, decoding them, and ensuring that the decoded
output program matches the unmodified original program.

Design
------

### Overview

A binary SkSL file has the following header:

| Type                 | Field Name   |
|----------------------|--------------|
| `uint16`             | version      |
| `uint16`             | stringLength |
| `char[stringLength]` | stringData   |

The version number is incremented whenever the file format changes. This document describes version
8.

`stringLength` is the total length of all of the string data in the file, including the length bytes
of the strings, but not counting the `stringLength` field itself. Each string consists of a `uint8`
length followed by the string’s characters, which are not null terminated.

The header is immediately followed by a sequence of one or more commands encoding the contents. A
typical SkSL binary will contain exactly one `kProgram_Command`.

### Types

The following types may appear as data members of commands. All numbers are stored in little endian
byte ordering.

| Name                   | Description                                                             |
|------------------------|-------------------------------------------------------------------------|
| `int8`                 | a signed 8 bit integer                                                  |
| `int16`                | a signed 16 bit integer                                                 |
| `int32`                | a signed 32 bit integer                                                 |
| `uint8`                | an unsigned 8 bit integer                                               |
| `uint16`               | an unsigned 16 bit integer                                              |
| `uint32`               | an unsigned 32 bit integer                                              |
| `float`                | a 32 bit IEEE floating point number                                     |
| `bool`                 | a single byte with the value either 0 (`false`) or 1 (`true`)           |
| `String`               | a `uint8` length, followed by a `uint16` offset [^1]                    |
| `ProgramKind`          | a `uint8` mapping to a value in the `SkSL::ProgramKind` enum            |
| `VariableStorage`      | a `uint8` mapping to a value in the `SkSL::VariableStorage` enum        |
| `Operator`             | a `uint8` mapping to a value in the `SkSL::Token::Kind` enum            |
| `FieldAccessOwnerKind` | a `uint8` mapping to a value in the `SkSL::FieldAccessOwnerKind` enum   |
| `VariableRefKind`      | a `uint8` mapping to a value in the `SkSL::VariableRefKind` enum        |
| `SwizzleComponent`     | a `uint8` mapping to a value in the `SkSL::SwizzleComponent::Type` enum |
| `Symbol`               | an instance of any command in the Symbol category                       |
| `SymbolId`             | a `uint16` ID specified by a Symbol category command [^2]               |
| `Expression`           | an instance of any command in the Expressions category                  |
| `ProgramElement`       | an instance of any command in the ProgramElements category              |
| `Statement`            | an instance of any command in the Statements category                   |
| `Type`                 | an instance of one of the “Type” Symbol commands [^3]                   |
| `Layout`               | an instance of the one of the Layout commands [^4]                      |
| `Modifiers`            | an instance of one of the Modifiers commands [^5]                       |
| `k*_Command`           | an instance of the specified command                                    |

[^1]: The offset of a `String` is relative to the beginning of the `stringData` field of the header.

[^2]: There is one exception to `SymbolId`s referring to Symbol commands: if a `SymbolId` is
      `kBuiltin_Symbol`, the `SymbolId` is followed by a String representing the name of the symbol.

[^3]: The Type symbol commands are `kArrayType_Command`, `kStructType_Command`, or a
      `kSymbolRef_Command` referring to an existing type.

[^4]: The "Layout" commands are `kBuiltinLayout_Command`, `kDefaultLayout_Command`, and
      `kLayout_Command`.

[^5]: The "Modifiers" commands are `kDefaultModifiers_Command`, `kModifiers8Bit_Command`, and
      `kModifiers_Command)`.

Commands
--------

Each command consists of a single `uint8` identifying the command, followed by its data.

### General Commands

#### kBuiltinLayout_Command

| Type    | Field Name |
|---------|------------|
| `int16` | builtin    |

An `SkSL::Layout` with the specified builtin value, but all other fields left with their default
values.

---

#### kDefaultLayout_Command

An `SkSL::Layout` with every field set to its default value.

---

#### kDefaultModifiers_Command

An `SkSL::Modifiers` with every field set to its default value.

---

#### kElements_Command

| Type                        | Field Name |
|-----------------------------|------------|
| `ProgramElement[]`          | elements   |
| `kElementsComplete_Command` |            |

The end of the elements array is indicated by the presence of `kElementsComplete_Command`.

---

#### kElementsComplete_Command

Signifies the end of a list of elements.

---

#### kLayout_Command

| Type    | Field Name           |
|---------|----------------------|
| `int32` | flags                |
| `int8`  | location             |
| `int16` | offset               |
| `int16` | binding              |
| `int8`  | index                |
| `int8`  | set                  |
| `int16` | builtin              |
| `int8`  | inputAttachmentIndex |

Represents an `SkSL::Layout` with all fields specified.

---

#### kModifiers8Bit_Command

| Type     | Field Name |
|----------|------------|
| `Layout` | layout     |
| `uint8`  | flags      |

A shortened version of `Modifiers` which only represents the least significant 8 bits of `flags`,
leaving the other bits as zero.

---

#### kModifiers_Command

| Type     | Field Name |
|----------|------------|
| `Layout` | layout     |
| `uint32` | flags      |

---

#### kProgram_Command

| Type                   | Field Name       |
|------------------------|------------------|
| `ProgramKind`          | programKind      |
| `kSymbolTable_Command` | symbolTable      |
| `kElements_Command`    | elements         |
| `bool`                 | useFlipRTUniform |

`useFlipRTUniform` refers to a field in `Program::Inputs`.

---

#### kSymbolTable_Command

| Type                       | Field Name       |
|----------------------------|------------------|
| `uint16`                   | ownedSymbolCount |
| `Symbol[ownedSymbolCount]` | ownedSymbols     |
| `uint16`                   | symbolCount      |
| `SymbolID[symbolCount]`    | symbols          |

The IDs in the symbols array do not relate to indices in `ownedSymbols`, and instead refer to the
IDs specified at the beginning of commands in the Symbol category.

---

### ProgramElement Commands

#### kFunctionDefinition_Command

| Type        | Field Name  |
|-------------|-------------|
| `SymbolId`  | declaration |
| `Statement` | body        |

Provides the body corresponding to a `FunctionDeclaration`.

---

#### kFunctionPrototype_Command

| Type     | Field Name  |
|----------|-------------|
| `uint16` | declaration |

Represents a function declaration appearing in the program as a function prototype.

---

#### kInterfaceBlock_Command

| Type       | Field Name   |
|------------|--------------|
| `SymbolId` | var          |
| `String`   | typeName     |
| `String`   | instanceName |
| `uint8`    | arraySize    |

An interface block. `instanceName` may be zero-length to indicate the lack of an instance name.
`arraySize` will be zero if the instance name was not present or was not followed by an array size.

---

#### kStructDefinition_Command

| Type   | Field Name |
|--------|------------|
| `Type` | structType |

Represents a struct definition appearing at top-level scope.

---

#### kSharedFunction_Command

| Type                       | Field Name     |
|----------------------------|----------------|
| `uint8`                    | parameterCount |
| `Variable[parameterCount]` | parameters     |
| `FunctionDeclaration`      | decl           |
| `FunctionDefinition`       | defn           |

Represents a function from `Program.fSharedElements`.

---

#### kGlobalVar_Command

| Type                      | Field Name |
|---------------------------|------------|
| `kVarDeclaration_Command` | decl       |

A `VarDeclaration` appearing in global scope.

---

### Symbol Commands

#### kArrayType_Command

| Type     | Field Name    |
|----------|---------------|
| `uint16` | id            |
| `Type`   | componentType |
| `uint8`  | count         |

---

#### kField_Command

| Type       | Field Name |
|------------|------------|
| `SymbolId` | ownerId    |
| `uint8`    | index      |

Symbol referring to a particular field of a `StructType`. Unlike other symbols, Field does not have
its own ID; references to a Field are compiled in terms of the owner’s ID.

---

#### kFunctionDeclaration_Command

| Type                       | Field Name     |
|----------------------------|----------------|
| `uint16`                   | id             |
| `Modifiers`                | modifiers      |
| `String`                   | name           |
| `uint8`                    | parameterCount |
| `SymbolId[parameterCount]` | parameters     |
| `Type`                     | returnType     |

---

#### kStructType_Command

| Type                | Field Name |
|---------------------|------------|
| `uint16`            | id         |
| `String`            | name       |
| `uint8`             | fieldCount |
| `Field[fieldCount]` | fields     |

The `Field` type referenced in this command is:

    struct Field {
        Modifiers modifiers;
        String name;
        Type type;
    };

---

#### kSymbolRef_Command

| Type       | Field Name |
|------------|------------|
| `SymbolId` | id         |

Refers to a Symbol which has already been written by another command.

---

#### kUnresolvedFunction_Command

| Type                         | Field Name |
|------------------------------|------------|
| `uint16`                     | id         |
| `uint8`                      | count      |
| `FunctionDeclaration[count]` | functions  |

---

#### kVariable_Command

| Type                 | Field Name |
|----------------------|------------|
| `uint16`             | id         |
| `Modifiers`          | modifiers  |
| `String`             | name       |
| `Type`               | type       |
| `VariableStorage`    | storage    |

---

### Expression Commands

#### kBinary_Command

| Type         | Field Name |
|--------------|------------|
| `Expression` | left       |
| `Operator`   | op         |
| `Expression` | right      |
| `Type`       | type       |

Represents a binary operator applied to two expressions.

---

#### kBoolLiteral_Command

| Type   | Field Name |
|--------|------------|
| `bool` | value      |

Represents a literal `true` or `false` value.

---

#### kConstructorArray_Command

| Type                   | Field Name |
|------------------------|------------|
| `Type`                 | type       |
| `uint8`                | argCount   |
| `Expression[argCount]` | arguments  |

Represents an instance of `SkSL::ConstructorArray`.

---

#### kConstructorArrayCast_Command

| Type                   | Field Name |
|------------------------|------------|
| `Type`                 | type       |
| `uint8`                | argCount   |
| `Expression[argCount]` | arguments  |

Represents an instance of `SkSL::ConstructorArrayCast`.

---

#### kConstructorCompound_Command

| Type                   | Field Name |
|------------------------|------------|
| `Type`                 | type       |
| `uint8`                | argCount   |
| `Expression[argCount]` | arguments  |

Represents an instance of `SkSL::ConstructorCompound`.

---

#### kConstructorCompoundCast_Command

| Type                   | Field Name |
|------------------------|------------|
| `Type`                 | type       |
| `uint8`                | argCount   |
| `Expression[argCount]` | arguments  |

Represents an instance of `SkSL::ConstructorCompoundCast`.

---

#### kConstructorDiagonalMatrix_Command

| Type                   | Field Name |
|------------------------|------------|
| `Type`                 | type       |
| `uint8`                | argCount   |
| `Expression[argCount]` | arguments  |

Represents an instance of `SkSL::ConstructorDiagonalMatrix`.

---

#### kConstructorMatrixResize_Command

| Type                   | Field Name |
|------------------------|------------|
| `Type`                 | type       |
| `uint8`                | argCount   |
| `Expression[argCount]` | arguments  |

Represents an instance of `SkSL::ConstructorMatrixResize`.

---

#### kConstructorScalarCast_Command

| Type                   | Field Name |
|------------------------|------------|
| `Type`                 | type       |
| `uint8`                | argCount   |
| `Expression[argCount]` | arguments  |

Represents an instance of `SkSL::ConstructorScalarCast`.

---

#### kConstructorSplat_Command

| Type                   | Field Name |
|------------------------|------------|
| `Type`                 | type       |
| `uint8`                | argCount   |
| `Expression[argCount]` | arguments  |

Represents an instance of `SkSL::ConstructorSplat`.

---

#### kConstructorStruct_Command

| Type                   | Field Name |
|------------------------|------------|
| `Type`                 | type       |
| `uint8`                | argCount   |
| `Expression[argCount]` | arguments  |

Represents an instance of `SkSL::ConstructorStruct`.

---

#### kFieldAccess_Command

| Type                   | Field Name |
|------------------------|------------|
| `Expression`           | base       |
| `uint8`                | index      |
| `FieldAccessOwnerKind` | ownerKind  |

A reference to the `index`’th field of the `base` struct.

---

#### kFloatLiteral_Command

| Type    | Field Name |
|---------|------------|
| `float` | value      |

Represents a floating point literal.

---

#### kFunctionCall_Command

| Type                   | Field Name |
|------------------------|------------|
| `Type`                 | type       |
| `SymbolId`             | function   |
| `uint8`                | argCount   |
| `Expression[argCount]` | arguments  |

Represents a call to `function(arguments...)`.

---

#### kIndex_Command

| Type         | Field Name |
|--------------|------------|
| `Expression` | base       |
| `Expression` | index      |

Represents the expression `base[index]`.

---

#### kIntLiteral_Command

| Type    | Field Name |
|---------|------------|
| `int32` | value      |

Represents an integer literal.

---

#### kPostfix_Command

| Type         | Field Name |
|--------------|------------|
| `Operator`   | op         |
| `Expression` | operand    |

Represents applying the postfix operator `op` to `operand`.

---

#### kPrefix_Command

| Type         | Field Name |
|--------------|------------|
| `Operator`   | op         |
| `Expression` | operand    |

Represents applying the prefix operator `op` to `operand`.

---

#### kSetting_Command

| Type     | Field Name |
|----------|------------|
| `String` | name       |

Represents a field of `sk_Caps`, such as `“builtinDeterminantSupport”`.

---

#### kSwizzle_Command

| Type                               | Field Name     |
|------------------------------------|----------------|
| `Expression`                       | base           |
| `uint8`                            | componentCount |
| `SwizzleComponent[componentCount]` | components     |

Represents the swizzle `base.components`.

---

#### kTernary_Command

| Type         | Field Name |
|--------------|------------|
| `Expression` | test       |
| `Expression` | ifTrue     |
| `Expression` | ifFalse    |

Represents the ternary expression `test ? ifTrue : ifFalse`.

---

#### kVariableReference_Command

| Type              | Field Name |
|-------------------|------------|
| `SymbolID`        | var        |
| `VariableRefKind` | refKind    |

Represents a reference to the variable `var`.

---

### Statement Commands

#### kBlock_Command

| Type                        | Field Name     |
|-----------------------------|----------------|
| `kSymbolTable_Command`      | symbolTable    |
| `uint8`                     | statementCount |
| `Statement[statementCount]` | statements     |
| `bool`                      | isScope        |

A block of statements.

---

#### kBreak_Command

A `break` statement.

---

#### kContinue_Command

A `continue` statement.

---

#### kDiscard_Command

A `discard` statement.

---

#### kDo_Command

| Type         | Field Name |
|--------------|------------|
| `Statement`  | stmt       |
| `Expression` | test       |

Represents `do stmt; while(test);`.

---

#### kExpressionStatement_Command

| Type         | Field Name |
|--------------|------------|
| `Expression` | expression |

Represents the statement `expression;`.

---

#### kFor_Command

| Type          | Field Name  |
|---------------|-------------|
| `Statement`   | initializer |
| `Expression`  | test        |
| `Expression`  | next        |
| `Statement`   | body        |
| `SymbolTable` | symbols     |

Represents `for (initializer; test; next) body;`.
`test`, `next`, and `body` may all be `kVoid_Command` to represent their absence.

---

#### kIf_Command

| Type         | Field Name |
|--------------|------------|
| `bool`       | isStatic   |
| `Expression` | test       |
| `Statement`  | ifTrue     |
| `Statement`  | ifFalse    |

Represents `if (test) ifTrue; else ifFalse;` (or `@if`, if `isStatic` is true).
`ifFalse` may be `kVoid_Command` to represent the absence of an `else` statement.

---

#### kInlineMarker_Command

| Type                   | Field Name |
|------------------------|------------|
| `SymbolId`             | function   |

Represents an `SkSL::InlineMarker`, which is inserted before an inlined function’s code.

---

#### kNop_Command

Represents an empty statement (a bare semicolon).

---

#### kReturn_Command

| Type                   | Field Name |
|------------------------|------------|
| `Expression`           | value      |

Represents the statement `return value;`.
`value` may be `kVoid_Command` to represent the absence of a return value.

---

#### kSwitch_Command

| Type                   | Field Name |
|------------------------|------------|
| `bool`                 | isStatic   |
| `kSymbolTable_Command` | symbols    |
| `Expression`           | value      |
| `uint8`                | caseCount  |
| `Case[caseCount]`      | cases      |

The `Case` type referenced in this command is:

    struct Case {
        bool isDefault;
        (when !isDefault) Expression value;
        uint8 statementCount;
        Statement statements[statementCount];
    };

Represents a `switch` or `@switch` statement on value. Each case begins with a `bool` identifying
whether or not it is the default case; the default case does not contain the `value` field.

---

#### kVarDeclaration_Command

| Type                   | Field Name |
|------------------------|------------|
| `SymbolId`             | var        |
| `Type`                 | baseType   |
| `uint8`                | arraySize  |
| `Expression`           | value      |

Represents the variable declaration statement `baseType var[arraySize] = value;`.
An `arraySize` of zero omits the array declaration. A value of `kVoid_Command` omits the
initializer.

---

#### kVoid_Command

A marker indicating that a node is not present.
