in fragmentProcessor child;

half4 main(float2 coord) {
    return sample(child) + sample(child, coord / 2);
}
