# Coding Standard for the ANGLE Project

## Google Style Guide

We generally use the [Google C++ Style Guide]
(http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml) as a basis for
our Coding Standard, however we will deviate from it in a few areas, as noted
below.

Items marked {DEV} indicate a deviation from the Google guidelines. Items marked
{DO} are reiterating points from the Google guidelines.

### [Header Files](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Header_Files)

*   We will use **`.h`** for C++ headers.
*   {DEV} #define guards should be of the form: `<PATH>_<FILE>_H_`. (Compiler
    codebase is varied, including `<PROJECT>_` makes the names excessively
    long).

### [Scoping](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Scoping)

*   {DO} avoid globally scoped variables, unless absolutely necessary.

### [Classes](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Classes)

*   {DO} disable Copy and Assignment constructors using the
    DISALLOW\_COPY\_AND\_ASSIGN macro (defined in common/angleutils.h) in the
    **private** section of a class: ``` class Foo { public: Foo(int f); ~Foo();

    private: DISALLOW_COPY_AND_ASSIGN(Foo); }; ```

### [Other C++ Features](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Other_C++_Features)

*   {DEV} all parameters passed by reference, except for STL containers (e.g.
    std::vector, std::list), must be labeled `const`. For return parameters
    other than STL containers, use a pointer.
*   {DO} avoid use of default arguments. exception: for functions emulating
    variadic arguments, they are allowed.
*   {DONT} use C++ exceptions, they are disabled in the builds and not caught.
*   {DO} use nullptr (instead of 0 or NULL) for pointers.
*   {DO} use size\_t for loop iterators and size values.
*   {DO} use uint8\_t pointers instead of void pointers to denote binary data.
*   {DO} use C++11 according to the [Chromium guide on C++11]
    (http://chromium-cpp.appspot.com/).
*   {DEV} we permit C++11 STL classes inside the D3D Renderers, since D3D is
    only supported on MSVS.

### [Naming ](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Naming)

#### File Names

*   {DEV} Filenames should be all lowercase and can include underscores (`_`).
    If the file is an implementation of a class, the filename may be capitalized
    the same as the major class.
*   {DEV} We use .cpp (instead of .cc), .h and .inl (inlined files) for C++
    files and headers. #### Directory Names
*   Directory names should be all lowercase, unless following an externally
    imposed capitalization (eg include/EGL, or src/libGLESv2, etc)

#### Variable Names

Use the following guidelines, they do deviate somewhat from the Google
guidelines. * class and type names: start with capital letter and use CamelCase.
* {DEV} class member variables: use an **`m`** prefix instead of trailing
underscore and use CamelCase. * global variables (if they must be used): use a
**`g_`** prefix. * {DEV} variable names: start with lower case and use CamelCase
(chosen for consitency) * {DEV} function names: Member functions start with
lower case and use CamelCase. Non-member functions start with capital letter and
use CamelCase (chosen for consistency) * Constants: start with a **`k`** and use
CamelCase * namespaces: use all lower case * Enumerator Names - follow constants
* macros: all uppercase with underscores * exceptions to naming: use common
sense!

### [Comments](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Comments)

*   {DO} read and follow Google's recommendations.
*   Each file **must** start with the following boilerplate notice: `// //
    Copyright (c) 2002-2011 The ANGLE Project Authors. All rights reserved. //
    Use of this source code is governed by a BSD-style license that can be //
    found in the LICENSE file. //
    `

### [Formatting](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Formatting)

*   {DEV} Avoid excessively long lines. Please keep lines under 120 columns
    long.
*   Use unix-style newlines.
*   {DO} use only spaces. No tab characters. Configure your editor to emit
    spaces when you hit the TAB-key.
*   {DEV} indent 4 spaces at a time.
*   conditionals: place space outside the parenthesis. No spaces inside.
*   switch statements: indent the case statements by 2 spaces. The body of the
    case statements should be intended another 2 spaces so that they are a full
    4-space indent from the switch.
*   class format(eg private, public, protected): indent by 2 spaces. Regular
    4-space indent from the outer scope for declarations/definitions.
*   pointers and references: **`*`** and **`&`** tight against the variable
*   namespaces: are not indented.
*   extern code blocks: are not indented.
*   {DEV} braces should go on a separate line, except for functions defined in a
    header file where the whole function declaration and definition fit on one
    line.

Examples: `if (conditional) { stuff(); } else { otherstuff() }
` `switch (conditional) { case foo: dostuff(); break; case bar: otherstuff()
break; default: WTFBBQ(); }
` `class MyClass : public Foo { public: MyClass(); ~MyClass() {}; private:
DISALLOW_COPY_AND_ASSIGN(MyClass); };
` `char *c; const string &str;
`

### [Exceptions to the Rules](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Exceptions_to_the_Rules)

*   If modifying pre-existing code that does not match the standard, the altered
    portions of the code should be changed to match the standard.
*   For changes which fix bugs, we do not require that pre-existing style issues
    be addressed in the change itself, but reviewers may request a follow-on CL
    to address style inconsistencies. This exception does not apply to changes
    which implement new features, perform refactoring, or introduce a style
    issue or inconsistency themselves.
