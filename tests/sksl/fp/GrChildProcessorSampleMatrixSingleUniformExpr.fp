in fragmentProcessor? child;
uniform float3x3 matrix;

void main() {
    sk_OutColor = sample(child, 0.5 * matrix);
}
