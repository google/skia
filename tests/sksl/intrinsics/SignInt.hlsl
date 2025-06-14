cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_testInputs : packoffset(c0);
    float4 _11_colorGreen : packoffset(c1);
    float4 _11_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    int4 expected = int4(-1, 0, 0, 1);
    bool _59 = false;
    if (sign(int(_11_testInputs.x)) == (-1))
    {
        int2 _45 = sign(int2(int(_11_testInputs.xy.x), int(_11_testInputs.xy.y)));
        _59 = all(bool2(_45.x == int4(-1, 0, 0, 1).xy.x, _45.y == int4(-1, 0, 0, 1).xy.y));
    }
    else
    {
        _59 = false;
    }
    bool _79 = false;
    if (_59)
    {
        int3 _62 = sign(int3(int(_11_testInputs.xyz.x), int(_11_testInputs.xyz.y), int(_11_testInputs.xyz.z)));
        _79 = all(bool3(_62.x == int4(-1, 0, 0, 1).xyz.x, _62.y == int4(-1, 0, 0, 1).xyz.y, _62.z == int4(-1, 0, 0, 1).xyz.z));
    }
    else
    {
        _79 = false;
    }
    bool _97 = false;
    if (_79)
    {
        int4 _82 = sign(int4(int(_11_testInputs.x), int(_11_testInputs.y), int(_11_testInputs.z), int(_11_testInputs.w)));
        _97 = all(bool4(_82.x == int4(-1, 0, 0, 1).x, _82.y == int4(-1, 0, 0, 1).y, _82.z == int4(-1, 0, 0, 1).z, _82.w == int4(-1, 0, 0, 1).w));
    }
    else
    {
        _97 = false;
    }
    bool _101 = false;
    if (_97)
    {
        _101 = true;
    }
    else
    {
        _101 = false;
    }
    bool _108 = false;
    if (_101)
    {
        _108 = all(bool2(int2(-1, 0).x == int4(-1, 0, 0, 1).xy.x, int2(-1, 0).y == int4(-1, 0, 0, 1).xy.y));
    }
    else
    {
        _108 = false;
    }
    bool _115 = false;
    if (_108)
    {
        _115 = all(bool3(int3(-1, 0, 0).x == int4(-1, 0, 0, 1).xyz.x, int3(-1, 0, 0).y == int4(-1, 0, 0, 1).xyz.y, int3(-1, 0, 0).z == int4(-1, 0, 0, 1).xyz.z));
    }
    else
    {
        _115 = false;
    }
    bool _118 = false;
    if (_115)
    {
        _118 = true;
    }
    else
    {
        _118 = false;
    }
    float4 _119 = 0.0f.xxxx;
    if (_118)
    {
        _119 = _11_colorGreen;
    }
    else
    {
        _119 = _11_colorRed;
    }
    return _119;
}

void frag_main()
{
    float2 _21 = 0.0f.xx;
    sk_FragColor = main(_21);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
