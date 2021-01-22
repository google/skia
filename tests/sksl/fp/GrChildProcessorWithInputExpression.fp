uniform half4 color;

in fragmentProcessor child;

void main() {
    sk_OutColor = sample(child, color * half4(0.5));
}
