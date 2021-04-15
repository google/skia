uniform half4    C;
uniform float3x3 M0;
uniform float3x3 M1;

in fragmentProcessor passthrough;
in fragmentProcessor explicit;
in fragmentProcessor matrix_const;
in fragmentProcessor matrix_uniform;
in fragmentProcessor matrix_uniform_multi;

half4 main(float2 coord) {
    return sample(passthrough, C) +
           sample(explicit, coord / 2, C) +
           sample(matrix_const, float3x3(2), C) +
           sample(matrix_uniform, M0, C) +
           sample(matrix_uniform_multi, M0, C) + sample(matrix_uniform_multi, M1, C);
}
