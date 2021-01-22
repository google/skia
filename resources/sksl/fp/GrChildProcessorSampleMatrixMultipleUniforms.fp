in fragmentProcessor child;
in uniform float3x3 matrixA;
in uniform float3x3 matrixB;

half4 main() {
    return sample(child, matrixA) + sample(child, matrixB);
}
