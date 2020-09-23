in fragmentProcessor child;
bool opaque = child.preservesOpaqueInput;

void main() {
    if (opaque) {
        sk_OutColor = sample(child);
    } else {
        sk_OutColor = half4(0.5);
    }
}
