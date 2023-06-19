float foo_ff(inout float _13[2])
{
    float _18 = _13[1];
    _13[0] = _18;
    return _18;
}

void frag_main()
{
    float y[2] = { 0.0f, 0.0f };
    float _26[2] = y;
    float _27 = foo_ff(_26);
}

void main()
{
    frag_main();
}
