### Compilation failed:

=================================================================
==95050==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013f40 at pc 0x0001020e0365 bp 0x7ffeedc32a60 sp 0x7ffeedc32a58
READ of size 8 at 0x60d000013f40 thread T0
    #0 0x1020e0364 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x1020df03e in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x1020de49a in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x10218b021 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1489
    #4 0x1021998e7 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1688
    #5 0x102196976 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1671
    #6 0x101fcb4f8 in main SkSLMain.cpp:258
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013f40 is located 128 bytes inside of 136-byte region [0x60d000013ec0,0x60d000013f48)
freed by thread T0 here:
    #0 0x103d23c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10247a3dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x1022c9171 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x10218631d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x1021eed8d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x102185d16 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x102181768 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1295
    #7 0x10218b2a2 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1495
    #8 0x1021998e7 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1688
    #9 0x102196976 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1671
    #10 0x101fcb4f8 in main SkSLMain.cpp:258
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x103d2384d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x10253588d in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable const*&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable const*&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x10253faec in SkSL::Inliner::inlineCall(SkSL::FunctionCall*, SkSL::SymbolTable*)::$_6::operator()(SkSL::String const&, SkSL::Type const*, SkSL::Modifiers, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*) const SkSLInliner.cpp:635
    #3 0x10253aa55 in SkSL::Inliner::inlineCall(SkSL::FunctionCall*, SkSL::SymbolTable*) SkSLInliner.cpp:678
    #4 0x102372a8f in SkSL::IRGenerator::call(int, SkSL::FunctionDeclaration const&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >) SkSLIRGenerator.cpp:2115
    #5 0x102376663 in SkSL::IRGenerator::call(int, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >) SkSLIRGenerator.cpp:2206
    #6 0x102395561 in SkSL::IRGenerator::convertCallExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:2742
    #7 0x10236121a in SkSL::IRGenerator::convertExpression(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1318
    #8 0x1023559c7 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:452
    #9 0x10233d486 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #10 0x10233b4ee in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #11 0x10234c453 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #12 0x10233cd73 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #13 0x10237f9bd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #14 0x1023e4413 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #15 0x1021960f7 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1658
    #16 0x101fcb4f8 in main SkSLMain.cpp:258
    #17 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c1a00002790: 00 00 00 00 00 00 00 00 00 00 00 fa fa fa fa fa
  0x1c1a000027a0: fa fa fa fa 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1a000027b0: 00 00 00 00 00 fa fa fa fa fa fa fa fa fa fd fd
  0x1c1a000027c0: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fa
  0x1c1a000027d0: fa fa fa fa fa fa fa fa fd fd fd fd fd fd fd fd
=>0x1c1a000027e0: fd fd fd fd fd fd fd fd[fd]fa fa fa fa fa fa fa
  0x1c1a000027f0: fa fa 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002800: 00 00 00 fa fa fa fa fa fa fa fa fa 00 00 00 00
  0x1c1a00002810: 00 00 00 00 00 00 00 00 00 00 00 00 00 fa fa fa
  0x1c1a00002820: fa fa fa fa fa fa 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002830: 00 00 00 00 00 00 00 fa fa fa fa fa fa fa fa fa
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
==95050==ABORTING
