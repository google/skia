in fragmentProcessor child1;
in fragmentProcessor child2;

half4 main(float2 coord) {
    return sample(coord.x > 10 ? child1 : child2);
}
