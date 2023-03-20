cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_testInputs : packoffset(c0);
    float4 _10_colorGreen : packoffset(c1);
    float4 _10_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    int4 expected = int4(-1, 0, 0, 1);
    bool _58 = false;
    if (sign(int(_10_testInputs.x)) == (-1))
    {
        int2 _44 = sign(int2(int(_10_testInputs.xy.x), int(_10_testInputs.xy.y)));
        _58 = all(bool2(_44.x == int4(-1, 0, 0, 1).xy.x, _44.y == int4(-1, 0, 0, 1).xy.y));
    }
    else
    {
        _58 = false;
    }
    bool _78 = false;
    if (_58)
    {
        int3 _61 = sign(int3(int(_10_testInputs.xyz.x), int(_10_testInputs.xyz.y), int(_10_testInputs.xyz.z)));
        _78 = all(bool3(_61.x == int4(-1, 0, 0, 1).xyz.x, _61.y == int4(-1, 0, 0, 1).xyz.y, _61.z == int4(-1, 0, 0, 1).xyz.z));
    }
    else
    {
        _78 = false;
    }
    bool _96 = false;
    if (_78)
    {
        int4 _81 = sign(int4(int(_10_testInputs.x), int(_10_testInputs.y), int(_10_testInputs.z), int(_10_testInputs.w)));
        _96 = all(bool4(_81.x == int4(-1, 0, 0, 1).x, _81.y == int4(-1, 0, 0, 1).y, _81.z == int4(-1, 0, 0, 1).z, _81.w == int4(-1, 0, 0, 1).w));
    }
    else
    {
        _96 = false;
    }
    bool _100 = false;
    if (_96)
    {
        _100 = true;
    }
    else
    {
        _100 = false;
    }
    bool _107 = false;
    if (_100)
    {
        _107 = all(bool2(int2(-1, 0).x == int4(-1, 0, 0, 1).xy.x, int2(-1, 0).y == int4(-1, 0, 0, 1).xy.y));
    }
    else
    {
        _107 = false;
    }
    bool _114 = false;
    if (_107)
    {
        _114 = all(bool3(int3(-1, 0, 0).x == int4(-1, 0, 0, 1).xyz.x, int3(-1, 0, 0).y == int4(-1, 0, 0, 1).xyz.y, int3(-1, 0, 0).z == int4(-1, 0, 0, 1).xyz.z));
    }
    else
    {
        _114 = false;
    }
    bool _117 = false;
    if (_114)
    {
        _117 = true;
    }
    else
    {
        _117 = false;
    }
    float4 _118 = 0.0f.xxxx;
    if (_117)
    {
        _118 = _10_colorGreen;
    }
    else
    {
        _118 = _10_colorRed;
    }
    return _118;
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
