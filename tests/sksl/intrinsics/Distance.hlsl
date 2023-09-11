cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _7_pos1 : packoffset(c0);
    float4 _7_pos2 : packoffset(c1);
    float4 _7_colorGreen : packoffset(c2);
    float4 _7_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _21)
{
    float4 expected = float4(3.0f, 3.0f, 5.0f, 13.0f);
    bool _53 = false;
    if (distance(_7_pos1.x, _7_pos2.x) == 3.0f)
    {
        _53 = distance(_7_pos1.xy, _7_pos2.xy) == 3.0f;
    }
    else
    {
        _53 = false;
    }
    bool _65 = false;
    if (_53)
    {
        _65 = distance(_7_pos1.xyz, _7_pos2.xyz) == 5.0f;
    }
    else
    {
        _65 = false;
    }
    bool _74 = false;
    if (_65)
    {
        _74 = distance(_7_pos1, _7_pos2) == 13.0f;
    }
    else
    {
        _74 = false;
    }
    bool _78 = false;
    if (_74)
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
    bool _87 = false;
    if (_84)
    {
        _87 = true;
    }
    else
    {
        _87 = false;
    }
    float4 _88 = 0.0f.xxxx;
    if (_87)
    {
        _88 = _7_colorGreen;
    }
    else
    {
        _88 = _7_colorRed;
    }
    return _88;
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
