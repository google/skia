cbuffer _UniformBuffer : register(b0, space0)
{
    row_major float4x4 _10_testMatrix4x4 : packoffset(c0);
    float4 _10_colorGreen : packoffset(c4);
    float4 _10_colorRed : packoffset(c5);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 inputA = _10_testMatrix4x4[0];
    float4 inputB = _10_testMatrix4x4[1];
    float4 expected = float4(5.0f, 17.0f, 38.0f, 70.0f);
    bool _58 = false;
    if ((_10_testMatrix4x4[0].x * _10_testMatrix4x4[1].x) == 5.0f)
    {
        _58 = dot(_10_testMatrix4x4[0].xy, _10_testMatrix4x4[1].xy) == 17.0f;
    }
    else
    {
        _58 = false;
    }
    bool _66 = false;
    if (_58)
    {
        _66 = dot(_10_testMatrix4x4[0].xyz, _10_testMatrix4x4[1].xyz) == 38.0f;
    }
    else
    {
        _66 = false;
    }
    bool _71 = false;
    if (_66)
    {
        _71 = dot(_10_testMatrix4x4[0], _10_testMatrix4x4[1]) == 70.0f;
    }
    else
    {
        _71 = false;
    }
    bool _75 = false;
    if (_71)
    {
        _75 = true;
    }
    else
    {
        _75 = false;
    }
    bool _78 = false;
    if (_75)
    {
        _78 = true;
    }
    else
    {
        _78 = false;
    }
    bool _81 = false;
    if (_78)
    {
        _81 = true;
    }
    else
    {
        _81 = false;
    }
    bool _84 = false;
    if (_81)
    {
        _84 = true;
    }
    else
    {
        _84 = false;
    }
    float4 _85 = 0.0f.xxxx;
    if (_84)
    {
        _85 = _10_colorGreen;
    }
    else
    {
        _85 = _10_colorRed;
    }
    return _85;
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
