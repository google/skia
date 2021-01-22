in fragmentProcessor child;
bool hasCap = sk_Caps.externalTextureSupport;

half4 main() {
    if (hasCap) {
        return sample(child);
    } else {
        return half4(1);
    }
}
