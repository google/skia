in fragmentProcessor? child;
half4 main() {
    if (child != null) {
        return sample(child);
    } else {
        return half4(0.5);
    }
}
