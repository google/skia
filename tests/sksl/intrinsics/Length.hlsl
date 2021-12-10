cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_inputVal : packoffset(c0);
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
    float4 expected = float4(3.0f, 3.0f, 5.0f, 13.0f);
    bool _52 = false;
    if (length(_10_inputVal.x) == expected.x)
    {
        _52 = length(_10_inputVal.xy) == expected.y;
    }
    else
    {
        _52 = false;
    }
    bool _63 = false;
    if (_52)
    {
        _63 = length(_10_inputVal.xyz) == expected.z;
    }
    else
    {
        _63 = false;
    }
    bool _72 = false;
    if (_63)
    {
        _72 = length(_10_inputVal) == expected.w;
    }
    else
    {
        _72 = false;
    }
    bool _78 = false;
    if (_72)
    {
        _78 = 3.0f == expected.x;
    }
    else
    {
        _78 = false;
    }
    bool _84 = false;
    if (_78)
    {
        _84 = 3.0f == expected.y;
    }
    else
    {
        _84 = false;
    }
    bool _90 = false;
    if (_84)
    {
        _90 = 5.0f == expected.z;
    }
    else
    {
        _90 = false;
    }
    bool _96 = false;
    if (_90)
    {
        _96 = 13.0f == expected.w;
    }
    else
    {
        _96 = false;
    }
    float4 _97 = 0.0f.xxxx;
    if (_96)
    {
        _97 = _10_colorGreen;
    }
    else
    {
        _97 = _10_colorRed;
    }
    return _97;
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
