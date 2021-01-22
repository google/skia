in fragmentProcessor child;

fragmentProcessor get_child() { return child; }

void main() {
    sk_OutColor = sample(get_child());
}
