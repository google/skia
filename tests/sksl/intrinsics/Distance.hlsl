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
    bool _55 = false;
    if (distance(_10_pos1.x, _10_pos2.x) == 3.0f)
    {
        _55 = distance(_10_pos1.xy, _10_pos2.xy) == 3.0f;
    }
    else
    {
        _55 = false;
    }
    bool _67 = false;
    if (_55)
    {
        _67 = distance(_10_pos1.xyz, _10_pos2.xyz) == 5.0f;
    }
    else
    {
        _67 = false;
    }
    bool _76 = false;
    if (_67)
    {
        _76 = distance(_10_pos1, _10_pos2) == 13.0f;
    }
    else
    {
        _76 = false;
    }
    bool _80 = false;
    if (_76)
    {
        _80 = true;
    }
    else
    {
        _80 = false;
    }
    bool _83 = false;
    if (_80)
    {
        _83 = true;
    }
    else
    {
        _83 = false;
    }
    bool _86 = false;
    if (_83)
    {
        _86 = true;
    }
    else
    {
        _86 = false;
    }
    bool _89 = false;
    if (_86)
    {
        _89 = true;
    }
    else
    {
        _89 = false;
    }
    float4 _90 = 0.0f.xxxx;
    if (_89)
    {
        _90 = _10_colorGreen;
    }
    else
    {
        _90 = _10_colorRed;
    }
    return _90;
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
