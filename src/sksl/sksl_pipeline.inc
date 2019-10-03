STRINGIFY(
    // special-cased within the compiler - append takes various arguments depending on what kind of
    // stage is being appended
    sk_has_side_effects void append();

    sk_has_side_effects void print(float x);

    half4 sample(fragmentProcessor fp);
)
