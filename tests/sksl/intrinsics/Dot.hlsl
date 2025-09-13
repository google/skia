cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float4x4 _11_testMatrix4x4 : packoffset(c0);
    float4 _11_colorGreen : packoffset(c4);
    float4 _11_colorRed : packoffset(c5);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _26)
{
    float4 inputA = _11_testMatrix4x4[0];
    float4 inputB = _11_testMatrix4x4[1];
    float4 expected = float4(5.0f, 17.0f, 38.0f, 70.0f);
    bool _59 = false;
    if ((_11_testMatrix4x4[0].x * _11_testMatrix4x4[1].x) == 5.0f)
    {
        _59 = dot(_11_testMatrix4x4[0].xy, _11_testMatrix4x4[1].xy) == 17.0f;
    }
    else
    {
        _59 = false;
    }
    bool _67 = false;
    if (_59)
    {
        _67 = dot(_11_testMatrix4x4[0].xyz, _11_testMatrix4x4[1].xyz) == 38.0f;
    }
    else
    {
        _67 = false;
    }
    bool _72 = false;
    if (_67)
    {
        _72 = dot(_11_testMatrix4x4[0], _11_testMatrix4x4[1]) == 70.0f;
    }
    else
    {
        _72 = false;
    }
    bool _76 = false;
    if (_72)
    {
        _76 = true;
    }
    else
    {
        _76 = false;
    }
    bool _79 = false;
    if (_76)
    {
        _79 = true;
    }
    else
    {
        _79 = false;
    }
    bool _82 = false;
    if (_79)
    {
        _82 = true;
    }
    else
    {
        _82 = false;
    }
    bool _85 = false;
    if (_82)
    {
        _85 = true;
    }
    else
    {
        _85 = false;
    }
    float4 _86 = 0.0f.xxxx;
    if (_85)
    {
        _86 = _11_colorGreen;
    }
    else
    {
        _86 = _11_colorRed;
    }
    return _86;
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
