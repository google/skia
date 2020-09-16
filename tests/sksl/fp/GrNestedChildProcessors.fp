uniform half4 color;
in fragmentProcessor child1;
in fragmentProcessor child2;

void main() {
    sk_OutColor = sample(child2, color * sample(child1, color * half4(0.5)));
}
