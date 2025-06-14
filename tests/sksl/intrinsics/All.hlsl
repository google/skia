cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
    float4 _11_colorRed : packoffset(c1);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    bool4 _44 = bool4(_11_colorRed.xxzw.x != 0.0f, _11_colorRed.xxzw.y != 0.0f, _11_colorRed.xxzw.z != 0.0f, _11_colorRed.xxzw.w != 0.0f);
    bool4 inputVal = _44;
    bool4 _57 = bool4(_11_colorRed.xyzz.x != 0.0f, _11_colorRed.xyzz.y != 0.0f, _11_colorRed.xyzz.z != 0.0f, _11_colorRed.xyzz.w != 0.0f);
    bool4 expected = _57;
    bool _62 = _57.x;
    bool _71 = false;
    if (all(_44.xy) == _62)
    {
        _71 = all(_44.xyz) == _57.y;
    }
    else
    {
        _71 = false;
    }
    bool _77 = false;
    if (_71)
    {
        _77 = all(_44) == _57.z;
    }
    else
    {
        _77 = false;
    }
    bool _80 = false;
    if (_77)
    {
        _80 = _62;
    }
    else
    {
        _80 = false;
    }
    bool _85 = false;
    if (_80)
    {
        _85 = false == _57.y;
    }
    else
    {
        _85 = false;
    }
    bool _90 = false;
    if (_85)
    {
        _90 = false == _57.z;
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
