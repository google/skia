in fragmentProcessor fp1, fp2;

half4 main() {
    const float2 coords = float2(0.5);
    const half4 inColor = half4(0.75);

    return sample(fp1) *
           sample(fp2, coords) *
           sample(fp1, inColor) *
           sample(fp2, coords, inColor);
}
