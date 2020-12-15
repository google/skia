uniform half4 color;

in fragmentProcessor child;

half4 main() {
    return sample(child, color * half4(0.5));
}
