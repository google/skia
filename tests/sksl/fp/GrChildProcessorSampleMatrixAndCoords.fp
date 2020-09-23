uniform half4 color;
in fragmentProcessor? child;

void main(float2 coord) {
    float3x3 matrix = float3x3(color.a);
    sk_OutColor = sample(child, matrix);
    sk_OutColor = sample(child, coord / 2);
}
