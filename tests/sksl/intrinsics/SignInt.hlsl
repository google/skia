cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_testInputs : packoffset(c0);
    float4 _7_colorGreen : packoffset(c1);
    float4 _7_colorRed : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    int4 expected = int4(-1, 0, 0, 1);
    bool _56 = false;
    if (sign(int(_7_testInputs.x)) == (-1))
    {
        int2 _42 = sign(int2(int(_7_testInputs.xy.x), int(_7_testInputs.xy.y)));
        _56 = all(bool2(_42.x == int4(-1, 0, 0, 1).xy.x, _42.y == int4(-1, 0, 0, 1).xy.y));
    }
    else
    {
        _56 = false;
    }
    bool _76 = false;
    if (_56)
    {
        int3 _59 = sign(int3(int(_7_testInputs.xyz.x), int(_7_testInputs.xyz.y), int(_7_testInputs.xyz.z)));
        _76 = all(bool3(_59.x == int4(-1, 0, 0, 1).xyz.x, _59.y == int4(-1, 0, 0, 1).xyz.y, _59.z == int4(-1, 0, 0, 1).xyz.z));
    }
    else
    {
        _76 = false;
    }
    bool _94 = false;
    if (_76)
    {
        int4 _79 = sign(int4(int(_7_testInputs.x), int(_7_testInputs.y), int(_7_testInputs.z), int(_7_testInputs.w)));
        _94 = all(bool4(_79.x == int4(-1, 0, 0, 1).x, _79.y == int4(-1, 0, 0, 1).y, _79.z == int4(-1, 0, 0, 1).z, _79.w == int4(-1, 0, 0, 1).w));
    }
    else
    {
        _94 = false;
    }
    bool _98 = false;
    if (_94)
    {
        _98 = true;
    }
    else
    {
        _98 = false;
    }
    bool _105 = false;
    if (_98)
    {
        _105 = all(bool2(int2(-1, 0).x == int4(-1, 0, 0, 1).xy.x, int2(-1, 0).y == int4(-1, 0, 0, 1).xy.y));
    }
    else
    {
        _105 = false;
    }
    bool _112 = false;
    if (_105)
    {
        _112 = all(bool3(int3(-1, 0, 0).x == int4(-1, 0, 0, 1).xyz.x, int3(-1, 0, 0).y == int4(-1, 0, 0, 1).xyz.y, int3(-1, 0, 0).z == int4(-1, 0, 0, 1).xyz.z));
    }
    else
    {
        _112 = false;
    }
    bool _115 = false;
    if (_112)
    {
        _115 = true;
    }
    else
    {
        _115 = false;
    }
    float4 _116 = 0.0f.xxxx;
    if (_115)
    {
        _116 = _7_colorGreen;
    }
    else
    {
        _116 = _7_colorRed;
    }
    return _116;
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
