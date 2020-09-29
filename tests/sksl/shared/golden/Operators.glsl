
void main() {
    float x = 1.0, y = 2.0;
    int z = 3;
    x = (x - x) + (6.0 * x) * -1.0;
    y = (x / y) / 3.0;
    z = (((z / 2) % 3 << 4) >> 2) << 1;
    bool b = x > 4.0 == x < 2.0 || 2.0 >= sqrt(2.0) && y <= float(z);
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
