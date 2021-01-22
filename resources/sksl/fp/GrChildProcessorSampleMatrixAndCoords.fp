uniform half4 color;
in fragmentProcessor child;

half4 main(float2 coord) {
    float3x3 matrix = float3x3(color.a);
    return sample(child, matrix) * sample(child, coord / 2);
}
