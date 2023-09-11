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
        float4 _65 = 0.0f.xxxx;
        if (all(bool4(_44.x == _52.x, _44.y == _52.y, _44.z == _52.z, _44.w == _52.w)))
        {
            _65 = _52;
        }
        else
        {
            _65 = _44;
        }
        _59 = _65;
    }
    else
    {
        float4 _72 = 0.0f.xxxx;
        if (any(bool4(_52.x != _44.x, _52.y != _44.y, _52.z != _44.z, _52.w != _44.w)))
        {
            _72 = _36;
        }
        else
        {
            _72 = _10_colorWhite;
        }
        _59 = _72;
    }
    float4 result = _59;
    float4 _82 = 0.0f.xxxx;
    if (all(bool4(_52.x == _36.x, _52.y == _36.y, _52.z == _36.z, _52.w == _36.w)))
    {
        _82 = _10_colorWhite;
    }
    else
    {
        float4 _90 = 0.0f.xxxx;
        if (any(bool4(_52.x != _44.x, _52.y != _44.y, _52.z != _44.z, _52.w != _44.w)))
        {
            _90 = _59;
        }
        else
        {
            float4 _98 = 0.0f.xxxx;
            if (all(bool4(_52.x == _10_colorWhite.x, _52.y == _10_colorWhite.y, _52.z == _10_colorWhite.z, _52.w == _10_colorWhite.w)))
            {
                _98 = _36;
            }
            else
            {
                _98 = _52;
            }
            _90 = _98;
        }
        _82 = _90;
    }
    return _82;
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
