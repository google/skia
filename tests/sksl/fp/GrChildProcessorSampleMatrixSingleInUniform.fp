in fragmentProcessor? child;
in uniform float3x3 matrix;

void main() {
    sk_OutColor = sample(child, matrix);
}
