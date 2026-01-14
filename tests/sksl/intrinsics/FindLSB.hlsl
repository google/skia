cbuffer _UniformBuffer : register(b0, space0)
{
    int _11_a : packoffset(c0);
    uint _11_b : packoffset(c0.y);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

void frag_main()
{
    int _30 = firstbitlow(_11_a) + int(firstbitlow(_11_b));
    int b1 = _30;
    int2 _43 = firstbitlow(_11_a.xx) + int2(firstbitlow(_11_b.xx));
    int2 b2 = _43;
    int3 _56 = firstbitlow(_11_a.xxx) + int3(firstbitlow(_11_b.xxx));
    int3 b3 = _56;
    int4 _69 = firstbitlow(_11_a.xxxx) + int4(firstbitlow(_11_b.xxxx));
    int4 b4 = _69;
    int4 _78 = ((_30.xxxx + _43.xyxy) + int4(_56, 1)) + _69;
    sk_FragColor = float4(float(_78.x), float(_78.y), float(_78.z), float(_78.w));
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
