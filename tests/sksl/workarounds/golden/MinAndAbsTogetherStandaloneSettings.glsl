### Compilation failed:

=================================================================
==77437==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013cd0 at pc 0x000102fd3c05 bp 0x7ffeecd41ee0 sp 0x7ffeecd41ed8
READ of size 8 at 0x60d000013cd0 thread T0
    #0 0x102fd3c04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x102fd29a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x102fd1fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x103077d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x103086657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x1030836e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x102ebe886 in main SkSLMain.cpp:242
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013cd0 is located 128 bytes inside of 136-byte region [0x60d000013c50,0x60d000013cd8)
freed by thread T0 here:
    #0 0x104beac6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10335b8dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x103198e01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x10307308d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x1030be97d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x103072a86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x10306e4a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x103078012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x103086657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x1030836e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x102ebe886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x104bea84d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x103233fdd in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable*, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable*&&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x1032269a3 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:472
    #3 0x10320d116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x10320b17e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x10321c0e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x10320ca03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #7 0x10324f8cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #8 0x1032b4663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #9 0x103082e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #10 0x102ebe886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c1a00002740: 00 00 00 00 00 00 00 00 00 00 00 00 00 fa fa fa
  0x1c1a00002750: fa fa fa fa fa fa 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002760: 00 00 00 00 00 00 00 fa fa fa fa fa fa fa fa fa
  0x1c1a00002770: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002780: 00 fa fa fa fa fa fa fa fa fa fd fd fd fd fd fd
=>0x1c1a00002790: fd fd fd fd fd fd fd fd fd fd[fd]fa fa fa fa fa
  0x1c1a000027a0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a000027b0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==77437==ABORTING
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -5.0
Node 1 (0x6130000018d8): half x = -5.0
Node 2 (0x6130000018f0): half x = -5.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): x
Node 6 (0x613000001950): abs(x)
Node 7 (0x613000001968): 6.0
Node 8 (0x613000001980): min(abs(x), 6.0)
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(x), 6.0))
Node 10 (0x6130000019b0): (sk_FragColor.x = min(abs(x), 6.0));
Exits: [1]

Block 1
-------
Before: [half x = -5.0]
Entrances: [0]
Exits: []

about to simplify EX -5.0 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -5.0
Node 1 (0x6130000018d8): half x = -5.0
Node 2 (0x6130000018f0): half x = -5.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): x
Node 6 (0x613000001950): abs(x)
Node 7 (0x613000001968): 6.0
Node 8 (0x613000001980): min(abs(x), 6.0)
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(x), 6.0))
Node 10 (0x6130000019b0): (sk_FragColor.x = min(abs(x), 6.0));
Exits: [1]

Block 1
-------
Before: [half x = -5.0]
Entrances: [0]
Exits: []

about to simplify ST half x = -5.0 
simplifying vardecl
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -5.0
Node 1 (0x6130000018d8): half x = -5.0
Node 2 (0x6130000018f0): half x = -5.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): x
Node 6 (0x613000001950): abs(x)
Node 7 (0x613000001968): 6.0
Node 8 (0x613000001980): min(abs(x), 6.0)
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(x), 6.0))
Node 10 (0x6130000019b0): (sk_FragColor.x = min(abs(x), 6.0));
Exits: [1]

Block 1
-------
Before: [half x = -5.0]
Entrances: [0]
Exits: []

about to simplify ST half x = -5.0; 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -5.0
Node 1 (0x6130000018d8): half x = -5.0
Node 2 (0x6130000018f0): half x = -5.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): x
Node 6 (0x613000001950): abs(x)
Node 7 (0x613000001968): 6.0
Node 8 (0x613000001980): min(abs(x), 6.0)
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(x), 6.0))
Node 10 (0x6130000019b0): (sk_FragColor.x = min(abs(x), 6.0));
Exits: [1]

Block 1
-------
Before: [half x = -5.0]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -5.0
Node 1 (0x6130000018d8): half x = -5.0
Node 2 (0x6130000018f0): half x = -5.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): x
Node 6 (0x613000001950): abs(x)
Node 7 (0x613000001968): 6.0
Node 8 (0x613000001980): min(abs(x), 6.0)
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(x), 6.0))
Node 10 (0x6130000019b0): (sk_FragColor.x = min(abs(x), 6.0));
Exits: [1]

Block 1
-------
Before: [half x = -5.0]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor.x 
optimizing swiz 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -5.0
Node 1 (0x6130000018d8): half x = -5.0
Node 2 (0x6130000018f0): half x = -5.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): x
Node 6 (0x613000001950): abs(x)
Node 7 (0x613000001968): 6.0
Node 8 (0x613000001980): min(abs(x), 6.0)
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(x), 6.0))
Node 10 (0x6130000019b0): (sk_FragColor.x = min(abs(x), 6.0));
Exits: [1]

Block 1
-------
Before: [half x = -5.0]
Entrances: [0]
Exits: []

about to simplify EX x 
optimized to -5.0 
coerced to -5.0 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -5.0
Node 1 (0x6130000018d8): half x = -5.0
Node 2 (0x6130000018f0): half x = -5.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): -5.0
Node 6 (0x613000001950): abs(-5.0)
Node 7 (0x613000001968): 6.0
Node 8 (0x613000001980): min(abs(-5.0), 6.0)
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(-5.0), 6.0))
Node 10 (0x6130000019b0): (sk_FragColor.x = min(abs(-5.0), 6.0));
Exits: [1]

Block 1
-------
Before: [half x = -5.0]
Entrances: [0]
Exits: []

about to simplify EX abs(-5.0) 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -5.0
Node 1 (0x6130000018d8): half x = -5.0
Node 2 (0x6130000018f0): half x = -5.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): -5.0
Node 6 (0x613000001950): abs(-5.0)
Node 7 (0x613000001968): 6.0
Node 8 (0x613000001980): min(abs(-5.0), 6.0)
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(-5.0), 6.0))
Node 10 (0x6130000019b0): (sk_FragColor.x = min(abs(-5.0), 6.0));
Exits: [1]

Block 1
-------
Before: [half x = -5.0]
Entrances: [0]
Exits: []

about to simplify EX 6.0 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -5.0
Node 1 (0x6130000018d8): half x = -5.0
Node 2 (0x6130000018f0): half x = -5.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): -5.0
Node 6 (0x613000001950): abs(-5.0)
Node 7 (0x613000001968): 6.0
Node 8 (0x613000001980): min(abs(-5.0), 6.0)
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(-5.0), 6.0))
Node 10 (0x6130000019b0): (sk_FragColor.x = min(abs(-5.0), 6.0));
Exits: [1]

Block 1
-------
Before: [half x = -5.0]
Entrances: [0]
Exits: []

about to simplify EX min(abs(-5.0), 6.0) 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -5.0
Node 1 (0x6130000018d8): half x = -5.0
Node 2 (0x6130000018f0): half x = -5.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): -5.0
Node 6 (0x613000001950): abs(-5.0)
Node 7 (0x613000001968): 6.0
Node 8 (0x613000001980): min(abs(-5.0), 6.0)
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(-5.0), 6.0))
Node 10 (0x6130000019b0): (sk_FragColor.x = min(abs(-5.0), 6.0));
Exits: [1]

Block 1
-------
Before: [half x = -5.0]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor.x = min(abs(-5.0), 6.0)) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -5.0
Node 1 (0x6130000018d8): half x = -5.0
Node 2 (0x6130000018f0): half x = -5.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): -5.0
Node 6 (0x613000001950): abs(-5.0)
Node 7 (0x613000001968): 6.0
Node 8 (0x613000001980): min(abs(-5.0), 6.0)
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(-5.0), 6.0))
Node 10 (0x6130000019b0): (sk_FragColor.x = min(abs(-5.0), 6.0));
Exits: [1]

Block 1
-------
Before: [half x = -5.0]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor.x = min(abs(-5.0), 6.0)); 
simplifying expr
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -5.0
Node 1 (0x6130000018d8): half x = -5.0
Node 2 (0x6130000018f0): half x = -5.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): -5.0
Node 6 (0x613000001950): abs(-5.0)
Node 7 (0x613000001968): 6.0
Node 8 (0x613000001980): min(abs(-5.0), 6.0)
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(-5.0), 6.0))
Node 10 (0x6130000019b0): (sk_FragColor.x = min(abs(-5.0), 6.0));
Exits: [1]

Block 1
-------
Before: [half x = -5.0]
Entrances: [0]
Exits: []

about to simplify EX -5.0 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): -5.0
Node 1 (0x6130000018d8): half x = -5.0
Node 2 (0x6130000018f0): half x = -5.0;
Node 3 (0x613000001908): sk_FragColor
Node 4 (0x613000001920): sk_FragColor.x
Node 5 (0x613000001938): -5.0
Node 6 (0x613000001950): abs(-5.0)
Node 7 (0x613000001968): 6.0
Node 8 (0x613000001980): min(abs(-5.0), 6.0)
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(-5.0), 6.0))
Node 10 (0x6130000019b0): (sk_FragColor.x = min(abs(-5.0), 6.0));
Exits: [1]

Block 1
-------
Before: [half x = -5.0]
Entrances: [0]
Exits: []

about to simplify ST half x = -5.0 
simplifying vardecl
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): half ;
Node 2 (0x6130000018f0): sk_FragColor
Node 3 (0x613000001908): sk_FragColor.x
Node 4 (0x613000001920): -5.0
Node 5 (0x613000001938): abs(-5.0)
Node 6 (0x613000001950): 6.0
Node 7 (0x613000001968): min(abs(-5.0), 6.0)
Node 8 (0x613000001980): (sk_FragColor.x = min(abs(-5.0), 6.0))
Node 9 (0x613000001998): (sk_FragColor.x = min(abs(-5.0), 6.0));
Exits: [1]

Block 1
-------
Before: [
