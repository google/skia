in fragmentProcessor child;
bool hasCap = sk_Caps.externalTextureSupport;

void main() {
    if (hasCap) {
        sk_OutColor = sample(child);
    } else {
        sk_OutColor = half4(1);
    }
}
