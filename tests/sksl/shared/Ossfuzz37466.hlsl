float foo_ff(inout float _10[2])
{
    float _15 = _10[1];
    _10[0] = _15;
    return _15;
}

void frag_main()
{
    float y[2] = { 0.0f, 0.0f };
    float _23[2] = y;
    float _24 = foo_ff(_23);
}

void main()
{
    frag_main();
}
