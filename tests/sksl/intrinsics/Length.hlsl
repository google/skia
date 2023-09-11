cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_testMatrix2x2 : packoffset(c0);
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
    float4 _35 = _7_testMatrix2x2 + float4(2.0f, -2.0f, 1.0f, 8.0f);
    float4 inputVal = _35;
    float4 expected = float4(3.0f, 3.0f, 5.0f, 13.0f);
    bool _56 = false;
    if (abs(length(_35.x) - 3.0f) < 0.0500000007450580596923828125f)
    {
        _56 = abs(length(_35.xy) - 3.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _56 = false;
    }
    bool _65 = false;
    if (_56)
    {
        _65 = abs(length(_35.xyz) - 5.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _65 = false;
    }
    bool _72 = false;
    if (_65)
    {
        _72 = abs(length(_35) - 13.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _72 = false;
    }
    bool _78 = false;
    if (_72)
    {
        _78 = abs(3.0f - 3.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _78 = false;
    }
    bool _84 = false;
    if (_78)
    {
        _84 = abs(3.0f - 3.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _84 = false;
    }
    bool _90 = false;
    if (_84)
    {
        _90 = abs(5.0f - 5.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _90 = false;
    }
    bool _96 = false;
    if (_90)
    {
        _96 = abs(13.0f - 13.0f) < 0.0500000007450580596923828125f;
    }
    else
    {
        _96 = false;
    }
    float4 _97 = 0.0f.xxxx;
    if (_96)
    {
        _97 = _7_colorGreen;
    }
    else
    {
        _97 = _7_colorRed;
    }
    return _97;
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
