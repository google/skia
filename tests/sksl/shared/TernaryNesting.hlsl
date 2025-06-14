cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorWhite : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 _36 = float4(0.0f, 0.0f, _11_colorWhite.zw);
    float4 colorBlue = _36;
    float4 _44 = float4(0.0f, _11_colorWhite.y, 0.0f, _11_colorWhite.w);
    float4 colorGreen = _44;
    float4 _52 = float4(_11_colorWhite.x, 0.0f, 0.0f, _11_colorWhite.w);
    float4 colorRed = _52;
    float4 _60 = 0.0f.xxxx;
    if (any(bool4(_11_colorWhite.x != _36.x, _11_colorWhite.y != _36.y, _11_colorWhite.z != _36.z, _11_colorWhite.w != _36.w)))
    {
        float4 _66 = 0.0f.xxxx;
        if (all(bool4(_44.x == _52.x, _44.y == _52.y, _44.z == _52.z, _44.w == _52.w)))
        {
            _66 = _52;
        }
        else
        {
            _66 = _44;
        }
        _60 = _66;
    }
    else
    {
        float4 _73 = 0.0f.xxxx;
        if (any(bool4(_52.x != _44.x, _52.y != _44.y, _52.z != _44.z, _52.w != _44.w)))
        {
            _73 = _36;
        }
        else
        {
            _73 = _11_colorWhite;
        }
        _60 = _73;
    }
    float4 result = _60;
    float4 _83 = 0.0f.xxxx;
    if (all(bool4(_52.x == _36.x, _52.y == _36.y, _52.z == _36.z, _52.w == _36.w)))
    {
        _83 = _11_colorWhite;
    }
    else
    {
        float4 _91 = 0.0f.xxxx;
        if (any(bool4(_52.x != _44.x, _52.y != _44.y, _52.z != _44.z, _52.w != _44.w)))
        {
            _91 = _60;
        }
        else
        {
            float4 _99 = 0.0f.xxxx;
            if (all(bool4(_52.x == _11_colorWhite.x, _52.y == _11_colorWhite.y, _52.z == _11_colorWhite.z, _52.w == _11_colorWhite.w)))
            {
                _99 = _36;
            }
            else
            {
                _99 = _52;
            }
            _91 = _99;
        }
        _83 = _91;
    }
    return _83;
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
