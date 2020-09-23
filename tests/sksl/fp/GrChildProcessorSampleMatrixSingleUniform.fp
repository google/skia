in fragmentProcessor? child;
uniform float3x3 matrix;

void main() {
    sk_OutColor = sample(child, matrix);
}
