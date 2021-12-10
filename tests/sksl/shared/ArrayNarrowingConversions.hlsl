cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    int _32[2] = { 1, 2 };
    int i2[2] = _32;
    int _35[2] = { 1, 2 };
    int s2[2] = _35;
    float _41[2] = { 1.0f, 2.0f };
    float f2[2] = _41;
    float _44[2] = { 1.0f, 2.0f };
    float h2[2] = _44;
    i2 = s2;
    s2 = i2;
    f2 = h2;
    h2 = f2;
    float _50[2] = { 1.0f, 2.0f };
    float cf2[2] = _50;
    bool _72 = false;
    if ((i2[1] == s2[1]) && (i2[0] == s2[0]))
    {
        _72 = (f2[1] == h2[1]) && (f2[0] == h2[0]);
    }
    else
    {
        _72 = false;
    }
    bool _84 = false;
    if (_72)
    {
        int _76[2] = { 1, 2 };
        _84 = (i2[1] == _76[1]) && (i2[0] == _76[0]);
    }
    else
    {
        _84 = false;
    }
    bool _96 = false;
    if (_84)
    {
        _96 = (h2[1] == cf2[1]) && (h2[0] == cf2[0]);
    }
    else
    {
        _96 = false;
    }
    float4 _97 = 0.0f.xxxx;
    if (_96)
    {
        _97 = _10_colorGreen;
    }
    else
    {
        _97 = _10_colorRed;
    }
    return _97;
}

void frag_main()
{
    float2 _20 = 0.0f.xx;
    sk_FragColor = main(_20);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
