cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _15_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void outParameterWrite_vh4(out float4 _30)
{
    _30 = _15_colorGreen;
}

void outParameterWriteIndirect_vh4(out float4 _36)
{
    float4 _38 = 0.0f.xxxx;
    outParameterWrite_vh4(_38);
    _36 = _38;
}

void inoutParameterWrite_vh4(inout float4 _41)
{
    _41 *= _41;
}

void inoutParameterWriteIndirect_vh4(inout float4 _46)
{
    float4 _49 = _46;
    inoutParameterWrite_vh4(_49);
    _46 = _49;
}

float4 main(float2 _53)
{
    float4 _56 = 0.0f.xxxx;
    outParameterWrite_vh4(_56);
    float4 c = _56;
    float4 _59 = 0.0f.xxxx;
    outParameterWriteIndirect_vh4(_59);
    c = _59;
    float4 _62 = _59;
    inoutParameterWrite_vh4(_62);
    c = _62;
    float4 _65 = _62;
    inoutParameterWriteIndirect_vh4(_65);
    c = _65;
    return _65;
}

void frag_main()
{
    float2 _25 = 0.0f.xx;
    sk_FragColor = main(_25);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
