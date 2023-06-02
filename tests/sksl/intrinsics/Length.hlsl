cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_testMatrix2x2 : packoffset(c0);
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
    float4 _38 = _10_testMatrix2x2 + float4(2.0f, -2.0f, 1.0f, 8.0f);
    float4 inputVal = _38;
    float4 expected = float4(3.0f, 3.0f, 5.0f, 13.0f);
    bool _58 = false;
    if (abs(length(_38.x) - 3.0f) < 0.0500000007450580596923828125f)
    {
        _58 = abs(length(_38.xy) - 3.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _58 = false;
    }
    bool _67 = false;
    if (_58)
    {
        _67 = abs(length(_38.xyz) - 5.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _67 = false;
    }
    bool _74 = false;
    if (_67)
    {
        _74 = abs(length(_38) - 13.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _74 = false;
    }
    bool _80 = false;
    if (_74)
    {
        _80 = abs(3.0f - 3.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _80 = false;
    }
    bool _86 = false;
    if (_80)
    {
        _86 = abs(3.0f - 3.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _86 = false;
    }
    bool _92 = false;
    if (_86)
    {
        _92 = abs(5.0f - 5.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _92 = false;
    }
    bool _98 = false;
    if (_92)
    {
        _98 = abs(13.0f - 13.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _98 = false;
    }
    bool4 _100 = _98.xxxx;
    return float4(_100.x ? _10_colorGreen.x : _10_colorRed.x, _100.y ? _10_colorGreen.y : _10_colorRed.y, _100.z ? _10_colorGreen.z : _10_colorRed.z, _100.w ? _10_colorGreen.w : _10_colorRed.w);
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
