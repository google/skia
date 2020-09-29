### Compilation failed:

=================================================================
==77539==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000019c70 at pc 0x00010a80fc05 bp 0x7ffee5505e50 sp 0x7ffee5505e48
READ of size 8 at 0x602000019c70 thread T0
    #0 0x10a80fc04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x10a810bad in SkSL::BasicBlock::Node::description() const SkSLCFGGenerator.h:65
    #2 0x10a80f069 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:79
    #3 0x10a80dfcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #4 0x10a8b3d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #5 0x10a8c2657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #6 0x10a8bf6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #7 0x10a6fb108 in main SkSLMain.cpp:258
    #8 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x602000019c70 is located 0 bytes inside of 16-byte region [0x602000019c70,0x602000019c80)
freed by thread T0 here:
    #0 0x10c42bc6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10a717f9c in std::__1::_DeallocateCaller::__do_call(void*) new:334
    #2 0x10a717f40 in std::__1::_DeallocateCaller::__do_deallocate_handle_size(void*, unsigned long) new:292
    #3 0x10ab0c45c in std::__1::_DeallocateCaller::__do_deallocate_handle_size_align(void*, unsigned long, unsigned long) new:268
    #4 0x10ab0c3ec in std::__1::__libcpp_deallocate(void*, unsigned long, unsigned long) new:340
    #5 0x10ab658f1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::deallocate(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1872
    #6 0x10ab654dc in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::deallocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1594
    #7 0x10ab64eab in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~__vector_base() vector:464
    #8 0x10aec529d in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:555
    #9 0x10aebf371 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:550
    #10 0x10aebf2a0 in SkSL::IRNode::~IRNode() SkSLIRNode.cpp:51
    #11 0x10a8ef4b4 in SkSL::Expression::~Expression() SkSLExpression.h:27
    #12 0x10ab11da4 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #13 0x10ab0feb1 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #14 0x10ab0ff11 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #15 0x10a95a1f1 in std::__1::default_delete<SkSL::Expression>::operator()(SkSL::Expression*) const memory:2368
    #16 0x10a959bdd in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::reset(SkSL::Expression*) memory:2623
    #17 0x10a8a615d in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator=(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:2542
    #18 0x10a8a7e24 in SkSL::delete_left(SkSL::BasicBlock*, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, bool*, bool*) SkSLCompiler.cpp:719
    #19 0x10a8a44b7 in SkSL::Compiler::simplifyExpression(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:925
    #20 0x10a8b3eff in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1519
    #21 0x10a8c2657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #22 0x10a8bf6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #23 0x10a6fb108 in main SkSLMain.cpp:258
    #24 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x10c42b84d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x10ab0ae64 in std::__1::__libcpp_allocate(unsigned long, unsigned long) new:253
    #2 0x10ab694b1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::allocate(unsigned long, void const*) memory:1869
    #3 0x10ab68fa8 in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::allocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, unsigned long) memory:1586
    #4 0x10ab68c0c in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:318
    #5 0x10ab67d2b in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:317
    #6 0x10aae9341 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::reserve(unsigned long) vector:1586
    #7 0x10ab0eaf4 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:55
    #8 0x10aa7b4e7 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:54
    #9 0x10aacb9eb in std::__1::__unique_if<SkSL::BinaryExpression>::__unique_single std::__1::make_unique<SkSL::BinaryExpression, int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*&>(int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Type const*&) memory:3033
    #10 0x10aaa07b0 in SkSL::IRGenerator::convertBinaryExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1962
    #11 0x10aa6cc39 in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1313
    #12 0x10aa53e58 in SkSL::IRGenerator::convertExpressionStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:697
    #13 0x10aa4757b in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:243
    #14 0x10aa580e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #15 0x10aa502a0 in SkSL::IRGenerator::convertSwitch(SkSL::ASTNode const&) SkSLIRGenerator.cpp:682
    #16 0x10aa47377 in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:232
    #17 0x10aa580e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #18 0x10aa48a03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #19 0x10aa8b8cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #20 0x10aaf0663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #21 0x10a8bee67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #22 0x10a6fb108 in main SkSLMain.cpp:258
    #23 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c0400003330: fa fa fd fa fa fa 00 fa fa fa fd fa fa fa fc fa
  0x1c0400003340: fa fa fd fa fa fa fc fa fa fa fd fa fa fa fc fa
  0x1c0400003350: fa fa fd fd fa fa 00 00 fa fa 00 fa fa fa fd fa
  0x1c0400003360: fa fa 00 00 fa fa 00 fa fa fa 00 fa fa fa fd fa
  0x1c0400003370: fa fa fd fd fa fa fd fa fa fa fd fd fa fa fd fa
=>0x1c0400003380: fa fa fd fd fa fa fd fa fa fa fd fa fa fa[fd]fd
  0x1c0400003390: fa fa fd fa fa fa fd fd fa fa fd fd fa fa fd fd
  0x1c04000033a0: fa fa fd fa fa fa fd fd fa fa 00 fa fa fa fd fa
  0x1c04000033b0: fa fa fd fd fa fa fd fd fa fa fd fd fa fa fd fd
  0x1c04000033c0: fa fa fd fd fa fa fd fd fa fa fd fd fa fa fd fd
  0x1c04000033d0: fa fa fd fd fa fa fd fd fa fa fd fd fa fa fd fa
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
==77539==ABORTING
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x608000010420): half x
Node 1 (0x608000010438): half x;
Node 2 (0x608000010450): 1
Node 3 (0x608000010468): @switch (1) {
case 0:
(x = 0.0);
break;
default:
(x = 1.0);
discard;
}
Exits: [2, 4]

Block 1
-------
Before: [half x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half4(x)
Node 3 (0x610000000f88): (sk_FragColor = half4(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(x));
Exits: [6]

Block 2
-------
Before: [half x = <undefined>]
Entrances: [0]
Node 0 (0x610000000d40): 0
Node 1 (0x610000000d58): x
Node 2 (0x610000000d70): 0.0
Node 3 (0x610000000d88): (x = 0.0)
Node 4 (0x610000000da0): (x = 0.0);
Node 5 (0x610000000db8): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [half x = <undefined>]
Entrances: [0]
Node 0 (0x610000000e40): x
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): (x = 1.0)
Node 3 (0x610000000e88): (x = 1.0);
Node 4 (0x610000000ea0): discard;
Exits: []

Block 5
-------
Before: []
Entrances: []
Exits: []

Block 6
-------
Before: [half x = 0.0]
Entrances: [1]
Exits: []

about to simplify ST half x 
simplifying vardecl
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x608000010420): half x
Node 1 (0x608000010438): half x;
Node 2 (0x608000010450): 1
Node 3 (0x608000010468): @switch (1) {
case 0:
(x = 0.0);
break;
default:
(x = 1.0);
discard;
}
Exits: [2, 4]

Block 1
-------
Before: [half x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half4(x)
Node 3 (0x610000000f88): (sk_FragColor = half4(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(x));
Exits: [6]

Block 2
-------
Before: [half x = <undefined>]
Entrances: [0]
Node 0 (0x610000000d40): 0
Node 1 (0x610000000d58): x
Node 2 (0x610000000d70): 0.0
Node 3 (0x610000000d88): (x = 0.0)
Node 4 (0x610000000da0): (x = 0.0);
Node 5 (0x610000000db8): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [half x = <undefined>]
Entrances: [0]
Node 0 (0x610000000e40): x
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): (x = 1.0)
Node 3 (0x610000000e88): (x = 1.0);
Node 4 (0x610000000ea0): discard;
Exits: []

Block 5
-------
Before: []
Entrances: []
Exits: []

Block 6
-------
Before: [half x = 0.0]
Entrances: [1]
Exits: []

about to simplify ST half x; 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x608000010420): half x
Node 1 (0x608000010438): half x;
Node 2 (0x608000010450): 1
Node 3 (0x608000010468): @switch (1) {
case 0:
(x = 0.0);
break;
default:
(x = 1.0);
discard;
}
Exits: [2, 4]

Block 1
-------
Before: [half x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half4(x)
Node 3 (0x610000000f88): (sk_FragColor = half4(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(x));
Exits: [6]

Block 2
-------
Before: [half x = <undefined>]
Entrances: [0]
Node 0 (0x610000000d40): 0
Node 1 (0x610000000d58): x
Node 2 (0x610000000d70): 0.0
Node 3 (0x610000000d88): (x = 0.0)
Node 4 (0x610000000da0): (x = 0.0);
Node 5 (0x610000000db8): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [half x = <undefined>]
Entrances: [0]
Node 0 (0x610000000e40): x
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): (x = 1.0)
Node 3 (0x610000000e88): (x = 1.0);
Node 4 (0x610000000ea0): discard;
Exits: []

Block 5
-------
Before: []
Entrances: []
Exits: []

Block 6
-------
Before: [half x = 0.0]
Entrances: [1]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x608000010420): half x
Node 1 (0x608000010438): half x;
Node 2 (0x608000010450): 1
Node 3 (0x608000010468): @switch (1) {
case 0:
(x = 0.0);
break;
default:
(x = 1.0);
discard;
}
Exits: [2, 4]

Block 1
-------
Before: [half x = 0.0]
Entrances: [2]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): x
Node 2 (0x610000000f70): half4(x)
Node 3 (0x610000000f88): (sk_FragColor = half4(x))
Node 4 (0x610000000fa0): (sk_FragColor = half4(x));
Exits: [6]

Block 2
-------
Before: [half x = <undefined>]
Entrances: [0]
Node 0 (0x610000000d40): 0
Node 1 (0x610000000d58): x
Node 2 (0x610000000d70): 0.0
Node 3 (0x610000000d88): (x = 0.0)
Node 4 (0x610000000da0): (x = 0.0);
Node 5 (0x610000000db8): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [half x = <undefined>]
Entrances: [0]
Node 0 (0x610000000e40): x
Node 1 (0x610000000e58): 1.0
Node 2 (0x610000000e70): (x = 1.0)
Node 3 (0x610000000e88): (x = 1.0);
Node 4 (0x610000000ea0): discard;
Exits: []

Block 5
-------
Before: []
Entrances: []
Exits: []

Block 6
-------
Before: [half x = 0.0]
Entrances: [1]
Exits: []

about to simplify ST @switch (1) {
case 0:
(x = 0.0);
break;
default:
(x = 1.0);
discard;
} 
simplifying switch
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x610000001040): half x
Node 1 (0x610000001058): half x;
Node 2 (0x610000001070): x
Node 3 (0x610000001088): 1.0
Node 4 (0x6100000010a0): (x = 1.0)
Node 5 (0x6100000010b8): (x = 1.0);
Node 6 (0x6100000010d0): discard;
Exits: []

Block 1
-------
Before: []
Entrances: []
Node 0 (0x610000001140): sk_FragColor
Node 1 (0x610000001158): x
Node 2 (0x610000001170): half4(x)
Node 3 (0x610000001188): (sk_FragColor = half4(x))
Node 4 (0x6100000011a0): (sk_FragColor = half4(x));
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify ST half x 
simplifying vardecl
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x610000001040): half x
Node 1 (0x610000001058): half x;
Node 2 (0x610000001070): x
Node 3 (0x610000001088): 1.0
Node 4 (0x6100000010a0): (x = 1.0)
Node 5 (0x6100000010b8): (x = 1.0);
Node 6 (0x6100000010d0): discard;
Exits: []

Block 1
-------
Before: []
Entrances: []
Node 0 (0x610000001140): sk_FragColor
Node 1 (0x610000001158): x
Node 2 (0x610000001170): half4(x)
Node 3 (0x610000001188): (sk_FragColor = half4(x))
Node 4 (0x6100000011a0): (sk_FragColor = half4(x));
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify ST half x; 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x610000001040): half x
Node 1 (0x610000001058): half x;
Node 2 (0x610000001070): x
Node 3 (0x610000001088): 1.0
Node 4 (0x6100000010a0): (x = 1.0)
Node 5 (0x6100000010b8): (x = 1.0);
Node 6 (0x6100000010d0): discard;
Exits: []

Block 1
-------
Before: []
Entrances: []
Node 0 (0x610000001140): sk_FragColor
Node 1 (0x610000001158): x
Node 2 (0x610000001170): half4(x)
Node 3 (0x610000001188): (sk_FragColor = half4(x))
Node 4 (0x6100000011a0): (sk_FragColor = half4(x));
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x610000001040): half x
Node 1 (0x610000001058): half x;
Node 2 (0x610000001070): x
Node 3 (0x610000001088): 1.0
Node 4 (0x6100000010a0): (x = 1.0)
Node 5 (0x6100000010b8): (x = 1.0);
Node 6 (0x6100000010d0): discard;
Exits: []

Block 1
-------
Before: []
Entrances: []
Node 0 (0x610000001140): sk_FragColor
Node 1 (0x610000001158): x
Node 2 (0x610000001170): half4(x)
Node 3 (0x610000001188): (sk_FragColor = half4(x))
Node 4 (0x6100000011a0): (sk_FragColor = half4(x));
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x610000001040): half x
Node 1 (0x610000001058): half x;
Node 2 (0x610000001070): x
Node 3 (0x610000001088): 1.0
Node 4 (0x6100000010a0): (x = 1.0)
Node 5 (0x6100000010b8): (x = 1.0);
Node 6 (0x6100000010d0): discard;
Exits: []

Block 1
-------
Before: []
Entrances: []
Node 0 (0x610000001140): sk_FragColor
Node 1 (0x610000001158): x
Node 2 (0x610000001170): half4(x)
Node 3 (0x610000001188): (sk_FragColor = half4(x))
Node 4 (0x6100000011a0): (sk_FragColor = half4(x));
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify EX (x = 1.0) 
optimizing binary 
deadass? update=0 rescan=0 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x610000001040): half x
Node 1 (0x610000001058): half x;
Node 2 (0x610000001070): x
Node 3 (0x610000001088): 1.0
Node 4 (0x6100000010a0): (x = 1.0)
Node 5 (0x6100000010b8): (x = 1.0);
Node 6 (0x6100000010d0): discard;
Exits: []

Block 1
-------
Before: []
Entrances: []
Node 0 (0x610000001140): sk_FragColor
Node 1 (0x610000001158): x
Node 2 (0x610000001170): half4(x)
Node 3 (0x610000001188): (sk_FragColor = half4(x))
Node 4 (0x6100000011a0): (sk_FragColor = half4(x));
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify ST (x = 1.0); 
simplifying expr
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x610000001040): half x
Node 1 (0x610000001058): half x;
Node 2 (0x610000001070): x
Node 3 (0x610000001088): 1.0
Node 4 (0x6100000010a0): (x = 1.0)
Node 5 (0x6100000010b8): (x = 1.0);
Node 6 (0x6100000010d0): discard;
Exits: []

Block 1
-------
Before: []
Entrances: []
Node 0 (0x610000001140): sk_FragColor
Node 1 (0x610000001158): x
Node 2 (0x610000001170): half4(x)
Node 3 (0x610000001188): (sk_FragColor = half4(x))
Node 4 (0x6100000011a0): (sk_FragColor = half4(x));
Exits: []

Block 2
-------
Before: []
Entrances: []
Exits: []

about to simplify ST discard; 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x610000001240): half x
Node 1 (0x610000001258): half x;
Node 2 (0x610000001270): x
Node 3 (0x610000001288): 1.0
Node 4 (0x6100000012a0): (x = 1.0)
Node 5 (0x6100000012b8): (x = 1.0);
Node 6 (0x6100000012d0): discard;
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

about to simplify ST half x 
simplifying vardecl
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x610000001240): ;
Node 1 (0x610000001258): half ;
Node 2 (0x610000001270): x
Node 3 (0x610000001288): 1.0
Node 4 (0x6100000012a0): (x = 1.0)
Node 5 (0x6100000012b8): (x = 1.0);
Node 6 (0x6100000012d0): discard;
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

about to simplify ST half ; 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x610000001240): ;
Node 1 (0x610000001258): half ;
Node 2 (0x610000001270): x
Node 3 (0x610000001288): 1.0
Node 4 (0x6100000012a0): (x = 1.0)
Node 5 (0x6100000012b8): (x = 1.0);
Node 6 (0x6100000012d0): discard;
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

about to simplify EX x 
optimizing varref 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x610000001240): ;
Node 1 (0x610000001258): half ;
Node 2 (0x610000001270): x
Node 3 (0x610000001288): 1.0
Node 4 (0x6100000012a0): (x = 1.0)
Node 5 (0x6100000012b8): (x = 1.0);
Node 6 (0x6100000012d0): discard;
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

about to simplify EX 1.0 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x610000001240): ;
Node 1 (0x610000001258): half ;
Node 2 (0x610000001270): x
Node 3 (0x610000001288): 1.0
Node 4 (0x6100000012a0): (x = 1.0)
Node 5 (0x6100000012b8): (x = 1.0);
Node 6 (0x6100000012d0): discard;
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

about to simplify EX (x = 1.0) 
optimizing binary 
deadass? update=1 rescan=0 
deadass! update=1 rescan=0 
delete_left update=1 rescan=0 
Block 0
-------
Before: [half x = <undefined>]
Entrances: []
Node 0 (0x610000001240): ;
Node 1 (0x610000001258): half ;
