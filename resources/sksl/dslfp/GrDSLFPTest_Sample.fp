in fragmentProcessor fp1, fp2, fp3;

half4 main() {
    const float2 coords = float2(0.5);
    const float3x3 xform = float3x3(2);
    const half4 inColor = half4(0.75);

    return sample(fp1) *
           sample(fp2, coords) *
           sample(fp3, xform) *
           sample(fp1, inColor) *
           sample(fp2, coords, inColor) *
           sample(fp3, xform, inColor);
}
