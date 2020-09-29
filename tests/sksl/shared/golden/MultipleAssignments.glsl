### Compilation failed:

=================================================================
==94954==ERROR: AddressSanitizer: heap-use-after-free on address 0x602000019cd8 at pc 0x0001042fb365 bp 0x7ffeeba17a60 sp 0x7ffeeba17a58
READ of size 8 at 0x602000019cd8 thread T0
    #0 0x1042fb364 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x1042fa03e in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x1042f949a in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x1043a6021 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1489
    #4 0x1043b48e7 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1688
    #5 0x1043b1976 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1671
    #6 0x1041e5c76 in main SkSLMain.cpp:242
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x602000019cd8 is located 8 bytes inside of 16-byte region [0x602000019cd0,0x602000019ce0)
freed by thread T0 here:
    #0 0x105f3bc6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10420338c in std::__1::_DeallocateCaller::__do_call(void*) new:334
    #2 0x104203330 in std::__1::_DeallocateCaller::__do_deallocate_handle_size(void*, unsigned long) new:292
    #3 0x1043cfd9c in std::__1::_DeallocateCaller::__do_deallocate_handle_size_align(void*, unsigned long, unsigned long) new:268
    #4 0x1043cfd2c in std::__1::__libcpp_deallocate(void*, unsigned long, unsigned long) new:340
    #5 0x1043e4211 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::deallocate(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1872
    #6 0x1043e3c6c in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::deallocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, unsigned long) memory:1594
    #7 0x1044012db in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~__vector_base() vector:464
    #8 0x1049c2d9d in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:555
    #9 0x1049bce71 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::~vector() vector:550
    #10 0x1049bcda0 in SkSL::IRNode::~IRNode() SkSLIRNode.cpp:51
    #11 0x1043ee684 in SkSL::Expression::~Expression() SkSLExpression.h:27
    #12 0x104620b54 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #13 0x10461ec61 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #14 0x10461ecc1 in SkSL::BinaryExpression::~BinaryExpression() SkSLBinaryExpression.h:49
    #15 0x1044695c1 in std::__1::default_delete<SkSL::Expression>::operator()(SkSL::Expression*) const memory:2368
    #16 0x104468fad in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::reset(SkSL::Expression*) memory:2623
    #17 0x104394c8d in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator=(std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:2542
    #18 0x104396a54 in SkSL::delete_left(SkSL::BasicBlock*, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, bool*, bool*) SkSLCompiler.cpp:719
    #19 0x104390a05 in SkSL::Compiler::simplifyExpression(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:906
    #20 0x1043a618f in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1492
    #21 0x1043b48e7 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1688
    #22 0x1043b1976 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1671
    #23 0x1041e5c76 in main SkSLMain.cpp:242
    #24 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x105f3b84d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x1043cbd14 in std::__1::__libcpp_allocate(unsigned long, unsigned long) new:253
    #2 0x1043e2cf1 in std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >::allocate(unsigned long, void const*) memory:1869
    #3 0x1043e27e8 in std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::allocate(std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&, unsigned long) memory:1586
    #4 0x1043e244c in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:318
    #5 0x1043e152b in std::__1::__split_buffer<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&>::__split_buffer(unsigned long, unsigned long, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >&) __split_buffer:317
    #6 0x1043ff531 in std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >::reserve(unsigned long) vector:1586
    #7 0x10461d8a4 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:55
    #8 0x10458a647 in SkSL::BinaryExpression::BinaryExpression(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*) SkSLBinaryExpression.h:54
    #9 0x1045daa2b in std::__1::__unique_if<SkSL::BinaryExpression>::__unique_single std::__1::make_unique<SkSL::BinaryExpression, int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, SkSL::Type const*&>(int const&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Token::Kind&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&, SkSL::Type const*&) memory:3033
    #10 0x1045af8a0 in SkSL::IRGenerator::convertBinaryExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1962
    #11 0x10457bfa9 in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1313
    #12 0x1045ad832 in SkSL::IRGenerator::convertBinaryExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1920
    #13 0x10457bfa9 in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1313
    #14 0x1045631c8 in SkSL::IRGenerator::convertExpressionStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:697
    #15 0x1045568eb in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:243
    #16 0x104567453 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #17 0x104557d73 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #18 0x10459a9bd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #19 0x1045ff413 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #20 0x1043b10f7 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1658
    #21 0x1041e5c76 in main SkSLMain.cpp:242
    #22 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c0400003340: fa fa fd fa fa fa 00 fa fa fa fd fa fa fa 00 fa
  0x1c0400003350: fa fa fd fa fa fa 00 fa fa fa fd fd fa fa 00 00
  0x1c0400003360: fa fa 00 fa fa fa fd fa fa fa fd fd fa fa fd fa
  0x1c0400003370: fa fa 00 fa fa fa fd fa fa fa fd fa fa fa fd fd
  0x1c0400003380: fa fa 00 fa fa fa fd fd fa fa fd fa fa fa 00 fa
=>0x1c0400003390: fa fa 00 00 fa fa 00 fa fa fa fd[fd]fa fa 00 00
  0x1c04000033a0: fa fa 00 fa fa fa 00 fa fa fa 00 00 fa fa 00 fa
  0x1c04000033b0: fa fa fd fd fa fa fd fd fa fa fa fa fa fa fa fa
  0x1c04000033c0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c04000033d0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c04000033e0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==94954==ABORTING
