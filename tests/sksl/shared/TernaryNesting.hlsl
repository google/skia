cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorWhite : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _24)
{
    float4 _36 = float4(0.0f, 0.0f, _10_colorWhite.zw);
    float4 colorBlue = _36;
    float4 _44 = float4(0.0f, _10_colorWhite.y, 0.0f, _10_colorWhite.w);
    float4 colorGreen = _44;
    float4 _52 = float4(_10_colorWhite.x, 0.0f, 0.0f, _10_colorWhite.w);
    float4 colorRed = _52;
    float4 _59 = 0.0f.xxxx;
    if (any(bool4(_10_colorWhite.x != _36.x, _10_colorWhite.y != _36.y, _10_colorWhite.z != _36.z, _10_colorWhite.w != _36.w)))
    {
        bool4 _65 = all(bool4(_44.x == _52.x, _44.y == _52.y, _44.z == _52.z, _44.w == _52.w)).xxxx;
        _59 = float4(_65.x ? _52.x : _44.x, _65.y ? _52.y : _44.y, _65.z ? _52.z : _44.z, _65.w ? _52.w : _44.w);
    }
    else
    {
        bool4 _69 = any(bool4(_52.x != _44.x, _52.y != _44.y, _52.z != _44.z, _52.w != _44.w)).xxxx;
        _59 = float4(_69.x ? _36.x : _10_colorWhite.x, _69.y ? _36.y : _10_colorWhite.y, _69.z ? _36.z : _10_colorWhite.z, _69.w ? _36.w : _10_colorWhite.w);
    }
    float4 result = _59;
    float4 _76 = 0.0f.xxxx;
    if (all(bool4(_52.x == _36.x, _52.y == _36.y, _52.z == _36.z, _52.w == _36.w)))
    {
        _76 = _10_colorWhite;
    }
    else
    {
        float4 _84 = 0.0f.xxxx;
        if (any(bool4(_52.x != _44.x, _52.y != _44.y, _52.z != _44.z, _52.w != _44.w)))
        {
            _84 = _59;
        }
        else
        {
            bool4 _92 = all(bool4(_52.x == _10_colorWhite.x, _52.y == _10_colorWhite.y, _52.z == _10_colorWhite.z, _52.w == _10_colorWhite.w)).xxxx;
            _84 = float4(_92.x ? _36.x : _52.x, _92.y ? _36.y : _52.y, _92.z ? _36.z : _52.z, _92.w ? _36.w : _52.w);
        }
        _76 = _84;
    }
    return _76;
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
