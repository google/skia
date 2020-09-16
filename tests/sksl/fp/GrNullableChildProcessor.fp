in fragmentProcessor? child;
void main() {
    if (child != null) {
        sk_OutColor = sample(child);
    } else {
        sk_OutColor = half4(0.5);
    }
}
