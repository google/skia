cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _11_colorGreen : packoffset(c0);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void outParameterWrite_vh4(out float4 _26)
{
    _26 = _11_colorGreen;
}

void outParameterWriteIndirect_vh4(out float4 _33)
{
    float4 _35 = 0.0f.xxxx;
    outParameterWrite_vh4(_35);
    _33 = _35;
}

void inoutParameterWrite_vh4(inout float4 _38)
{
    _38 *= _38;
}

void inoutParameterWriteIndirect_vh4(inout float4 _43)
{
    float4 _46 = _43;
    inoutParameterWrite_vh4(_46);
    _43 = _46;
}

float4 main(float2 _50)
{
    float4 _53 = 0.0f.xxxx;
    outParameterWrite_vh4(_53);
    float4 c = _53;
    float4 _56 = 0.0f.xxxx;
    outParameterWriteIndirect_vh4(_56);
    c = _56;
    float4 _59 = _56;
    inoutParameterWrite_vh4(_59);
    c = _59;
    float4 _62 = _59;
    inoutParameterWriteIndirect_vh4(_62);
    c = _62;
    return _62;
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
