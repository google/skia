cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float4x4 _7_testMatrix4x4 : packoffset(c0);
    float4 _7_colorGreen : packoffset(c4);
    float4 _7_colorRed : packoffset(c5);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _22)
{
    float4 inputA = _7_testMatrix4x4[0];
    float4 inputB = _7_testMatrix4x4[1];
    float4 expected = float4(5.0f, 17.0f, 38.0f, 70.0f);
    bool _56 = false;
    if ((_7_testMatrix4x4[0].x * _7_testMatrix4x4[1].x) == 5.0f)
    {
        _56 = dot(_7_testMatrix4x4[0].xy, _7_testMatrix4x4[1].xy) == 17.0f;
    }
    else
    {
        _56 = false;
    }
    bool _64 = false;
    if (_56)
    {
        _64 = dot(_7_testMatrix4x4[0].xyz, _7_testMatrix4x4[1].xyz) == 38.0f;
    }
    else
    {
        _64 = false;
    }
    bool _69 = false;
    if (_64)
    {
        _69 = dot(_7_testMatrix4x4[0], _7_testMatrix4x4[1]) == 70.0f;
    }
    else
    {
        _69 = false;
    }
    bool _73 = false;
    if (_69)
    {
        _73 = true;
    }
    else
    {
        _73 = false;
    }
    bool _76 = false;
    if (_73)
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
    float4 _83 = 0.0f.xxxx;
    if (_82)
    {
        _83 = _7_colorGreen;
    }
    else
    {
        _83 = _7_colorRed;
    }
    return _83;
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
