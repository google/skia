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
    float4 expected = 0.0f.xxxx;
    bool _51 = false;
    if (ddx(_10_testInputs.x) == expected.x)
    {
        float2 _42 = ddx(_10_testInputs.xy);
        _51 = all(bool2(_42.x == expected.xy.x, _42.y == expected.xy.y));
    }
    else
    {
        _51 = false;
    }
    bool _64 = false;
    if (_51)
    {
        float3 _54 = ddx(_10_testInputs.xyz);
        _64 = all(bool3(_54.x == expected.xyz.x, _54.y == expected.xyz.y, _54.z == expected.xyz.z));
    }
    else
    {
        _64 = false;
    }
    bool _74 = false;
    if (_64)
    {
        float4 _67 = ddx(_10_testInputs);
        _74 = all(bool4(_67.x == expected.x, _67.y == expected.y, _67.z == expected.z, _67.w == expected.w));
    }
    else
    {
        _74 = false;
    }
    bool _85 = false;
    if (_74)
    {
        float2 _77 = sign(fwidth(_24.xx));
        _85 = all(bool2(_77.x == 1.0f.xx.x, _77.y == 1.0f.xx.y));
    }
    else
    {
        _85 = false;
    }
    bool _96 = false;
    if (_85)
    {
        float2 _88 = sign(fwidth(float2(_24.x, 1.0f)));
        _96 = all(bool2(_88.x == float2(1.0f, 0.0f).x, _88.y == float2(1.0f, 0.0f).y));
    }
    else
    {
        _96 = false;
    }
    bool _105 = false;
    if (_96)
    {
        float2 _99 = sign(fwidth(_24.yy));
        _105 = all(bool2(_99.x == 1.0f.xx.x, _99.y == 1.0f.xx.y));
    }
    else
    {
        _105 = false;
    }
    bool _116 = false;
    if (_105)
    {
        float2 _108 = sign(fwidth(float2(0.0f, _24.y)));
        _116 = all(bool2(_108.x == float2(0.0f, 1.0f).x, _108.y == float2(0.0f, 1.0f).y));
    }
    else
    {
        _116 = false;
    }
    bool _124 = false;
    if (_116)
    {
        float2 _119 = sign(fwidth(_24));
        _124 = all(bool2(_119.x == 1.0f.xx.x, _119.y == 1.0f.xx.y));
    }
    else
    {
        _124 = false;
    }
    float4 _125 = 0.0f.xxxx;
    if (_124)
    {
        _125 = _10_colorGreen;
    }
    else
    {
        _125 = _10_colorRed;
    }
    return _125;
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
