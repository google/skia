### Compilation failed:

=================================================================
==77798==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013cd0 at pc 0x00010ea2ec05 bp 0x7ffee12e6f00 sp 0x7ffee12e6ef8
READ of size 8 at 0x60d000013cd0 thread T0
    #0 0x10ea2ec04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x10ea2d9a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x10ea2cfcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x10ead2d91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x10eae1657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x10eade6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x10e919886 in main SkSLMain.cpp:242
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013cd0 is located 128 bytes inside of 136-byte region [0x60d000013c50,0x60d000013cd8)
freed by thread T0 here:
    #0 0x110647c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10edb68dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x10ebf3e01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x10eace08d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x10eb1997d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x10eacda86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x10eac94a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x10ead3012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x10eae1657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x10eade6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x10e919886 in main SkSLMain.cpp:242
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x11064784d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x10ec8efdd in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable*, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable*&&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x10ec819a3 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:472
    #3 0x10ec68116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x10ec6617e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x10ec770e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x10ec67a03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #7 0x10ecaa8cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #8 0x10ed0f663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #9 0x10eadde67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #10 0x10e919886 in main SkSLMain.cpp:242
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
==77798==ABORTING
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1
Node 1 (0x610000000d58): int x = 1
Node 2 (0x610000000d70): int x = 1;
Node 3 (0x610000000d88): x
Node 4 (0x610000000da0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = 1]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = 1]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = 1]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = 1]
Entrances: [1]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1
Node 1 (0x610000000d58): int x = 1
Node 2 (0x610000000d70): int x = 1;
Node 3 (0x610000000d88): x
Node 4 (0x610000000da0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = 1]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = 1]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = 1]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = 1]
Entrances: [1]
Exits: []

about to simplify ST int x = 1 
simplifying vardecl
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1
Node 1 (0x610000000d58): int x = 1
Node 2 (0x610000000d70): int x = 1;
Node 3 (0x610000000d88): x
Node 4 (0x610000000da0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = 1]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = 1]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = 1]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = 1]
Entrances: [1]
Exits: []

about to simplify ST int x = 1; 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1
Node 1 (0x610000000d58): int x = 1
Node 2 (0x610000000d70): int x = 1;
Node 3 (0x610000000d88): x
Node 4 (0x610000000da0): @switch (x) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = 1]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = 1]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = 1]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = 1]
Entrances: [1]
Exits: []

about to simplify EX x 
optimized to 1 
coerced to 1 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000000d40): 1
Node 1 (0x610000000d58): int x = 1
Node 2 (0x610000000d70): int x = 1;
Node 3 (0x610000000d88): 1
Node 4 (0x610000000da0): @switch (1) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
}
Exits: [2, 4]

Block 1
-------
Before: [int x = 1]
Entrances: [2, 4]
Exits: [5]

Block 2
-------
Before: [int x = 1]
Entrances: [0]
Node 0 (0x610000000e40): 1
Node 1 (0x610000000e58): sk_FragColor
Node 2 (0x610000000e70): 1.0
Node 3 (0x610000000e88): half4(1.0)
Node 4 (0x610000000ea0): (sk_FragColor = half4(1.0))
Node 5 (0x610000000eb8): (sk_FragColor = half4(1.0));
Node 6 (0x610000000ed0): break;
Exits: [1]

Block 3
-------
Before: []
Entrances: []
Exits: []

Block 4
-------
Before: [int x = 1]
Entrances: [0]
Node 0 (0x610000000f40): sk_FragColor
Node 1 (0x610000000f58): 0.0
Node 2 (0x610000000f70): half4(0.0)
Node 3 (0x610000000f88): (sk_FragColor = half4(0.0))
Node 4 (0x610000000fa0): (sk_FragColor = half4(0.0));
Exits: [1]

Block 5
-------
Before: [int x = 1]
Entrances: [1]
Exits: []

about to simplify ST @switch (1) {
case 1:
(sk_FragColor = half4(1.0));
break;
default:
(sk_FragColor = half4(0.0));
} 
simplifying switch
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): int x = 1
Node 2 (0x610000001070): int x = 1;
Node 3 (0x610000001088): sk_FragColor
Node 4 (0x6100000010a0): 1.0
Node 5 (0x6100000010b8): half4(1.0)
Node 6 (0x6100000010d0): (sk_FragColor = half4(1.0))
Node 7 (0x6100000010e8): (sk_FragColor = half4(1.0));
Exits: [1]

Block 1
-------
Before: [int x = 1]
Entrances: [0]
Exits: []

about to simplify EX 1 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): 1
Node 1 (0x610000001058): int x = 1
Node 2 (0x610000001070): int x = 1;
Node 3 (0x610000001088): sk_FragColor
Node 4 (0x6100000010a0): 1.0
Node 5 (0x6100000010b8): half4(1.0)
Node 6 (0x6100000010d0): (sk_FragColor = half4(1.0))
Node 7 (0x6100000010e8): (sk_FragColor = half4(1.0));
Exits: [1]

Block 1
-------
Before: [int x = 1]
Entrances: [0]
Exits: []

about to simplify ST int x = 1 
simplifying vardecl
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x610000001040): ;
Node 1 (0x610000001058): int ;
Node 2 (0x610000001070): sk_FragColor
Node 3 (0x610000001088): 1.0
Node 4 (0x6100000010a0): half4(1.0)
Node 5 (0x6100000010b8): (sk_FragColor = half4(1.0))
Node 6 (0x6100000010d0): (sk_FragColor = half4(1.0));
Exits: [1]

Block 1
-------
Before: [
