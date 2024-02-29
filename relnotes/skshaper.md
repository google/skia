The following SkShaper functions have been moved or deleted:
  - SkShaper::MakePrimitive() -> SkShapers::Primitive()
  - SkShaper::MakeShaperDrivenWrapper() -> SkShapers::HB::ShaperDrivenWrapper()
  - SkShaper::MakeShapeThenWrap() -> SkShapers::HB::ShapeThenWrap()
  - SkShaper::MakeShapeDontWrapOrReorder() -> SkShapers::HB::ShapeDontWrapOrReorder()
  - SkShaper::MakeCoreText() -> SkShapers::CT::CoreText()
  - SkShaper::Make() -> deleted, use one of the above directly,
  - SkShaper::MakeSkUnicodeBidiRunIterator() -> SkShapers::unicode::BidiRunIterator()
  - SkShaper::MakeBiDiRunIterator() -> deleted, use SkShapers::unicode::BidiRunIterator() or SkShapers::TrivialBiDiRunIterator()
  - SkShaper::MakeIcuBiDiRunIterator() -> deleted, use SkShapers::unicode::BidiRunIterator()
  - SkShaper::MakeSkUnicodeHbScriptRunIterator() -> SkShapers::HB::ScriptRunIterator()
  - SkShaper::MakeHbIcuScriptRunIterator() -> SkShapers::HB::ScriptRunIterator()
  - SkShaper::MakeScriptRunIterator() -> deleted, use SkShapers::HB::ScriptRunIterator() or SkShapers::TrivialScriptRunIterator

Additionally, two `SkShaper::shape` method overloads have been removed - clients now need to
specify all 10 arguments (although it is common to pass in nullptr for features).
