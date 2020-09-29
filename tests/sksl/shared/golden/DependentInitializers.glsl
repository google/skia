### Compilation failed:

=================================================================
==77517==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013cd0 at pc 0x00010876fc05 bp 0x7ffee75a5f00 sp 0x7ffee75a5ef8
READ of size 8 at 0x60d000013cd0 thread T0
    #0 0x10876fc04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x10876e9a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x10876dfcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x108813d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x108822657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x10881f6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x10865a886 in main SkSLMain.cpp:242
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013cd0 is located 128 bytes inside of 136-byte region [0x60d000013c50,0x60d000013cd8)
freed by thread T0 here:
    #0 0x10a387c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x108af78dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x108934e01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x10880f08d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x10885a97d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x10880ea86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x10880a4a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x108814012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x108822657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x10881f6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x10865a886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x10a38784d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x1089cffdd in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable*, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable*&&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x1089c29a3 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:472
    #3 0x1089a9116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x1089a717e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x1089b80e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x1089a8a03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #7 0x1089eb8cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #8 0x108a50663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #9 0x10881ee67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #10 0x10865a886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c1a00002740: 00 00 00 00 00 00 00 00 00 00 00 00 00 fa fa fa
  0x1c1a00002750: fa fa fa fa fa fa 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002760: 00 00 00 00 00 00 00 fa fa fa fa fa fa fa fa fa
  0x1c1a00002770: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002780: 00 fa fa fa fa fa fa fa fa fa fd fd fd fd fd fd
=>0x1c1a00002790: fd fd fd fd fd fd fd fd fd fd[fd]fa fa fa fa fa
  0x1c1a000027a0: fa fa fa fa 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1a000027b0: 00 00 00 00 00 fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027c0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027d0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027e0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==77517==ABORTING
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.5
Node 1 (0x6130000018d8): float x = 0.5
Node 2 (0x6130000018f0): x
Node 3 (0x613000001908): 2.0
Node 4 (0x613000001920): (x * 2.0)
Node 5 (0x613000001938): float y = (x * 2.0)
Node 6 (0x613000001950): float x = 0.5, y = (x * 2.0);
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): y
Node 9 (0x613000001998): half(y)
Node 10 (0x6130000019b0): half4(half(y))
Node 11 (0x6130000019c8): (sk_FragColor = half4(half(y)))
Node 12 (0x6130000019e0): (sk_FragColor = half4(half(y)));
Exits: [1]

Block 1
-------
Before: [float y = (x * 2.0), float x = 0.5]
Entrances: [0]
Exits: []

about to simplify EX 0.5 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.5
Node 1 (0x6130000018d8): float x = 0.5
Node 2 (0x6130000018f0): x
Node 3 (0x613000001908): 2.0
Node 4 (0x613000001920): (x * 2.0)
Node 5 (0x613000001938): float y = (x * 2.0)
Node 6 (0x613000001950): float x = 0.5, y = (x * 2.0);
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): y
Node 9 (0x613000001998): half(y)
Node 10 (0x6130000019b0): half4(half(y))
Node 11 (0x6130000019c8): (sk_FragColor = half4(half(y)))
Node 12 (0x6130000019e0): (sk_FragColor = half4(half(y)));
Exits: [1]

Block 1
-------
Before: [float y = (x * 2.0), float x = 0.5]
Entrances: [0]
Exits: []

about to simplify ST float x = 0.5 
simplifying vardecl
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.5
Node 1 (0x6130000018d8): float x = 0.5
Node 2 (0x6130000018f0): x
Node 3 (0x613000001908): 2.0
Node 4 (0x613000001920): (x * 2.0)
Node 5 (0x613000001938): float y = (x * 2.0)
Node 6 (0x613000001950): float x = 0.5, y = (x * 2.0);
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): y
Node 9 (0x613000001998): half(y)
Node 10 (0x6130000019b0): half4(half(y))
Node 11 (0x6130000019c8): (sk_FragColor = half4(half(y)))
Node 12 (0x6130000019e0): (sk_FragColor = half4(half(y)));
Exits: [1]

Block 1
-------
Before: [float y = (x * 2.0), float x = 0.5]
Entrances: [0]
Exits: []

about to simplify EX x 
optimized to 0.5 
coerced to 0.5 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.5
Node 1 (0x6130000018d8): float x = 0.5
Node 2 (0x6130000018f0): 0.5
Node 3 (0x613000001908): 2.0
Node 4 (0x613000001920): (0.5 * 2.0)
Node 5 (0x613000001938): float y = (0.5 * 2.0)
Node 6 (0x613000001950): float x = 0.5, y = (0.5 * 2.0);
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): y
Node 9 (0x613000001998): half(y)
Node 10 (0x6130000019b0): half4(half(y))
Node 11 (0x6130000019c8): (sk_FragColor = half4(half(y)))
Node 12 (0x6130000019e0): (sk_FragColor = half4(half(y)));
Exits: [1]

Block 1
-------
Before: [float y = (0.5 * 2.0), float x = 0.5]
Entrances: [0]
Exits: []

about to simplify EX 2.0 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.5
Node 1 (0x6130000018d8): float x = 0.5
Node 2 (0x6130000018f0): 0.5
Node 3 (0x613000001908): 2.0
Node 4 (0x613000001920): (0.5 * 2.0)
Node 5 (0x613000001938): float y = (0.5 * 2.0)
Node 6 (0x613000001950): float x = 0.5, y = (0.5 * 2.0);
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): y
Node 9 (0x613000001998): half(y)
Node 10 (0x6130000019b0): half4(half(y))
Node 11 (0x6130000019c8): (sk_FragColor = half4(half(y)))
Node 12 (0x6130000019e0): (sk_FragColor = half4(half(y)));
Exits: [1]

Block 1
-------
Before: [float y = (0.5 * 2.0), float x = 0.5]
Entrances: [0]
Exits: []

about to simplify EX (0.5 * 2.0) 
optimized to 1.0 
coerced to 1.0 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0.5
Node 1 (0x613000001a98): float x = 0.5
Node 2 (0x613000001ab0): 1.0
Node 3 (0x613000001ac8): float y = 1.0
Node 4 (0x613000001ae0): float x = 0.5, y = 1.0;
Node 5 (0x613000001af8): sk_FragColor
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): half(y)
Node 8 (0x613000001b40): half4(half(y))
Node 9 (0x613000001b58): (sk_FragColor = half4(half(y)))
Node 10 (0x613000001b70): (sk_FragColor = half4(half(y)));
Exits: [1]

Block 1
-------
Before: [float y = 1.0, float x = 0.5]
Entrances: [0]
Exits: []

about to simplify EX 0.5 
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0.5
Node 1 (0x613000001a98): float x = 0.5
Node 2 (0x613000001ab0): 1.0
Node 3 (0x613000001ac8): float y = 1.0
Node 4 (0x613000001ae0): float x = 0.5, y = 1.0;
Node 5 (0x613000001af8): sk_FragColor
Node 6 (0x613000001b10): y
Node 7 (0x613000001b28): half(y)
Node 8 (0x613000001b40): half4(half(y))
Node 9 (0x613000001b58): (sk_FragColor = half4(half(y)))
Node 10 (0x613000001b70): (sk_FragColor = half4(half(y)));
Exits: [1]

Block 1
-------
Before: [float y = 1.0, float x = 0.5]
Entrances: [0]
Exits: []

about to simplify ST float x = 0.5 
simplifying vardecl
Block 0
-------
Before: [float y = <undefined>, float x = <undefined>]
Entrances: []
Node 0 (0x613000001a80): ;
Node 1 (0x613000001a98): 1.0
Node 2 (0x613000001ab0): float y = 1.0
Node 3 (0x613000001ac8): float y = 1.0;
Node 4 (0x613000001ae0): sk_FragColor
Node 5 (0x613000001af8): y
Node 6 (0x613000001b10): half(y)
Node 7 (0x613000001b28): half4(half(y))
Node 8 (0x613000001b40): (sk_FragColor = half4(half(y)))
Node 9 (0x613000001b58): (sk_FragColor = half4(half(y)));
Exits: [1]

Block 1
-------
Before: [float y = 1.0
