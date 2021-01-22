uniform half4 color;
in fragmentProcessor child1;
in fragmentProcessor child2;

half4 main() {
    return sample(child2, color * sample(child1, color * half4(0.5)));
}
