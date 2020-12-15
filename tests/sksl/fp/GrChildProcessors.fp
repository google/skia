in fragmentProcessor child1;
in fragmentProcessor child2;

half4 main() {
    return sample(child1) * sample(child2);
}
