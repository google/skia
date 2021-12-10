cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_pos1 : packoffset(c0);
    float4 _10_pos2 : packoffset(c1);
    float4 _10_colorGreen : packoffset(c2);
    float4 _10_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 expected = float4(3.0f, 3.0f, 5.0f, 13.0f);
    bool _59 = false;
    if (distance(_10_pos1.x, _10_pos2.x) == expected.x)
    {
        _59 = distance(_10_pos1.xy, _10_pos2.xy) == expected.y;
    }
    else
    {
        _59 = false;
    }
    bool _73 = false;
    if (_59)
    {
        _73 = distance(_10_pos1.xyz, _10_pos2.xyz) == expected.z;
    }
    else
    {
        _73 = false;
    }
    bool _84 = false;
    if (_73)
    {
        _84 = distance(_10_pos1, _10_pos2) == expected.w;
    }
    else
    {
        _84 = false;
    }
    bool _90 = false;
    if (_84)
    {
        _90 = 3.0f == expected.x;
    }
    else
    {
        _90 = false;
    }
    bool _96 = false;
    if (_90)
    {
        _96 = 3.0f == expected.y;
    }
    else
    {
        _96 = false;
    }
    bool _102 = false;
    if (_96)
    {
        _102 = 5.0f == expected.z;
    }
    else
    {
        _102 = false;
    }
    bool _108 = false;
    if (_102)
    {
        _108 = 13.0f == expected.w;
    }
    else
    {
        _108 = false;
    }
    float4 _109 = 0.0f.xxxx;
    if (_108)
    {
        _109 = _10_colorGreen;
    }
    else
    {
        _109 = _10_colorRed;
    }
    return _109;
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
