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
    int4 expected = int4(1, 0, 0, 2);
    bool _61 = false;
    if (abs(int(_10_testInputs.x)) == expected.x)
    {
        int2 _46 = abs(int2(int(_10_testInputs.xy.x), int(_10_testInputs.xy.y)));
        _61 = all(bool2(_46.x == expected.xy.x, _46.y == expected.xy.y));
    }
    else
    {
        _61 = false;
    }
    bool _82 = false;
    if (_61)
    {
        int3 _64 = abs(int3(int(_10_testInputs.xyz.x), int(_10_testInputs.xyz.y), int(_10_testInputs.xyz.z)));
        _82 = all(bool3(_64.x == expected.xyz.x, _64.y == expected.xyz.y, _64.z == expected.xyz.z));
    }
    else
    {
        _82 = false;
    }
    bool _101 = false;
    if (_82)
    {
        int4 _85 = abs(int4(int(_10_testInputs.x), int(_10_testInputs.y), int(_10_testInputs.z), int(_10_testInputs.w)));
        _101 = all(bool4(_85.x == expected.x, _85.y == expected.y, _85.z == expected.z, _85.w == expected.w));
    }
    else
    {
        _101 = false;
    }
    bool _107 = false;
    if (_101)
    {
        _107 = 1 == expected.x;
    }
    else
    {
        _107 = false;
    }
    bool _115 = false;
    if (_107)
    {
        _115 = all(bool2(int2(1, 0).x == expected.xy.x, int2(1, 0).y == expected.xy.y));
    }
    else
    {
        _115 = false;
    }
    bool _123 = false;
    if (_115)
    {
        _123 = all(bool3(int3(1, 0, 0).x == expected.xyz.x, int3(1, 0, 0).y == expected.xyz.y, int3(1, 0, 0).z == expected.xyz.z));
    }
    else
    {
        _123 = false;
    }
    bool _129 = false;
    if (_123)
    {
        _129 = all(bool4(int4(1, 0, 0, 2).x == expected.x, int4(1, 0, 0, 2).y == expected.y, int4(1, 0, 0, 2).z == expected.z, int4(1, 0, 0, 2).w == expected.w));
    }
    else
    {
        _129 = false;
    }
    float4 _130 = 0.0f.xxxx;
    if (_129)
    {
        _130 = _10_colorGreen;
    }
    else
    {
        _130 = _10_colorRed;
    }
    return _130;
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
