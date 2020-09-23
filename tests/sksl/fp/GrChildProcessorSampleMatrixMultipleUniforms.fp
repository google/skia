in fragmentProcessor? child;
in uniform float3x3 matrixA;
in uniform float3x3 matrixB;

void main() {
    sk_OutColor = sample(child, matrixA);
    sk_OutColor += sample(child, matrixB);
}
