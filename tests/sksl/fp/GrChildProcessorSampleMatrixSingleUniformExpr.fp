in fragmentProcessor child;
uniform float3x3 matrix;

half4 main() {
    return sample(child, 0.5 * matrix);
}
