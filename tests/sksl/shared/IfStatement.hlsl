cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _8_colorWhite : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 ifElseTest_h4h4h4h4(float4 _23, float4 _24, float4 _25)
{
    float4 result = 0.0f.xxxx;
    if (any(bool4(_8_colorWhite.x != _23.x, _8_colorWhite.y != _23.y, _8_colorWhite.z != _23.z, _8_colorWhite.w != _23.w)))
    {
        if (all(bool4(_24.x == _25.x, _24.y == _25.y, _24.z == _25.z, _24.w == _25.w)))
        {
            result = _25;
        }
        else
        {
            result = _24;
        }
    }
    else
    {
        if (any(bool4(_25.x != _24.x, _25.y != _24.y, _25.z != _24.z, _25.w != _24.w)))
        {
            result = _23;
        }
        else
        {
            result = _8_colorWhite;
        }
    }
    if (all(bool4(_25.x == _23.x, _25.y == _23.y, _25.z == _23.z, _25.w == _23.w)))
    {
        return _8_colorWhite;
    }
    if (any(bool4(_25.x != _24.x, _25.y != _24.y, _25.z != _24.z, _25.w != _24.w)))
    {
        return result;
    }
    if (all(bool4(_25.x == _8_colorWhite.x, _25.y == _8_colorWhite.y, _25.z == _8_colorWhite.z, _25.w == _8_colorWhite.w)))
    {
        return _23;
    }
    return _25;
}

float4 main(float2 _86)
{
    float4 _93 = float4(0.0f, 0.0f, _8_colorWhite.z, 1.0f);
    float4 _98 = float4(0.0f, _8_colorWhite.y, 0.0f, 1.0f);
    float4 _103 = float4(_8_colorWhite.x, 0.0f, 0.0f, 1.0f);
    return ifElseTest_h4h4h4h4(_93, _98, _103);
}

void frag_main()
{
    float2 _18 = 0.0f.xx;
    sk_FragColor = main(_18);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
