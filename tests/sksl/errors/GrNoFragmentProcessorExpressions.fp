in fragmentProcessor child1;
in fragmentProcessor child2;

void main(float2 coord) {
    sk_OutColor = sample(coord.x > 10 ? child1 : child2);
}
