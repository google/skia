### Compilation failed:

=================================================================
==76948==ERROR: AddressSanitizer: heap-use-after-free on address 0x60b000026e20 at pc 0x00010d533c05 bp 0x7ffee27e1e10 sp 0x7ffee27e1e08
READ of size 8 at 0x60b000026e20 thread T0
    #0 0x10d533c04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x10d534bad in SkSL::BasicBlock::Node::description() const SkSLCFGGenerator.h:65
    #2 0x10d533069 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:79
    #3 0x10d531fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #4 0x10d5d7d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #5 0x10d5e6657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #6 0x10d5e36e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #7 0x10d41e886 in main SkSLMain.cpp:242
    #8 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60b000026e20 is located 96 bytes inside of 112-byte region [0x60b000026dc0,0x60b000026e30)
freed by thread T0 here:
    #0 0x10f14dc6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10d9510fd in SkSL::IndexExpression::~IndexExpression() SkSLIndexExpression.h:44
    #2 0x10d87f4f1 in std::__1::default_delete<SkSL::Expression>::operator()(SkSL::Expression*) const memory:2368
    #3 0x10d87f17d in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::reset(SkSL::Expression*) memory:2623
    #4 0x10d87ef55 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::~unique_ptr() memory:2577
    #5 0x10d791931 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::~unique_ptr() memory:2577
    #6 0x10d952690 in SkSL::IndexExpression::~IndexExpression() SkSLIndexExpression.h:44
    #7 0x10d951091 in SkSL::IndexExpression::~IndexExpression() SkSLIndexExpression.h:44
    #8 0x10d9510f1 in SkSL::IndexExpression::~IndexExpression() SkSLIndexExpression.h:44
    #9 0x10d87f4f1 in std::__1::default_delete<SkSL::Expression>::operator()(SkSL::Expression*) const memory:2368
    #10 0x10d87f17d in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::reset(SkSL::Expression*) memory:2623
    #11 0x10d87ef55 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::~unique_ptr() memory:2577
    #12 0x10d791931 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::~unique_ptr() memory:2577
    #13 0x10d889897 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::destroy(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*) memory:1936
    #14 0x10d889814 in void std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::__destroy<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(std::__1::integral_constant<bool, true>, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*) memory:1798
    #15 0x10d88977c in void std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::destroy<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*) memory:1635
    #16 0x10d88960f in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::__destruct_at_end(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*) vector:426
    #17 0x10d8894a0 in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::clear() vector:369
    #18 0x10d888e38 in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~__vector_base() vector:463
    #19 0x10dbe929d in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:555
    #20 0x10dbe3371 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:550
    #21 0x10dbe32a0 in SkSL::IRNode::~IRNode() SkSLIRNode.cpp:51
    #22 0x10d6134b4 in SkSL::Expression::~Expression() SkSLExpression.h:27
    #23 0x10d835da4 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #24 0x10d833eb1 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #25 0x10d833f11 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #26 0x10d67e1f1 in std::__1::default_delete<SkSL::Expression>::operator()(SkSL::Expression*) const memory:2368
    #27 0x10d67dbdd in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::reset(SkSL::Expression*) memory:2623
    #28 0x10d5ca15d in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator=(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:2542
    #29 0x10d5cbe24 in SkSL::delete_left(SkSL::BasicBlock*, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, bool*, bool*) SkSLCompiler.cpp:719

previously allocated by thread T0 here:
    #0 0x10f14d84d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x10d8054f9 in std::__1::__unique_if<SkSL::IndexExpression>::__unique_single std::__1::make_unique<SkSL::IndexExpression, SkSL::Context const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Context const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x10d804f69 in SkSL::IRGenerator::convertIndex(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::ASTNode const&) SkSLIRGenerator.cpp:2463
    #3 0x10d7cb3ab in SkSL::IRGenerator::convertIndexExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:2716
    #4 0x10d79121f in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1327
    #5 0x10d7cb03e in SkSL::IRGenerator::convertIndexExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:2711
    #6 0x10d79121f in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1327
    #7 0x10d7c22b3 in SkSL::IRGenerator::convertBinaryExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1909
    #8 0x10d790c39 in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1313
    #9 0x10d777e58 in SkSL::IRGenerator::convertExpressionStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:697
    #10 0x10d76b57b in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:243
    #11 0x10d77c0e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #12 0x10d76ca03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #13 0x10d7af8cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #14 0x10d814663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #15 0x10d5e2e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #16 0x10d41e886 in main SkSLMain.cpp:242
    #17 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c1600004d70: fa fa fa fa fa fa 00 00 00 00 00 00 00 00 00 00
  0x1c1600004d80: 00 00 00 fa fa fa fa fa fa fa fa fa fd fd fd fd
  0x1c1600004d90: fd fd fd fd fd fd fd fd fd fd fa fa fa fa fa fa
  0x1c1600004da0: fa fa fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x1c1600004db0: fa fa fa fa fa fa fa fa fd fd fd fd fd fd fd fd
=>0x1c1600004dc0: fd fd fd fd[fd]fd fa fa fa fa fa fa fa fa fd fd
  0x1c1600004dd0: fd fd fd fd fd fd fd fd fd fd fd fd fa fa fa fa
  0x1c1600004de0: fa fa fa fa 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1600004df0: 00 fa fa fa fa fa fa fa fa fa fd fd fd fd fd fd
  0x1c1600004e00: fd fd fd fd fd fd fd fd fa fa fa fa fa fa fa fa
  0x1c1600004e10: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==76948==ABORTING
Block 0
-------
Before: [int i = <undefined>, int[2][2] x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): int[2][2] x[2][2]
Node 1 (0x6130000018d8): int x;
Node 2 (0x6130000018f0): int i
Node 3 (0x613000001908): int i;
Node 4 (0x613000001920): x
Node 5 (0x613000001938): i
Node 6 (0x613000001950): x[i]
Node 7 (0x613000001968): 1
Node 8 (0x613000001980): x[i][1]
Node 9 (0x613000001998): 4
Node 10 (0x6130000019b0): (x[i][1] = 4)
Node 11 (0x6130000019c8): (x[i][1] = 4);
Exits: [1]

Block 1
-------
Before: [int i = <undefined>, int[2][2] x = <defined>]
Entrances: [0]
Exits: []

about to simplify ST int[2][2] x[2][2] 
simplifying vardecl
Block 0
-------
Before: [int i = <undefined>, int[2][2] x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): int ;
Node 2 (0x6130000018f0): int i
Node 3 (0x613000001908): int i;
Node 4 (0x613000001920): x
Node 5 (0x613000001938): i
Node 6 (0x613000001950): x[i]
Node 7 (0x613000001968): 1
Node 8 (0x613000001980): x[i][1]
Node 9 (0x613000001998): 4
Node 10 (0x6130000019b0): (x[i][1] = 4)
Node 11 (0x6130000019c8): (x[i][1] = 4);
Exits: [1]

Block 1
-------
Before: [int i = <undefined>, int[2][2] x = <defined>]
Entrances: [0]
Exits: []

about to simplify ST int ; 
Block 0
-------
Before: [int i = <undefined>, int[2][2] x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): int ;
Node 2 (0x6130000018f0): int i
Node 3 (0x613000001908): int i;
Node 4 (0x613000001920): x
Node 5 (0x613000001938): i
Node 6 (0x613000001950): x[i]
Node 7 (0x613000001968): 1
Node 8 (0x613000001980): x[i][1]
Node 9 (0x613000001998): 4
Node 10 (0x6130000019b0): (x[i][1] = 4)
Node 11 (0x6130000019c8): (x[i][1] = 4);
Exits: [1]

Block 1
-------
Before: [int i = <undefined>, int[2][2] x = <defined>]
Entrances: [0]
Exits: []

about to simplify ST int i 
simplifying vardecl
Block 0
-------
Before: [int i = <undefined>, int[2][2] x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): int ;
Node 2 (0x6130000018f0): ;
Node 3 (0x613000001908): int ;
Node 4 (0x613000001920): x
Node 5 (0x613000001938): i
Node 6 (0x613000001950): x[i]
Node 7 (0x613000001968): 1
Node 8 (0x613000001980): x[i][1]
Node 9 (0x613000001998): 4
Node 10 (0x6130000019b0): (x[i][1] = 4)
Node 11 (0x6130000019c8): (x[i][1] = 4);
Exits: [1]

Block 1
-------
Before: [int i = <undefined>, int[2][2] x = <defined>]
Entrances: [0]
Exits: []

about to simplify ST int ; 
Block 0
-------
Before: [int i = <undefined>, int[2][2] x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): int ;
Node 2 (0x6130000018f0): ;
Node 3 (0x613000001908): int ;
Node 4 (0x613000001920): x
Node 5 (0x613000001938): i
Node 6 (0x613000001950): x[i]
Node 7 (0x613000001968): 1
Node 8 (0x613000001980): x[i][1]
Node 9 (0x613000001998): 4
Node 10 (0x6130000019b0): (x[i][1] = 4)
Node 11 (0x6130000019c8): (x[i][1] = 4);
Exits: [1]

Block 1
-------
Before: [int i = <undefined>, int[2][2] x = <defined>]
Entrances: [0]
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [int i = <undefined>, int[2][2] x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): int ;
Node 2 (0x6130000018f0): ;
Node 3 (0x613000001908): int ;
Node 4 (0x613000001920): x
Node 5 (0x613000001938): i
Node 6 (0x613000001950): x[i]
Node 7 (0x613000001968): 1
Node 8 (0x613000001980): x[i][1]
Node 9 (0x613000001998): 4
Node 10 (0x6130000019b0): (x[i][1] = 4)
Node 11 (0x6130000019c8): (x[i][1] = 4);
Exits: [1]

Block 1
-------
Before: [int i = <undefined>, int[2][2] x = <defined>]
Entrances: [0]
Exits: []

about to simplify EX i 
optimizing varref 
Block 0
-------
Before: [int i = <undefined>, int[2][2] x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): int ;
Node 2 (0x6130000018f0): ;
Node 3 (0x613000001908): int ;
Node 4 (0x613000001920): x
Node 5 (0x613000001938): i
Node 6 (0x613000001950): x[i]
Node 7 (0x613000001968): 1
Node 8 (0x613000001980): x[i][1]
Node 9 (0x613000001998): 4
Node 10 (0x6130000019b0): (x[i][1] = 4)
Node 11 (0x6130000019c8): (x[i][1] = 4);
Exits: [1]

Block 1
-------
Before: [int i = <undefined>, int[2][2] x = <defined>]
Entrances: [0]
Exits: []

about to simplify EX x[i] 
Block 0
-------
Before: [int i = <undefined>, int[2][2] x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): int ;
Node 2 (0x6130000018f0): ;
Node 3 (0x613000001908): int ;
Node 4 (0x613000001920): x
Node 5 (0x613000001938): i
Node 6 (0x613000001950): x[i]
Node 7 (0x613000001968): 1
Node 8 (0x613000001980): x[i][1]
Node 9 (0x613000001998): 4
Node 10 (0x6130000019b0): (x[i][1] = 4)
Node 11 (0x6130000019c8): (x[i][1] = 4);
Exits: [1]

Block 1
-------
Before: [int i = <undefined>, int[2][2] x = <defined>]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [int i = <undefined>, int[2][2] x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): int ;
Node 2 (0x6130000018f0): ;
Node 3 (0x613000001908): int ;
Node 4 (0x613000001920): x
Node 5 (0x613000001938): i
Node 6 (0x613000001950): x[i]
Node 7 (0x613000001968): 1
Node 8 (0x613000001980): x[i][1]
Node 9 (0x613000001998): 4
Node 10 (0x6130000019b0): (x[i][1] = 4)
Node 11 (0x6130000019c8): (x[i][1] = 4);
Exits: [1]

Block 1
-------
Before: [int i = <undefined>, int[2][2] x = <defined>]
Entrances: [0]
Exits: []

about to simplify EX x[i][1] 
Block 0
-------
Before: [int i = <undefined>, int[2][2] x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): int ;
Node 2 (0x6130000018f0): ;
Node 3 (0x613000001908): int ;
Node 4 (0x613000001920): x
Node 5 (0x613000001938): i
Node 6 (0x613000001950): x[i]
Node 7 (0x613000001968): 1
Node 8 (0x613000001980): x[i][1]
Node 9 (0x613000001998): 4
Node 10 (0x6130000019b0): (x[i][1] = 4)
Node 11 (0x6130000019c8): (x[i][1] = 4);
Exits: [1]

Block 1
-------
Before: [int i = <undefined>, int[2][2] x = <defined>]
Entrances: [0]
Exits: []

about to simplify EX 4 
Block 0
-------
Before: [int i = <undefined>, int[2][2] x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): int ;
Node 2 (0x6130000018f0): ;
Node 3 (0x613000001908): int ;
Node 4 (0x613000001920): x
Node 5 (0x613000001938): i
Node 6 (0x613000001950): x[i]
Node 7 (0x613000001968): 1
Node 8 (0x613000001980): x[i][1]
Node 9 (0x613000001998): 4
Node 10 (0x6130000019b0): (x[i][1] = 4)
Node 11 (0x6130000019c8): (x[i][1] = 4);
Exits: [1]

Block 1
-------
Before: [int i = <undefined>, int[2][2] x = <defined>]
Entrances: [0]
Exits: []

about to simplify EX (x[i][1] = 4) 
optimizing binary 
deadass? update=1 rescan=0 
deadass! update=1 rescan=0 
delete_left update=1 rescan=0 
Block 0
-------
Before: [int i = <undefined>, int[2][2] x = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): int ;
Node 2 (0x6130000018f0): ;
Node 3 (0x613000001908): int ;
