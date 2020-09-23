in fragmentProcessor? child;

void main(float2 coord) {
    sk_OutColor = sample(child, float3x3(0.5));
    sk_OutColor = sample(child, coord / 2);
}
