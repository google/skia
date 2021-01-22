in fragmentProcessor child;

half4 main() {
    if (child.preservesOpaqueInput) {
        return sample(child);
    } else {
        return half4(1);
    }
}
