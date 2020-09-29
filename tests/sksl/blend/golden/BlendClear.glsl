### Compilation failed:

=================================================================
==78102==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000019e30 at pc 0x0001062f6c05 bp 0x7ffee9a1ee50 sp 0x7ffee9a1ee48
READ of size 8 at 0x602000019e30 thread T0
    #0 0x1062f6c04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x1062f7bad in SkSL::BasicBlock::Node::description() const SkSLCFGGenerator.h:65
    #2 0x1062f6069 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:79
    #3 0x1062f4fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #4 0x10639ad91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #5 0x1063a9657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #6 0x1063a66e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #7 0x1061e1886 in main SkSLMain.cpp:242
    #8 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x602000019e30 is located 0 bytes inside of 16-byte region [0x602000019e30,0x602000019e40)
freed by thread T0 here:
    #0 0x107f14c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x1061fef9c in std::__1::_DeallocateCaller::__do_call(void*) new:334
    #2 0x1061fef40 in std::__1::_DeallocateCaller::__do_deallocate_handle_size(void*, unsigned long) new:292
    #3 0x1065f345c in std::__1::_DeallocateCaller::__do_deallocate_handle_size_align(void*, unsigned long, unsigned long) new:268
    #4 0x1065f33ec in std::__1::__libcpp_deallocate(void*, unsigned long, unsigned long) new:340
    #5 0x10664c8f1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::deallocate(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1872
    #6 0x10664c4dc in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::deallocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1594
    #7 0x10664beab in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~__vector_base() vector:464
    #8 0x1069ac29d in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:555
    #9 0x1069a6371 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:550
    #10 0x1069a62a0 in SkSL::IRNode::~IRNode() SkSLIRNode.cpp:51
    #11 0x1063d64b4 in SkSL::Expression::~Expression() SkSLExpression.h:27
    #12 0x1065f8da4 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #13 0x1065f6eb1 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #14 0x1065f6f11 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #15 0x1064411f1 in std::__1::default_delete<SkSL::Expression>::operator()(SkSL::Expression*) const memory:2368
    #16 0x106440bdd in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::reset(SkSL::Expression*) memory:2623
    #17 0x10638d15d in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator=(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:2542
    #18 0x10638ee24 in SkSL::delete_left(SkSL::BasicBlock*, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, bool*, bool*) SkSLCompiler.cpp:719
    #19 0x10638b4b7 in SkSL::Compiler::simplifyExpression(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:925
    #20 0x10639aeff in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1519
    #21 0x1063a9657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #22 0x1063a66e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #23 0x1061e1886 in main SkSLMain.cpp:242
    #24 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x107f1484d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x1065f1e64 in std::__1::__libcpp_allocate(unsigned long, unsigned long) new:253
    #2 0x1066504b1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::allocate(unsigned long, void const*) memory:1869
    #3 0x10664ffa8 in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::allocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, unsigned long) memory:1586
    #4 0x10664fc0c in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:318
    #5 0x10664ed2b in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:317
    #6 0x1065d0341 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::reserve(unsigned long) vector:1586
    #7 0x1065f5af4 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:55
    #8 0x1065624e7 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:54
    #9 0x106727b5b in std::__1::__unique_if<SkSL::BinaryExpression>::__unique_single std::__1::make_unique<SkSL::BinaryExpression, int&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*>(int&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Token::Kind&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Type const*&&) memory:3033
    #10 0x10672ef24 in SkSL::Inliner::inlineStatement(int, std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > > >*, SkSL::SymbolTable*, SkSL::Expression const*, bool, SkSL::Statement const&) SkSLInliner.cpp:487
    #11 0x10673fb84 in SkSL::Inliner::inlineCall(SkSL::FunctionCall*, SkSL::SymbolTable*) SkSLInliner.cpp:686
    #12 0x10656592f in SkSL::IRGenerator::call(int, SkSL::FunctionDeclaration const&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >) SkSLIRGenerator.cpp:2115
    #13 0x106569503 in SkSL::IRGenerator::call(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >) SkSLIRGenerator.cpp:2206
    #14 0x106588521 in SkSL::IRGenerator::convertCallExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:2742
    #15 0x106553eaa in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1318
    #16 0x106585742 in SkSL::IRGenerator::convertBinaryExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1920
    #17 0x106553c39 in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1313
    #18 0x10653ae58 in SkSL::IRGenerator::convertExpressionStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:697
    #19 0x10652e57b in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:243
    #20 0x10653f0e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #21 0x10652fa03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #22 0x1065728cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #23 0x1065d7663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #24 0x1063a5e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #25 0x1061e1886 in main SkSLMain.cpp:242
    #26 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c0400003370: fa fa fd fa fa fa fd fa fa fa fd fa fa fa fd fa
  0x1c0400003380: fa fa fd fa fa fa fd fa fa fa fd fd fa fa 00 00
  0x1c0400003390: fa fa 00 fa fa fa 00 fa fa fa fd fd fa fa fd fa
  0x1c04000033a0: fa fa fd fd fa fa 00 fa fa fa 00 00 fa fa 00 fa
  0x1c04000033b0: fa fa fd fa fa fa 00 fa fa fa fd fd fa fa 00 fa
=>0x1c04000033c0: fa fa 00 fa fa fa[fd]fd fa fa fd fa fa fa 00 00
  0x1c04000033d0: fa fa 00 00 fa fa 00 fa fa fa 00 00 fa fa fd fd
  0x1c04000033e0: fa fa 00 00 fa fa fd fd fa fa fd fd fa fa 00 fa
  0x1c04000033f0: fa fa fd fd fa fa 00 00 fa fa fa fa fa fa fa fa
  0x1c0400003400: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c0400003410: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==78102==ABORTING
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000102a0): 0.0
Node 1 (0x6080000102b8): half4(0.0)
Node 2 (0x6080000102d0): return half4(0.0);
Exits: []

Block 1
-------
Before: []
Entrances: []
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000102a0): 0.0
Node 1 (0x6080000102b8): half4(0.0)
Node 2 (0x6080000102d0): return half4(0.0);
Exits: []

Block 1
-------
Before: []
Entrances: []
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify EX half4(0.0) 
Block 0
-------
Before: []
Entrances: []
Node 0 (0x6080000102a0): 0.0
Node 1 (0x6080000102b8): half4(0.0)
Node 2 (0x6080000102d0): return half4(0.0);
Exits: []

Block 1
-------
Before: []
Entrances: []
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify ST return half4(0.0); 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): half4 _0_blend_clear
Node 1 (0x6130000018d8): half4 _0_blend_clear;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): _0_blend_clear
Node 9 (0x613000001998): (sk_FragColor = _0_blend_clear)
Node 10 (0x6130000019b0): (sk_FragColor = _0_blend_clear);
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify ST half4 _0_blend_clear 
simplifying vardecl
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): half4 _0_blend_clear
Node 1 (0x6130000018d8): half4 _0_blend_clear;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): _0_blend_clear
Node 9 (0x613000001998): (sk_FragColor = _0_blend_clear)
Node 10 (0x6130000019b0): (sk_FragColor = _0_blend_clear);
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify ST half4 _0_blend_clear; 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): half4 _0_blend_clear
Node 1 (0x6130000018d8): half4 _0_blend_clear;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): _0_blend_clear
Node 9 (0x613000001998): (sk_FragColor = _0_blend_clear)
Node 10 (0x6130000019b0): (sk_FragColor = _0_blend_clear);
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify EX _0_blend_clear 
optimizing varref 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): half4 _0_blend_clear
Node 1 (0x6130000018d8): half4 _0_blend_clear;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): _0_blend_clear
Node 9 (0x613000001998): (sk_FragColor = _0_blend_clear)
Node 10 (0x6130000019b0): (sk_FragColor = _0_blend_clear);
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): half4 _0_blend_clear
Node 1 (0x6130000018d8): half4 _0_blend_clear;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): _0_blend_clear
Node 9 (0x613000001998): (sk_FragColor = _0_blend_clear)
Node 10 (0x6130000019b0): (sk_FragColor = _0_blend_clear);
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify EX half4(0.0) 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): half4 _0_blend_clear
Node 1 (0x6130000018d8): half4 _0_blend_clear;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): _0_blend_clear
Node 9 (0x613000001998): (sk_FragColor = _0_blend_clear)
Node 10 (0x6130000019b0): (sk_FragColor = _0_blend_clear);
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify EX (_0_blend_clear = half4(0.0)) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): half4 _0_blend_clear
Node 1 (0x6130000018d8): half4 _0_blend_clear;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): _0_blend_clear
Node 9 (0x613000001998): (sk_FragColor = _0_blend_clear)
Node 10 (0x6130000019b0): (sk_FragColor = _0_blend_clear);
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify ST (_0_blend_clear = half4(0.0)); 
simplifying expr
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): half4 _0_blend_clear
Node 1 (0x6130000018d8): half4 _0_blend_clear;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): _0_blend_clear
Node 9 (0x613000001998): (sk_FragColor = _0_blend_clear)
Node 10 (0x6130000019b0): (sk_FragColor = _0_blend_clear);
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify EX sk_FragColor 
optimizing varref 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): half4 _0_blend_clear
Node 1 (0x6130000018d8): half4 _0_blend_clear;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): _0_blend_clear
Node 9 (0x613000001998): (sk_FragColor = _0_blend_clear)
Node 10 (0x6130000019b0): (sk_FragColor = _0_blend_clear);
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify EX _0_blend_clear 
optimized to half4(0.0) 
coerced to half4(0.0) 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): half4 _0_blend_clear
Node 1 (0x6130000018d8): half4 _0_blend_clear;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): 0.0
Node 9 (0x613000001998): half4(0.0)
Node 10 (0x6130000019b0): (sk_FragColor = half4(0.0))
Node 11 (0x6130000019c8): (sk_FragColor = half4(0.0));
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify EX (sk_FragColor = half4(0.0)) 
optimizing binary 
deadass? update=1 rescan=0 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): half4 _0_blend_clear
Node 1 (0x6130000018d8): half4 _0_blend_clear;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): 0.0
Node 9 (0x613000001998): half4(0.0)
Node 10 (0x6130000019b0): (sk_FragColor = half4(0.0))
Node 11 (0x6130000019c8): (sk_FragColor = half4(0.0));
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify ST (sk_FragColor = half4(0.0)); 
simplifying expr
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): half4 _0_blend_clear
Node 1 (0x6130000018d8): half4 _0_blend_clear;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): 0.0
Node 9 (0x613000001998): half4(0.0)
Node 10 (0x6130000019b0): (sk_FragColor = half4(0.0))
Node 11 (0x6130000019c8): (sk_FragColor = half4(0.0));
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify ST half4 _0_blend_clear 
simplifying vardecl
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): half4 ;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): 0.0
Node 9 (0x613000001998): half4(0.0)
Node 10 (0x6130000019b0): (sk_FragColor = half4(0.0))
Node 11 (0x6130000019c8): (sk_FragColor = half4(0.0));
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify ST half4 ; 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): half4 ;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): 0.0
Node 9 (0x613000001998): half4(0.0)
Node 10 (0x6130000019b0): (sk_FragColor = half4(0.0))
Node 11 (0x6130000019c8): (sk_FragColor = half4(0.0));
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify EX _0_blend_clear 
optimizing varref 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): half4 ;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): 0.0
Node 9 (0x613000001998): half4(0.0)
Node 10 (0x6130000019b0): (sk_FragColor = half4(0.0))
Node 11 (0x6130000019c8): (sk_FragColor = half4(0.0));
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify EX 0.0 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): half4 ;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): 0.0
Node 9 (0x613000001998): half4(0.0)
Node 10 (0x6130000019b0): (sk_FragColor = half4(0.0))
Node 11 (0x6130000019c8): (sk_FragColor = half4(0.0));
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify EX half4(0.0) 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): half4 ;
Node 2 (0x6130000018f0): _0_blend_clear
Node 3 (0x613000001908): 0.0
Node 4 (0x613000001920): half4(0.0)
Node 5 (0x613000001938): (_0_blend_clear = half4(0.0))
Node 6 (0x613000001950): (_0_blend_clear = half4(0.0));
Node 7 (0x613000001968): sk_FragColor
Node 8 (0x613000001980): 0.0
Node 9 (0x613000001998): half4(0.0)
Node 10 (0x6130000019b0): (sk_FragColor = half4(0.0))
Node 11 (0x6130000019c8): (sk_FragColor = half4(0.0));
Exits: [1]

Block 1
-------
Before: [half4 _0_blend_clear = half4(0.0)]
Entrances: [0]
Exits: []

about to simplify EX (_0_blend_clear = half4(0.0)) 
optimizing binary 
deadass? update=1 rescan=0 
deadass! update=1 rescan=0 
delete_left update=1 rescan=0 
Block 0
-------
Before: [half4 _0_blend_clear = <undefined>]
Entrances: []
Node 0 (0x6130000018c0): ;
Node 1 (0x6130000018d8): half4 ;
