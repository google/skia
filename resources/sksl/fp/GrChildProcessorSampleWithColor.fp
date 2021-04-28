uniform half4    C;

in fragmentProcessor passthrough;
in fragmentProcessor explicit;

half4 main(float2 coord) {
    return sample(passthrough, C) +
           sample(explicit, coord / 2, C);
}
