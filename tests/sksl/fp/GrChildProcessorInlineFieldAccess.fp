in fragmentProcessor child;

void main() {
    if (child.preservesOpaqueInput) {
        sk_OutColor = sample(child);
    } else {
        sk_OutColor = half4(1);
    }
}
