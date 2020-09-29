### Compilation failed:

=================================================================
==95136==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000019cb8 at pc 0x000107cfd365 bp 0x7ffee8015a60 sp 0x7ffee8015a58
READ of size 8 at 0x602000019cb8 thread T0
    #0 0x107cfd364 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x107cfc03e in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x107cfb49a in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x107da8021 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1489
    #4 0x107db68e7 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1688
    #5 0x107db3976 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1671
    #6 0x107be84f8 in main SkSLMain.cpp:258
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x602000019cb8 is located 8 bytes inside of 16-byte region [0x602000019cb0,0x602000019cc0)
freed by thread T0 here:
    #0 0x109941c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x107c0538c in std::__1::_DeallocateCaller::__do_call(void*) new:334
    #2 0x107c05330 in std::__1::_DeallocateCaller::__do_deallocate_handle_size(void*, unsigned long) new:292
    #3 0x107dd1d9c in std::__1::_DeallocateCaller::__do_deallocate_handle_size_align(void*, unsigned long, unsigned long) new:268
    #4 0x107dd1d2c in std::__1::__libcpp_deallocate(void*, unsigned long, unsigned long) new:340
    #5 0x107de6211 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::deallocate(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1872
    #6 0x107de5c6c in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::deallocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1594
    #7 0x107e032db in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~__vector_base() vector:464
    #8 0x1083c4d9d in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:555
    #9 0x1083bee71 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:550
    #10 0x1083beda0 in SkSL::IRNode::~IRNode() SkSLIRNode.cpp:51
    #11 0x107df0684 in SkSL::Expression::~Expression() SkSLExpression.h:27
    #12 0x108022b54 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #13 0x108020c61 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #14 0x108020cc1 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #15 0x107e6b5c1 in std::__1::default_delete<SkSL::Expression>::operator()(SkSL::Expression*) const memory:2368
    #16 0x107e6afad in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::reset(SkSL::Expression*) memory:2623
    #17 0x107d96c8d in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator=(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:2542
    #18 0x107d98a54 in SkSL::delete_left(SkSL::BasicBlock*, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, bool*, bool*) SkSLCompiler.cpp:719
    #19 0x107d92a05 in SkSL::Compiler::simplifyExpression(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:906
    #20 0x107da818f in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1492
    #21 0x107db68e7 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1688
    #22 0x107db3976 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1671
    #23 0x107be84f8 in main SkSLMain.cpp:258
    #24 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x10994184d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x107dcdd14 in std::__1::__libcpp_allocate(unsigned long, unsigned long) new:253
    #2 0x107de4cf1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::allocate(unsigned long, void const*) memory:1869
    #3 0x107de47e8 in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::allocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, unsigned long) memory:1586
    #4 0x107de444c in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:318
    #5 0x107de352b in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:317
    #6 0x107e01531 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::reserve(unsigned long) vector:1586
    #7 0x10801f8a4 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:55
    #8 0x107f8c647 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:54
    #9 0x107fdca2b in std::__1::__unique_if<SkSL::BinaryExpression>::__unique_single std::__1::make_unique<SkSL::BinaryExpression, int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*&>(int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Type const*&) memory:3033
    #10 0x107fb18a0 in SkSL::IRGenerator::convertBinaryExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1962
    #11 0x107f7dfa9 in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1313
    #12 0x107f651c8 in SkSL::IRGenerator::convertExpressionStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:697
    #13 0x107f588eb in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:243
    #14 0x107f69453 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #15 0x107f61610 in SkSL::IRGenerator::convertSwitch(SkSL::ASTNode const&) SkSLIRGenerator.cpp:682
    #16 0x107f586e7 in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:232
    #17 0x107f69453 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #18 0x107f59d73 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #19 0x107f9c9bd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #20 0x108001413 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #21 0x107db30f7 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1658
    #22 0x107be84f8 in main SkSLMain.cpp:258
    #23 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c0400003340: fa fa fd fa fa fa 00 fa fa fa fd fa fa fa 00 fa
  0x1c0400003350: fa fa fd fa fa fa 00 fa fa fa fd fd fa fa 00 00
  0x1c0400003360: fa fa 00 fa fa fa fd fa fa fa fd fa fa fa 00 00
  0x1c0400003370: fa fa 00 fa fa fa 00 fa fa fa fd fa fa fa fd fd
  0x1c0400003380: fa fa fd fa fa fa fd fd fa fa fd fa fa fa fd fa
=>0x1c0400003390: fa fa fd fa fa fa fd[fd]fa fa fd fa fa fa fd fd
  0x1c04000033a0: fa fa fd fd fa fa 00 fa fa fa 00 fa fa fa 00 00
  0x1c04000033b0: fa fa 00 fa fa fa fd fa fa fa fd fd fa fa fd fd
  0x1c04000033c0: fa fa fd fd fa fa fd fd fa fa fd fd fa fa fd fd
  0x1c04000033d0: fa fa fd fd fa fa fd fd fa fa fd fd fa fa fd fd
  0x1c04000033e0: fa fa fd fd fa fa fd fd fa fa fd fa fa fa 00 00
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
==95136==ABORTING
