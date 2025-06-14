cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_pos1 : packoffset(c0);
    float4 _11_pos2 : packoffset(c1);
    float4 _11_colorGreen : packoffset(c2);
    float4 _11_colorRed : packoffset(c3);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 expected = float4(3.0f, 3.0f, 5.0f, 13.0f);
    bool _56 = false;
    if (distance(_11_pos1.x, _11_pos2.x) == 3.0f)
    {
        _56 = distance(_11_pos1.xy, _11_pos2.xy) == 3.0f;
    }
    else
    {
        _56 = false;
    }
    bool _68 = false;
    if (_56)
    {
        _68 = distance(_11_pos1.xyz, _11_pos2.xyz) == 5.0f;
    }
    else
    {
        _68 = false;
    }
    bool _77 = false;
    if (_68)
    {
        _77 = distance(_11_pos1, _11_pos2) == 13.0f;
    }
    else
    {
        _77 = false;
    }
    bool _81 = false;
    if (_77)
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
    bool _90 = false;
    if (_87)
    {
        _90 = true;
    }
    else
    {
        _90 = false;
    }
    float4 _91 = 0.0f.xxxx;
    if (_90)
    {
        _91 = _11_colorGreen;
    }
    else
    {
        _91 = _11_colorRed;
    }
    return _91;
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
