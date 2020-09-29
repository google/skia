### Compilation failed:

=================================================================
==78026==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013cd0 at pc 0x0001030cac05 bp 0x7ffeecc4af00 sp 0x7ffeecc4aef8
READ of size 8 at 0x60d000013cd0 thread T0
    #0 0x1030cac04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x1030c99a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x1030c8fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x10316ed91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x10317d657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x10317a6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x102fb5886 in main SkSLMain.cpp:242
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013cd0 is located 128 bytes inside of 136-byte region [0x60d000013c50,0x60d000013cd8)
freed by thread T0 here:
    #0 0x104ce7c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x1034528dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x10328fe01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x10316a08d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x1031b597d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x103169a86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x1031654a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x10316f012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x10317d657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x10317a6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x102fb5886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x104ce784d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x10332afdd in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable*, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable*&&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x10331d9a3 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:472
    #3 0x103304116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x10330217e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x1033130e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x103303a03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #7 0x1033468cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #8 0x1033ab663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #9 0x103179e67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #10 0x102fb5886 in main SkSLMain.cpp:242
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
  0x1c1a000027d0: fa fa fa fa fa fa fa fa 00 00 00 00 00 00 00 00
  0x1c1a000027e0: 00 00 00 00 00 00 00 00 00 fa fa fa fa fa fa fa
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
==78026==ABORTING
Block 0
-------
Before: [float e = <undefined>, float d = <undefined>, float c = <undefined>, float b = <undefined>, float a = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float a = 1.0
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float b = 2.0
Node 4 (0x617000002060): 3.0
Node 5 (0x617000002078): float c = 3.0
Node 6 (0x617000002090): float a = 1.0, b = 2.0, c = 3.0;
Node 7 (0x6170000020a8): c
Node 8 (0x6170000020c0): float d = c
Node 9 (0x6170000020d8): float d = c;
Node 10 (0x6170000020f0): d
Node 11 (0x617000002108): float e = d
Node 12 (0x617000002120): float e = d;
Node 13 (0x617000002138): b
Node 14 (0x617000002150): b++
Node 15 (0x617000002168): b++;
Node 16 (0x617000002180): d
Node 17 (0x617000002198): d++
Node 18 (0x6170000021b0): d++;
Node 19 (0x6170000021c8): sk_FragColor
Node 20 (0x6170000021e0): b
Node 21 (0x6170000021f8): half(b)
Node 22 (0x617000002210): b
Node 23 (0x617000002228): half(b)
Node 24 (0x617000002240): d
Node 25 (0x617000002258): half(d)
Node 26 (0x617000002270): d
Node 27 (0x617000002288): half(d)
Node 28 (0x6170000022a0): half4(half(b), half(b), half(d), half(d))
Node 29 (0x6170000022b8): (sk_FragColor = half4(half(b), half(b), half(d), half(d)))
Node 30 (0x6170000022d0): (sk_FragColor = half4(half(b), half(b), half(d), half(d)));
Exits: [1]

Block 1
-------
Before: [float e = d, float d = <defined>, float c = 3.0, float b = <defined>, float a = 1.0]
Entrances: [0]
Exits: []

about to simplify EX 1.0 
Block 0
-------
Before: [float e = <undefined>, float d = <undefined>, float c = <undefined>, float b = <undefined>, float a = <undefined>]
Entrances: []
Node 0 (0x617000002000): 1.0
Node 1 (0x617000002018): float a = 1.0
Node 2 (0x617000002030): 2.0
Node 3 (0x617000002048): float b = 2.0
Node 4 (0x617000002060): 3.0
Node 5 (0x617000002078): float c = 3.0
Node 6 (0x617000002090): float a = 1.0, b = 2.0, c = 3.0;
Node 7 (0x6170000020a8): c
Node 8 (0x6170000020c0): float d = c
Node 9 (0x6170000020d8): float d = c;
Node 10 (0x6170000020f0): d
Node 11 (0x617000002108): float e = d
Node 12 (0x617000002120): float e = d;
Node 13 (0x617000002138): b
Node 14 (0x617000002150): b++
Node 15 (0x617000002168): b++;
Node 16 (0x617000002180): d
Node 17 (0x617000002198): d++
Node 18 (0x6170000021b0): d++;
Node 19 (0x6170000021c8): sk_FragColor
Node 20 (0x6170000021e0): b
Node 21 (0x6170000021f8): half(b)
Node 22 (0x617000002210): b
Node 23 (0x617000002228): half(b)
Node 24 (0x617000002240): d
Node 25 (0x617000002258): half(d)
Node 26 (0x617000002270): d
Node 27 (0x617000002288): half(d)
Node 28 (0x6170000022a0): half4(half(b), half(b), half(d), half(d))
Node 29 (0x6170000022b8): (sk_FragColor = half4(half(b), half(b), half(d), half(d)))
Node 30 (0x6170000022d0): (sk_FragColor = half4(half(b), half(b), half(d), half(d)));
Exits: [1]

Block 1
-------
Before: [float e = d, float d = <defined>, float c = 3.0, float b = <defined>, float a = 1.0]
Entrances: [0]
Exits: []

about to simplify ST float a = 1.0 
simplifying vardecl
Block 0
-------
Before: [float e = <undefined>, float d = <undefined>, float c = <undefined>, float b = <undefined>, float a = <undefined>]
Entrances: []
Node 0 (0x617000002000): ;
Node 1 (0x617000002018): 2.0
Node 2 (0x617000002030): float b = 2.0
Node 3 (0x617000002048): 3.0
Node 4 (0x617000002060): float c = 3.0
Node 5 (0x617000002078): float b = 2.0, c = 3.0;
Node 6 (0x617000002090): c
Node 7 (0x6170000020a8): float d = c
Node 8 (0x6170000020c0): float d = c;
Node 9 (0x6170000020d8): d
Node 10 (0x6170000020f0): float e = d
Node 11 (0x617000002108): float e = d;
Node 12 (0x617000002120): b
Node 13 (0x617000002138): b++
Node 14 (0x617000002150): b++;
Node 15 (0x617000002168): d
Node 16 (0x617000002180): d++
Node 17 (0x617000002198): d++;
Node 18 (0x6170000021b0): sk_FragColor
Node 19 (0x6170000021c8): b
Node 20 (0x6170000021e0): half(b)
Node 21 (0x6170000021f8): b
Node 22 (0x617000002210): half(b)
Node 23 (0x617000002228): d
Node 24 (0x617000002240): half(d)
Node 25 (0x617000002258): d
Node 26 (0x617000002270): half(d)
Node 27 (0x617000002288): half4(half(b), half(b), half(d), half(d))
Node 28 (0x6170000022a0): (sk_FragColor = half4(half(b), half(b), half(d), half(d)))
Node 29 (0x6170000022b8): (sk_FragColor = half4(half(b), half(b), half(d), half(d)));
Exits: [1]

Block 1
-------
Before: [float e = d, float d = <defined>, float c = 3.0, float b = <defined>
