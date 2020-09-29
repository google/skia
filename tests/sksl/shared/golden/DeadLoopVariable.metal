### Compilation failed:

=================================================================
==77520==ERROR: AddressSanitizer: heap-use-after-free on address 0x60d000013da0 at pc 0x0001100dbc05 bp 0x7ffedfc39f00 sp 0x7ffedfc39ef8
READ of size 8 at 0x60d000013da0 thread T0
    #0 0x1100dbc04 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const memory:2592
    #1 0x1100da9a2 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:67
    #2 0x1100d9fcc in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #3 0x11017fd91 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1516
    #4 0x11018e657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #5 0x11018b6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #6 0x10ffc7108 in main SkSLMain.cpp:258
    #7 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x60d000013da0 is located 128 bytes inside of 136-byte region [0x60d000013d20,0x60d000013da8)
freed by thread T0 here:
    #0 0x111cf2c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x1104638dd in SkSL::VarDeclaration::~VarDeclaration() SkSLVarDeclarations.h:23
    #2 0x1102a0e01 in std::__1::default_delete<SkSL::Statement>::operator()(SkSL::Statement*) const memory:2368
    #3 0x11017b08d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::reset(SkSL::Statement*) memory:2623
    #4 0x1101c697d in std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >::operator=(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >&&) memory:2542
    #5 0x11017aa86 in SkSL::BasicBlock::Node::setStatement(std::__1::unique_ptr<SkSL::Statement, std::__1::default_delete<SkSL::Statement> >) SkSLCFGGenerator.h:59
    #6 0x1101764a4 in SkSL::Compiler::simplifyStatement(std::__1::unordered_map<SkSL::Variable const*, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<std::__1::pair<SkSL::Variable const* const, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >*> > >&, SkSL::BasicBlock&, std::__1::__wrap_iter<SkSL::BasicBlock::Node*>*, std::__1::unordered_set<SkSL::Variable const*, std::__1::hash<SkSL::Variable const*>, std::__1::equal_to<SkSL::Variable const*>, std::__1::allocator<SkSL::Variable const*> >*, bool*, bool*) SkSLCompiler.cpp:1320
    #7 0x110180012 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1522
    #8 0x11018e657 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1715
    #9 0x11018b6e6 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1698
    #10 0x10ffc7108 in main SkSLMain.cpp:258
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

previously allocated by thread T0 here:
    #0 0x111cf284d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x11033bfdd in std::__1::__unique_if<SkSL::VarDeclaration>::__unique_single std::__1::make_unique<SkSL::VarDeclaration, SkSL::Variable*, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > >(SkSL::Variable*&&, std::__1::vector<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> > > >&&, std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >&&) memory:3033
    #2 0x11032e9a3 in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:472
    #3 0x110315116 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x11031317e in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x1103240e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x11031725a in SkSL::IRGenerator::convertFor(SkSL::ASTNode const&) SkSLIRGenerator.cpp:558
    #7 0x110313248 in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:226
    #8 0x1103240e3 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #9 0x110314a03 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #10 0x1103578cd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #11 0x1103bc663 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #12 0x11018ae67 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1685
    #13 0x10ffc7108 in main SkSLMain.cpp:258
    #14 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free memory:2592 in std::__1::unique_ptr<SkSL::Expression, std::__1::default_delete<SkSL::Expression> >::operator->() const
Shadow bytes around the buggy address:
  0x1c1a00002760: 00 00 00 00 00 00 00 fa fa fa fa fa fa fa fa fa
  0x1c1a00002770: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c1a00002780: 00 fa fa fa fa fa fa fa fa fa 00 00 00 00 00 00
  0x1c1a00002790: 00 00 00 00 00 00 00 00 00 00 00 fa fa fa fa fa
  0x1c1a000027a0: fa fa fa fa fd fd fd fd fd fd fd fd fd fd fd fd
=>0x1c1a000027b0: fd fd fd fd[fd]fa fa fa fa fa fa fa fa fa 00 00
  0x1c1a000027c0: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 fa
  0x1c1a000027d0: fa fa fa fa fa fa fa fa 00 00 00 00 00 00 00 00
  0x1c1a000027e0: 00 00 00 00 00 00 00 00 00 00 fa fa fa fa fa fa
  0x1c1a000027f0: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
  0x1c1a00002800: fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa fa
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
==77520==ABORTING
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x608000010220): 0
Node 1 (0x608000010238): int x = 0
Node 2 (0x608000010250): int x = 0;
Exits: [1]

Block 1
-------
Before: [int x = 0]
Entrances: [0]
Node 0 (0x6080000102a0): x
Node 1 (0x6080000102b8): 4
Node 2 (0x6080000102d0): (x < 4)
Exits: [4]

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [int x = 0]
Entrances: [4]
Exits: [6]

Block 4
-------
Before: [int x = 0]
Entrances: [1]
Node 0 (0x60300000a390): break;
Exits: [3]

Block 5
-------
Before: []
Entrances: []
Exits: []

Block 6
-------
Before: [int x = 0]
Entrances: [3]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x608000010220): 0
Node 1 (0x608000010238): int x = 0
Node 2 (0x608000010250): int x = 0;
Exits: [1]

Block 1
-------
Before: [int x = 0]
Entrances: [0]
Node 0 (0x6080000102a0): x
Node 1 (0x6080000102b8): 4
Node 2 (0x6080000102d0): (x < 4)
Exits: [4]

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [int x = 0]
Entrances: [4]
Exits: [6]

Block 4
-------
Before: [int x = 0]
Entrances: [1]
Node 0 (0x60300000a390): break;
Exits: [3]

Block 5
-------
Before: []
Entrances: []
Exits: []

Block 6
-------
Before: [int x = 0]
Entrances: [3]
Exits: []

about to simplify ST int x = 0 
simplifying vardecl
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x608000010220): 0
Node 1 (0x608000010238): int x = 0
Node 2 (0x608000010250): int x = 0;
Exits: [1]

Block 1
-------
Before: [int x = 0]
Entrances: [0]
Node 0 (0x6080000102a0): x
Node 1 (0x6080000102b8): 4
Node 2 (0x6080000102d0): (x < 4)
Exits: [4]

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [int x = 0]
Entrances: [4]
Exits: [6]

Block 4
-------
Before: [int x = 0]
Entrances: [1]
Node 0 (0x60300000a390): break;
Exits: [3]

Block 5
-------
Before: []
Entrances: []
Exits: []

Block 6
-------
Before: [int x = 0]
Entrances: [3]
Exits: []

about to simplify ST int x = 0; 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x608000010220): 0
Node 1 (0x608000010238): int x = 0
Node 2 (0x608000010250): int x = 0;
Exits: [1]

Block 1
-------
Before: [int x = 0]
Entrances: [0]
Node 0 (0x6080000102a0): x
Node 1 (0x6080000102b8): 4
Node 2 (0x6080000102d0): (x < 4)
Exits: [4]

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [int x = 0]
Entrances: [4]
Exits: [6]

Block 4
-------
Before: [int x = 0]
Entrances: [1]
Node 0 (0x60300000a390): break;
Exits: [3]

Block 5
-------
Before: []
Entrances: []
Exits: []

Block 6
-------
Before: [int x = 0]
Entrances: [3]
Exits: []

about to simplify EX x 
optimized to 0 
coerced to 0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x608000010220): 0
Node 1 (0x608000010238): int x = 0
Node 2 (0x608000010250): int x = 0;
Exits: [1]

Block 1
-------
Before: [int x = 0]
Entrances: [0]
Node 0 (0x6080000102a0): 0
Node 1 (0x6080000102b8): 4
Node 2 (0x6080000102d0): (0 < 4)
Exits: [4]

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [int x = 0]
Entrances: [4]
Exits: [6]

Block 4
-------
Before: [int x = 0]
Entrances: [1]
Node 0 (0x60300000a390): break;
Exits: [3]

Block 5
-------
Before: []
Entrances: []
Exits: []

Block 6
-------
Before: [int x = 0]
Entrances: [3]
Exits: []

about to simplify EX 4 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x608000010220): 0
Node 1 (0x608000010238): int x = 0
Node 2 (0x608000010250): int x = 0;
Exits: [1]

Block 1
-------
Before: [int x = 0]
Entrances: [0]
Node 0 (0x6080000102a0): 0
Node 1 (0x6080000102b8): 4
Node 2 (0x6080000102d0): (0 < 4)
Exits: [4]

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [int x = 0]
Entrances: [4]
Exits: [6]

Block 4
-------
Before: [int x = 0]
Entrances: [1]
Node 0 (0x60300000a390): break;
Exits: [3]

Block 5
-------
Before: []
Entrances: []
Exits: []

Block 6
-------
Before: [int x = 0]
Entrances: [3]
Exits: []

about to simplify EX (0 < 4) 
optimized to true 
coerced to true 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x608000010420): 0
Node 1 (0x608000010438): int x = 0
Node 2 (0x608000010450): int x = 0;
Exits: [1]

Block 1
-------
Before: [int x = 0]
Entrances: [0]
Node 0 (0x60300000a630): true
Exits: [4]

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [int x = 0]
Entrances: [4]
Exits: [6]

Block 4
-------
Before: [int x = 0]
Entrances: [1]
Node 0 (0x60300000a660): break;
Exits: [3]

Block 5
-------
Before: []
Entrances: []
Exits: []

Block 6
-------
Before: [int x = 0]
Entrances: [3]
Exits: []

about to simplify EX 0 
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x608000010420): 0
Node 1 (0x608000010438): int x = 0
Node 2 (0x608000010450): int x = 0;
Exits: [1]

Block 1
-------
Before: [int x = 0]
Entrances: [0]
Node 0 (0x60300000a630): true
Exits: [4]

Block 2
-------
Before: []
Entrances: []
Exits: []

Block 3
-------
Before: [int x = 0]
Entrances: [4]
Exits: [6]

Block 4
-------
Before: [int x = 0]
Entrances: [1]
Node 0 (0x60300000a660): break;
Exits: [3]

Block 5
-------
Before: []
Entrances: []
Exits: []

Block 6
-------
Before: [int x = 0]
Entrances: [3]
Exits: []

about to simplify ST int x = 0 
simplifying vardecl
Block 0
-------
Before: [int x = <undefined>]
Entrances: []
Node 0 (0x608000010420): ;
Node 1 (0x608000010438): int ;
Exits: [1]

Block 1
-------
Before: [
