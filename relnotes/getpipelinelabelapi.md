The `PrecompileContext` now has a `getPipelineLabel` method that will return a human-readable version of a serialized Pipeline key. Relatedly, `SkRuntimeEffect::Options` now has an `fName` member variable
which allows clients to provide names for their created runtime effects. The latter API addition is particularly appropriate for user-defined known runtime effects.
