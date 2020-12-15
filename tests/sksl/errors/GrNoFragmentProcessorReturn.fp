in fragmentProcessor child;

fragmentProcessor get_child() { return child; }

half4 main() {
    return sample(get_child());
}
