in fragmentProcessor child1;
in fragmentProcessor child2;

void main() {
    sk_OutColor = sample(child1) * sample(child2);
}
