in fragmentProcessor child;
in uniform float3x3 matrix;

half4 main() {
    return sample(child, matrix);
}
