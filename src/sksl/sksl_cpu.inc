STRINGIFY(
    // special-cased within the compiler - append takes various arguments depending on what kind of
    // stage is being appended
    sk_has_side_effects void append();

    float abs(float x);
    float sin(float x);
    float cos(float x);
    float tan(float x);
    float sqrt(float x);
    sk_has_side_effects void print(float x);
)
