### Compilation failed:

=================================================================
==77287==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013da0 at pc 0x000108eb4c05 bp 0x7ffee6e60ee0 sp 0x7ffee6e60ed8
READ of size 8 at 0x60d000013da0 thread T0
    #0 0x108eb4c04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x108eb39a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x108eb2fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x108f58d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x108f67657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x108f646e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x108d9f886 in main SkSLMain.cpp:242
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013da0 is located 128 bytes inside of 136-byte region [0x60d000013d20,0x60d000013da8)
freed by thread T0 here:
    #0 0x10aacbc6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10923c8dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x109079e01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x108f5408d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x108f9f97d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x108f53a86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x108f4f4a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x108f59012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x108f67657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x108f646e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x108d9f886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x10aacb84d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x109114fdd in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable*, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable*&&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x1091079a3 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:472
    #3 0x1090ee116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x1090ec17e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x1090fd0e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x1090eda03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #7 0x1091308cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #8 0x109195663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #9 0x108f63e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #10 0x108d9f886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c1a00002760: 00 00 00 00 00 00 00 fa fa fa fa fa fa fa fa fa
  0x1c1a00002770: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002780: 00 fa fa fa fa fa fa fa fa fa 00 00 00 00 00 00
  0x1c1a00002790: 00 00 00 00 00 00 00 00 00 00 00 fa fa fa fa fa
  0x1c1a000027a0: fa fa fa fa fd fd fd fd fd fd fd fd fd fd fd fd
=>0x1c1a000027b0: fd fd fd fd[fd]fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027c0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027d0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027e0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027f0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a00002800: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
Shadow byte legend (one shadow byte represents 8 application bytes):
  Addressable:           00
  Partially addressable: 01 02 03 04 05 06 07 
  Heap left redzone:       fa
  Freed heap region:       fd
  Stack left redzone:      f1
  Stack mid redzone:       f2
  Stack right redzone:     f3
  Stack after return:      f5
  Stack use after scope:   f8
  Global redzone:          f9
  Global init order:       f6
  Poisoned by user:        f7
  Container overflow:      fc
  Array cookie:            ac
  Intra object redzone:    bb
  ASan internal:           fe
  Left alloca redzone:     ca
  Right alloca redzone:    cb
  Shadow gap:              cc
==77287==ABORTING
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -42.0
Node 1 (0x6130000018d8): float x = -42.0
Node 2 (0x6130000018f0): float x = -42.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): x
Node 6 (0x613000001950): fract(x)
Node 7 (0x613000001968): half(fract(x))
Node 8 (0x613000001980): (sk_FragColor.x = half(fract(x)))
Node 9 (0x613000001998): (sk_FragColor.x = half(fract(x)));
Exits: [1]

Block 1
-------
Before: [float x = -42.0]
Entrances: [0]
Exits: []

about to simplify EX -42.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -42.0
Node 1 (0x6130000018d8): float x = -42.0
Node 2 (0x6130000018f0): float x = -42.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): x
Node 6 (0x613000001950): fract(x)
Node 7 (0x613000001968): half(fract(x))
Node 8 (0x613000001980): (sk_FragColor.x = half(fract(x)))
Node 9 (0x613000001998): (sk_FragColor.x = half(fract(x)));
Exits: [1]

Block 1
-------
Before: [float x = -42.0]
Entrances: [0]
Exits: []

about to simplify ST float x = -42.0 
simplifying vardecl
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -42.0
Node 1 (0x6130000018d8): float x = -42.0
Node 2 (0x6130000018f0): float x = -42.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): x
Node 6 (0x613000001950): fract(x)
Node 7 (0x613000001968): half(fract(x))
Node 8 (0x613000001980): (sk_FragColor.x = half(fract(x)))
Node 9 (0x613000001998): (sk_FragColor.x = half(fract(x)));
Exits: [1]

Block 1
-------
Before: [float x = -42.0]
Entrances: [0]
Exits: []

about to simplify ST float x = -42.0; 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -42.0
Node 1 (0x6130000018d8): float x = -42.0
Node 2 (0x6130000018f0): float x = -42.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): x
Node 6 (0x613000001950): fract(x)
Node 7 (0x613000001968): half(fract(x))
Node 8 (0x613000001980): (sk_FragColor.x = half(fract(x)))
Node 9 (0x613000001998): (sk_FragColor.x = half(fract(x)));
Exits: [1]

Block 1
-------
Before: [float x = -42.0]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -42.0
Node 1 (0x6130000018d8): float x = -42.0
Node 2 (0x6130000018f0): float x = -42.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): x
Node 6 (0x613000001950): fract(x)
Node 7 (0x613000001968): half(fract(x))
Node 8 (0x613000001980): (sk_FragColor.x = half(fract(x)))
Node 9 (0x613000001998): (sk_FragColor.x = half(fract(x)));
Exits: [1]

Block 1
-------
Before: [float x = -42.0]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor.x 
optimizing swiz 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -42.0
Node 1 (0x6130000018d8): float x = -42.0
Node 2 (0x6130000018f0): float x = -42.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): x
Node 6 (0x613000001950): fract(x)
Node 7 (0x613000001968): half(fract(x))
Node 8 (0x613000001980): (sk_FragColor.x = half(fract(x)))
Node 9 (0x613000001998): (sk_FragColor.x = half(fract(x)));
Exits: [1]

Block 1
-------
Before: [float x = -42.0]
Entrances: [0]
Exits: []

about to simplify EX x 
optimized to -42.0 
coerced to -42.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -42.0
Node 1 (0x6130000018d8): float x = -42.0
Node 2 (0x6130000018f0): float x = -42.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): -42.0
Node 6 (0x613000001950): fract(-42.0)
Node 7 (0x613000001968): half(fract(-42.0))
Node 8 (0x613000001980): (sk_FragColor.x = half(fract(-42.0)))
Node 9 (0x613000001998): (sk_FragColor.x = half(fract(-42.0)));
Exits: [1]

Block 1
-------
Before: [float x = -42.0]
Entrances: [0]
Exits: []

about to simplify EX fract(-42.0) 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -42.0
Node 1 (0x6130000018d8): float x = -42.0
Node 2 (0x6130000018f0): float x = -42.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): -42.0
Node 6 (0x613000001950): fract(-42.0)
Node 7 (0x613000001968): half(fract(-42.0))
Node 8 (0x613000001980): (sk_FragColor.x = half(fract(-42.0)))
Node 9 (0x613000001998): (sk_FragColor.x = half(fract(-42.0)));
Exits: [1]

Block 1
-------
Before: [float x = -42.0]
Entrances: [0]
Exits: []

about to simplify EX half(fract(-42.0)) 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -42.0
Node 1 (0x6130000018d8): float x = -42.0
Node 2 (0x6130000018f0): float x = -42.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): -42.0
Node 6 (0x613000001950): fract(-42.0)
Node 7 (0x613000001968): half(fract(-42.0))
Node 8 (0x613000001980): (sk_FragColor.x = half(fract(-42.0)))
Node 9 (0x613000001998): (sk_FragColor.x = half(fract(-42.0)));
Exits: [1]

Block 1
-------
Before: [float x = -42.0]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor.x = half(fract(-42.0))) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -42.0
Node 1 (0x6130000018d8): float x = -42.0
Node 2 (0x6130000018f0): float x = -42.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): -42.0
Node 6 (0x613000001950): fract(-42.0)
Node 7 (0x613000001968): half(fract(-42.0))
Node 8 (0x613000001980): (sk_FragColor.x = half(fract(-42.0)))
Node 9 (0x613000001998): (sk_FragColor.x = half(fract(-42.0)));
Exits: [1]

Block 1
-------
Before: [float x = -42.0]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor.x = half(fract(-42.0))); 
simplifying expr
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -42.0
Node 1 (0x6130000018d8): float x = -42.0
Node 2 (0x6130000018f0): float x = -42.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): -42.0
Node 6 (0x613000001950): fract(-42.0)
Node 7 (0x613000001968): half(fract(-42.0))
Node 8 (0x613000001980): (sk_FragColor.x = half(fract(-42.0)))
Node 9 (0x613000001998): (sk_FragColor.x = half(fract(-42.0)));
Exits: [1]

Block 1
-------
Before: [float x = -42.0]
Entrances: [0]
Exits: []

about to simplify EX -42.0 
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -42.0
Node 1 (0x6130000018d8): float x = -42.0
Node 2 (0x6130000018f0): float x = -42.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): -42.0
Node 6 (0x613000001950): fract(-42.0)
Node 7 (0x613000001968): half(fract(-42.0))
Node 8 (0x613000001980): (sk_FragColor.x = half(fract(-42.0)))
Node 9 (0x613000001998): (sk_FragColor.x = half(fract(-42.0)));
Exits: [1]

Block 1
-------
Before: [float x = -42.0]
Entrances: [0]
Exits: []

about to simplify ST float x = -42.0 
simplifying vardecl
Block 0
-------
Before: [float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): float ;
Node 2 (0x6130000018f0): sk_FragColor
Node 3 (0x613000001908): sk_FragColor.x
Node 4 (0x613000001920): -42.0
Node 5 (0x613000001938): fract(-42.0)
Node 6 (0x613000001950): half(fract(-42.0))
Node 7 (0x613000001968): (sk_FragColor.x = half(fract(-42.0)))
Node 8 (0x613000001980): (sk_FragColor.x = half(fract(-42.0)));
Exits: [1]

Block 1
-------
Before: [
