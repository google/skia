in fragmentProcessor child;

void main(float2 coord) {
    sk_OutColor = sample(child) + sample(child, coord / 2);
}
