in fragmentProcessor child;

half4 main(float2 coord) {
    return sample(child, float3x3(0.5)) * sample(child, coord / 2);
}
