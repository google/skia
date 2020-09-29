### Compilation failed:

=================================================================
==77496==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013cd0 at pc 0x000101360c05 bp 0x7ffeee9b4f00 sp 0x7ffeee9b4ef8
READ of size 8 at 0x60d000013cd0 thread T0
    #0 0x101360c04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x10135f9a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x10135efcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x101404d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x101413657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x1014106e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x10124c108 in main SkSLMain.cpp:258
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013cd0 is located 128 bytes inside of 136-byte region [0x60d000013c50,0x60d000013cd8)
freed by thread T0 here:
    #0 0x102f7dc6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x1016e88dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x101525e01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x10140008d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x10144b97d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x1013ffa86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x1013fb4a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x101405012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x101413657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x1014106e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x10124c108 in main SkSLMain.cpp:258
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x102f7d84d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x1015c0fdd in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable*, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable*&&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x1015b39a3 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:472
    #3 0x10159a116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x10159817e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x1015a90e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x101599a03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #7 0x1015dc8cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #8 0x101641663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #9 0x10140fe67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #10 0x10124c108 in main SkSLMain.cpp:258
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
  0x1c1a000027b0: 00 00 00 00 00 fa fa fa fa fa fa fa fa fa 00 00
  0x1c1a000027c0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 fa
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
==77496==ABORTING
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.0
Node 1 (0x6130000018d8): float4(0.0)
Node 2 (0x6130000018f0): const float4 a = float4(0.0)
Node 3 (0x613000001908): const float4 a = float4(0.0);
Node 4 (0x613000001920): 1.0
Node 5 (0x613000001938): float4(1.0)
Node 6 (0x613000001950): const float4 b = float4(1.0)
Node 7 (0x613000001968): const float4 b = float4(1.0);
Node 8 (0x613000001980): a
Node 9 (0x613000001998): b
Node 10 (0x6130000019b0): (a == b)
Node 11 (0x6130000019c8): if ((a == b)) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000a360): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.0
Node 1 (0x6130000018d8): float4(0.0)
Node 2 (0x6130000018f0): const float4 a = float4(0.0)
Node 3 (0x613000001908): const float4 a = float4(0.0);
Node 4 (0x613000001920): 1.0
Node 5 (0x613000001938): float4(1.0)
Node 6 (0x613000001950): const float4 b = float4(1.0)
Node 7 (0x613000001968): const float4 b = float4(1.0);
Node 8 (0x613000001980): a
Node 9 (0x613000001998): b
Node 10 (0x6130000019b0): (a == b)
Node 11 (0x6130000019c8): if ((a == b)) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000a360): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify EX float4(0.0) 
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.0
Node 1 (0x6130000018d8): float4(0.0)
Node 2 (0x6130000018f0): const float4 a = float4(0.0)
Node 3 (0x613000001908): const float4 a = float4(0.0);
Node 4 (0x613000001920): 1.0
Node 5 (0x613000001938): float4(1.0)
Node 6 (0x613000001950): const float4 b = float4(1.0)
Node 7 (0x613000001968): const float4 b = float4(1.0);
Node 8 (0x613000001980): a
Node 9 (0x613000001998): b
Node 10 (0x6130000019b0): (a == b)
Node 11 (0x6130000019c8): if ((a == b)) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000a360): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify ST const float4 a = float4(0.0) 
simplifying vardecl
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.0
Node 1 (0x6130000018d8): float4(0.0)
Node 2 (0x6130000018f0): const float4 a = float4(0.0)
Node 3 (0x613000001908): const float4 a = float4(0.0);
Node 4 (0x613000001920): 1.0
Node 5 (0x613000001938): float4(1.0)
Node 6 (0x613000001950): const float4 b = float4(1.0)
Node 7 (0x613000001968): const float4 b = float4(1.0);
Node 8 (0x613000001980): a
Node 9 (0x613000001998): b
Node 10 (0x6130000019b0): (a == b)
Node 11 (0x6130000019c8): if ((a == b)) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000a360): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify ST const float4 a = float4(0.0); 
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.0
Node 1 (0x6130000018d8): float4(0.0)
Node 2 (0x6130000018f0): const float4 a = float4(0.0)
Node 3 (0x613000001908): const float4 a = float4(0.0);
Node 4 (0x613000001920): 1.0
Node 5 (0x613000001938): float4(1.0)
Node 6 (0x613000001950): const float4 b = float4(1.0)
Node 7 (0x613000001968): const float4 b = float4(1.0);
Node 8 (0x613000001980): a
Node 9 (0x613000001998): b
Node 10 (0x6130000019b0): (a == b)
Node 11 (0x6130000019c8): if ((a == b)) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000a360): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.0
Node 1 (0x6130000018d8): float4(0.0)
Node 2 (0x6130000018f0): const float4 a = float4(0.0)
Node 3 (0x613000001908): const float4 a = float4(0.0);
Node 4 (0x613000001920): 1.0
Node 5 (0x613000001938): float4(1.0)
Node 6 (0x613000001950): const float4 b = float4(1.0)
Node 7 (0x613000001968): const float4 b = float4(1.0);
Node 8 (0x613000001980): a
Node 9 (0x613000001998): b
Node 10 (0x6130000019b0): (a == b)
Node 11 (0x6130000019c8): if ((a == b)) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000a360): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify EX float4(1.0) 
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.0
Node 1 (0x6130000018d8): float4(0.0)
Node 2 (0x6130000018f0): const float4 a = float4(0.0)
Node 3 (0x613000001908): const float4 a = float4(0.0);
Node 4 (0x613000001920): 1.0
Node 5 (0x613000001938): float4(1.0)
Node 6 (0x613000001950): const float4 b = float4(1.0)
Node 7 (0x613000001968): const float4 b = float4(1.0);
Node 8 (0x613000001980): a
Node 9 (0x613000001998): b
Node 10 (0x6130000019b0): (a == b)
Node 11 (0x6130000019c8): if ((a == b)) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000a360): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify ST const float4 b = float4(1.0) 
simplifying vardecl
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.0
Node 1 (0x6130000018d8): float4(0.0)
Node 2 (0x6130000018f0): const float4 a = float4(0.0)
Node 3 (0x613000001908): const float4 a = float4(0.0);
Node 4 (0x613000001920): 1.0
Node 5 (0x613000001938): float4(1.0)
Node 6 (0x613000001950): const float4 b = float4(1.0)
Node 7 (0x613000001968): const float4 b = float4(1.0);
Node 8 (0x613000001980): a
Node 9 (0x613000001998): b
Node 10 (0x6130000019b0): (a == b)
Node 11 (0x6130000019c8): if ((a == b)) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000a360): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify ST const float4 b = float4(1.0); 
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.0
Node 1 (0x6130000018d8): float4(0.0)
Node 2 (0x6130000018f0): const float4 a = float4(0.0)
Node 3 (0x613000001908): const float4 a = float4(0.0);
Node 4 (0x613000001920): 1.0
Node 5 (0x613000001938): float4(1.0)
Node 6 (0x613000001950): const float4 b = float4(1.0)
Node 7 (0x613000001968): const float4 b = float4(1.0);
Node 8 (0x613000001980): a
Node 9 (0x613000001998): b
Node 10 (0x6130000019b0): (a == b)
Node 11 (0x6130000019c8): if ((a == b)) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000a360): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify EX a 
optimized to float4(0.0) 
coerced to float4(0.0) 
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.0
Node 1 (0x6130000018d8): float4(0.0)
Node 2 (0x6130000018f0): const float4 a = float4(0.0)
Node 3 (0x613000001908): const float4 a = float4(0.0);
Node 4 (0x613000001920): 1.0
Node 5 (0x613000001938): float4(1.0)
Node 6 (0x613000001950): const float4 b = float4(1.0)
Node 7 (0x613000001968): const float4 b = float4(1.0);
Node 8 (0x613000001980): 0.0
Node 9 (0x613000001998): float4(0.0)
Node 10 (0x6130000019b0): b
Node 11 (0x6130000019c8): (float4(0.0) == b)
Node 12 (0x6130000019e0): if ((float4(0.0) == b)) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000a360): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify EX b 
optimized to float4(1.0) 
coerced to float4(1.0) 
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): 0.0
Node 1 (0x6130000018d8): float4(0.0)
Node 2 (0x6130000018f0): const float4 a = float4(0.0)
Node 3 (0x613000001908): const float4 a = float4(0.0);
Node 4 (0x613000001920): 1.0
Node 5 (0x613000001938): float4(1.0)
Node 6 (0x613000001950): const float4 b = float4(1.0)
Node 7 (0x613000001968): const float4 b = float4(1.0);
Node 8 (0x613000001980): 0.0
Node 9 (0x613000001998): float4(0.0)
Node 10 (0x6130000019b0): 1.0
Node 11 (0x6130000019c8): float4(1.0)
Node 12 (0x6130000019e0): (float4(0.0) == float4(1.0))
Node 13 (0x6130000019f8): if ((float4(0.0) == float4(1.0))) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000a360): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify EX (float4(0.0) == float4(1.0)) 
optimized to false 
coerced to false 
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0.0
Node 1 (0x613000001a98): float4(0.0)
Node 2 (0x613000001ab0): const float4 a = float4(0.0)
Node 3 (0x613000001ac8): const float4 a = float4(0.0);
Node 4 (0x613000001ae0): 1.0
Node 5 (0x613000001af8): float4(1.0)
Node 6 (0x613000001b10): const float4 b = float4(1.0)
Node 7 (0x613000001b28): const float4 b = float4(1.0);
Node 8 (0x613000001b40): false
Node 9 (0x613000001b58): if (false) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000ac90): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0.0
Node 1 (0x613000001a98): float4(0.0)
Node 2 (0x613000001ab0): const float4 a = float4(0.0)
Node 3 (0x613000001ac8): const float4 a = float4(0.0);
Node 4 (0x613000001ae0): 1.0
Node 5 (0x613000001af8): float4(1.0)
Node 6 (0x613000001b10): const float4 b = float4(1.0)
Node 7 (0x613000001b28): const float4 b = float4(1.0);
Node 8 (0x613000001b40): false
Node 9 (0x613000001b58): if (false) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000ac90): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify EX float4(0.0) 
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x613000001a80): 0.0
Node 1 (0x613000001a98): float4(0.0)
Node 2 (0x613000001ab0): const float4 a = float4(0.0)
Node 3 (0x613000001ac8): const float4 a = float4(0.0);
Node 4 (0x613000001ae0): 1.0
Node 5 (0x613000001af8): float4(1.0)
Node 6 (0x613000001b10): const float4 b = float4(1.0)
Node 7 (0x613000001b28): const float4 b = float4(1.0);
Node 8 (0x613000001b40): false
Node 9 (0x613000001b58): if (false) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Node 0 (0x60300000ac90): discard;
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [0]
Exits: [4]

Block 4
-------
Before: [const float4 b = float4(1.0), const float4 a = float4(0.0)]
Entrances: [3]
Exits: []

about to simplify ST const float4 a = float4(0.0) 
simplifying vardecl
Block 0
-------
Before: [const float4 b = <undefined>, const float4 a = <undefined>]
Entrances: []
Node 0 (0x613000001a80): ;
Node 1 (0x613000001a98): float4 ;
Node 2 (0x613000001ab0): 1.0
Node 3 (0x613000001ac8): float4(1.0)
Node 4 (0x613000001ae0): const float4 b = float4(1.0)
Node 5 (0x613000001af8): const float4 b = float4(1.0);
Node 6 (0x613000001b10): false
Node 7 (0x613000001b28): if (false) {
discard;
}

Exits: [1, 3]

Block 1
-------
Before: [const float4 b = float4(1.0)
