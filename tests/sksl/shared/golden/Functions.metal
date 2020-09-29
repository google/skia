### Compilation failed:

=================================================================
==94925==ERROR: AddressSanitizer: heap-use-after-free on address 0x61100002b2b1 at pc 0x0001103c8ada bp 0x7ffee15536c0 sp 0x7ffee1552e80
READ of size 8 at 0x61100002b2b1 thread T0
    #0 0x1103c8ad9 in wrap_memmove+0x169 (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x1dad9)
    #1 0x7fff6f361289 in std::__1::basic_string<char, std::__1::char_traits<char>, std::__1::allocator<char> >::append(char const*, unsigned long)+0x85 (libc++.1.dylib:x86_64+0x38289)
    #2 0x10ee60553 in SkSL::String::operator+(SkSL::StringFragment) const SkSLString.cpp:90
    #3 0x10e965d5d in SkSL::Variable::description() const SkSLVariable.h:53
    #4 0x10e7bde10 in SkSL::BasicBlock::dump() const SkSLCFGGenerator.cpp:66
    #5 0x10e7bd49a in SkSL::CFG::dump() const SkSLCFGGenerator.cpp:58
    #6 0x10e86a021 in SkSL::Compiler::scanCFG(SkSL::FunctionDefinition&) SkSLCompiler.cpp:1489
    #7 0x10e8788e7 in SkSL::Compiler::optimize(SkSL::Program&) SkSLCompiler.cpp:1688
    #8 0x10e875976 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1671
    #9 0x10e6aa4f8 in main SkSLMain.cpp:258
    #10 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

0x61100002b2b1 is located 113 bytes inside of 240-byte region [0x61100002b240,0x61100002b330)
freed by thread T0 here:
    #0 0x110400c6d in wrap__ZdlPv+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x55c6d)
    #1 0x10e944b3d in SkSL::Type::~Type() SkSLType.h:56
    #2 0x10e8fbf41 in std::__1::default_delete<SkSL::Symbol const>::operator()(SkSL::Symbol const*) const memory:2368
    #3 0x10e8fbbcd in std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >::reset(SkSL::Symbol const*) memory:2623
    #4 0x10e8fb9a5 in std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >::~unique_ptr() memory:2577
    #5 0x10e8fb941 in std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >::~unique_ptr() memory:2577
    #6 0x10e8fb8e7 in std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > >::destroy(std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >*) memory:1936
    #7 0x10e8fb864 in void std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::__destroy<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > >(std::__1::integral_constant<bool, true>, std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > >&, std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >*) memory:1798
    #8 0x10e8fb7cc in void std::__1::allocator_traits<std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::destroy<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > >(std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > >&, std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >*) memory:1635
    #9 0x10e8fb65f in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::__destruct_at_end(std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >*) vector:426
    #10 0x10e8fb430 in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::clear() vector:369
    #11 0x10e8faca8 in std::__1::__vector_base<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::~__vector_base() vector:463
    #12 0x10e8faa2d in std::__1::vector<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::~vector() vector:555
    #13 0x10e8f67d1 in std::__1::vector<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> >, std::__1::allocator<std::__1::unique_ptr<SkSL::Symbol const, std::__1::default_delete<SkSL::Symbol const> > > >::~vector() vector:550
    #14 0x10e8f6650 in SkSL::SymbolTable::~SymbolTable() SkSLSymbolTable.h:25
    #15 0x10e8f6311 in SkSL::SymbolTable::~SymbolTable() SkSLSymbolTable.h:25
    #16 0x10eb3e35f in std::__1::default_delete<SkSL::SymbolTable>::operator()(SkSL::SymbolTable*) const memory:2368
    #17 0x10eb3d2c8 in std::__1::__shared_ptr_pointer<SkSL::SymbolTable*, std::__1::default_delete<SkSL::SymbolTable>, std::__1::allocator<SkSL::SymbolTable> >::__on_zero_shared() memory:3541
    #18 0x10e6b7d36 in std::__1::__shared_count::__release_shared() memory:3445
    #19 0x10e6b7a97 in std::__1::__shared_weak_count::__release_shared() memory:3487
    #20 0x10e6e0381 in std::__1::shared_ptr<SkSL::SymbolTable>::~shared_ptr() memory:4212
    #21 0x10e6b0dc1 in std::__1::shared_ptr<SkSL::SymbolTable>::~shared_ptr() memory:4210
    #22 0x10e8dcec1 in SkSL::IRNode::BlockData::~BlockData() SkSLIRNode.h:67
    #23 0x10e8db3b1 in SkSL::IRNode::BlockData::~BlockData() SkSLIRNode.h:67
    #24 0x10ee83b8a in SkSL::IRNode::NodeData::cleanup() SkSLIRNode.h:172
    #25 0x10ee83c11 in SkSL::IRNode::NodeData::~NodeData() SkSLIRNode.h:165
    #26 0x10ee80ed1 in SkSL::IRNode::NodeData::~NodeData() SkSLIRNode.h:164
    #27 0x10ee80db2 in SkSL::IRNode::~IRNode() SkSLIRNode.cpp:51
    #28 0x10e8ceb54 in SkSL::Statement::~Statement() SkSLStatement.h:19
    #29 0x10e8dcf24 in SkSL::Block::~Block() SkSLBlock.h:19

previously allocated by thread T0 here:
    #0 0x11040084d in wrap__Znwm+0x7d (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x5584d)
    #1 0x10ea412b0 in std::__1::__unique_if<SkSL::Type>::__unique_single std::__1::make_unique<SkSL::Type, SkSL::String&, SkSL::Type::TypeKind, SkSL::Type const&, int>(SkSL::String&, SkSL::Type::TypeKind&&, SkSL::Type const&, int&&) memory:3033
    #2 0x10ea33a2e in SkSL::IRGenerator::convertVarDeclarations(SkSL::ASTNode const&, SkSL::Variable::Storage) SkSLIRGenerator.cpp:435
    #3 0x10ea1c486 in SkSL::IRGenerator::convertVarDeclarationStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:296
    #4 0x10ea1a4ee in SkSL::IRGenerator::convertSingleStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:222
    #5 0x10ea2b453 in SkSL::IRGenerator::convertStatement(SkSL::ASTNode const&) SkSLIRGenerator.cpp:264
    #6 0x10ea1bd73 in SkSL::IRGenerator::convertBlock(SkSL::ASTNode const&) SkSLIRGenerator.cpp:285
    #7 0x10ea5e9bd in SkSL::IRGenerator::convertFunction(SkSL::ASTNode const&) SkSLIRGenerator.cpp:1084
    #8 0x10eac3413 in SkSL::IRGenerator::convertProgram(SkSL::Program::Kind, char const*, unsigned long, std::__1::vector<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ProgramElement, std::__1::default_delete<SkSL::ProgramElement> > > >*) SkSLIRGenerator.cpp:2852
    #9 0x10e8750f7 in SkSL::Compiler::convertProgram(SkSL::Program::Kind, SkSL::String, SkSL::Program::Settings const&, std::__1::vector<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> >, std::__1::allocator<std::__1::unique_ptr<SkSL::ExternalValue, std::__1::default_delete<SkSL::ExternalValue> > > > const*) SkSLCompiler.cpp:1658
    #10 0x10e6aa4f8 in main SkSLMain.cpp:258
    #11 0x7fff7205ecc8 in start+0x0 (libdyld.dylib:x86_64+0x1acc8)

SUMMARY: AddressSanitizer: heap-use-after-free (libclang_rt.asan_osx_dynamic.dylib:x86_64h+0x1dad9) in wrap_memmove+0x169
Shadow bytes around the buggy address:
  0x1c2200005600: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c2200005610: 00 00 00 00 00 fa fa fa fa fa fa fa fa fa fa fa
  0x1c2200005620: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
  0x1c2200005630: 00 00 00 00 00 00 00 00 00 00 00 00 00 fa fa fa
  0x1c2200005640: fa fa fa fa fa fa fa fa fd fd fd fd fd fd fd fd
=>0x1c2200005650: fd fd fd fd fd fd[fd]fd fd fd fd fd fd fd fd fd
  0x1c2200005660: fd fd fd fd fd fd fa fa fa fa fa fa fa fa fa fa
  0x1c2200005670: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
  0x1c2200005680: fd fd fd fd fd fd fd fd fd fd fd fd fd fa fa fa
  0x1c2200005690: fa fa fa fa fa fa fa fa fd fd fd fd fd fd fd fd
  0x1c22000056a0: fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd fd
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
==94925==ABORTING
