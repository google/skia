
void main() {
    float x = 1.0;
    float y = 2.0;

    int z = 3;
    x = 2.0;
    y = 0.5;
    z = 8;
    bool c = sqrt(2.0) > 2.0;
    bool d = true ^^ c;
    bool e = c;
    x += 12.0;
    x -= 12.0;
    x *= (y /= 10.0);
    z |= 0;
    z &= -1;
    z ^= 0;
    z >>= 2;
    z <<= 4;
    z %= 5;
    x = float((vec2(sqrt(1.0)) , 6));
    y = ((float(c) * float(d)) * float(e) , 6.0);
    z = int((vec2(sqrt(1.0)) , 6));
}
