cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _12_colorWhite : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 ifElseTest_h4h4h4h4(float4 _27, float4 _28, float4 _29)
{
    float4 result = 0.0f.xxxx;
    if (any(bool4(_12_colorWhite.x != _27.x, _12_colorWhite.y != _27.y, _12_colorWhite.z != _27.z, _12_colorWhite.w != _27.w)))
    {
        if (all(bool4(_28.x == _29.x, _28.y == _29.y, _28.z == _29.z, _28.w == _29.w)))
        {
            result = _29;
        }
        else
        {
            result = _28;
        }
    }
    else
    {
        if (any(bool4(_29.x != _28.x, _29.y != _28.y, _29.z != _28.z, _29.w != _28.w)))
        {
            result = _27;
        }
        else
        {
            result = _12_colorWhite;
        }
    }
    if (all(bool4(_29.x == _27.x, _29.y == _27.y, _29.z == _27.z, _29.w == _27.w)))
    {
        return _12_colorWhite;
    }
    if (any(bool4(_29.x != _28.x, _29.y != _28.y, _29.z != _28.z, _29.w != _28.w)))
    {
        return result;
    }
    if (all(bool4(_29.x == _12_colorWhite.x, _29.y == _12_colorWhite.y, _29.z == _12_colorWhite.z, _29.w == _12_colorWhite.w)))
    {
        return _27;
    }
    return _29;
}

float4 main(float2 _89)
{
    float4 _96 = float4(0.0f, 0.0f, _12_colorWhite.z, 1.0f);
    float4 _101 = float4(0.0f, _12_colorWhite.y, 0.0f, 1.0f);
    float4 _106 = float4(_12_colorWhite.x, 0.0f, 0.0f, 1.0f);
    return ifElseTest_h4h4h4h4(_96, _101, _106);
}

void frag_main()
{
    float2 _22 = 0.0f.xx;
    sk_FragColor = main(_22);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
