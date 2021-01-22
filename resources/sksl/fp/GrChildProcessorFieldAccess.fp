in fragmentProcessor child;
bool opaque = child.preservesOpaqueInput;

half4 main() {
    if (opaque) {
        return sample(child);
    } else {
        return half4(0.5);
    }
}
