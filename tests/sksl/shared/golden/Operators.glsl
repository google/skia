
void main() {
    float x = 1.0;
    float y = 2.0;

    int z = 3;
    x = -6.0;
    y = -1.0;
    z = 8;
    bool b = false == true || 2.0 >= sqrt(2.0);
    x += 12.0;
    x -= 12.0;
    x *= (y /= float(z = 10));
    b ||= false;
    b &&= true;
    b ^^= false;
    z |= 0;
    z &= -1;
    z ^= 0;
    z >>= 2;
    z <<= 4;
    z %= 5;
    x = float((vec2(sqrt(1.0)) , 6));
    z = (vec2(sqrt(1.0)) , 6);
}
