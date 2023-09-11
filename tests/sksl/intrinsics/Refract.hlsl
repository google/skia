cbuffer _UniformBuffer : register(b0, space0)
{
    float _7_a : packoffset(c0);
    float _7_b : packoffset(c0.y);
    float _7_c : packoffset(c0.z);
    float4 _7_d : packoffset(c1);
    float4 _7_e : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float spvRefract(float i, float n, float eta)
{
    float NoI = n * i;
    float NoI2 = NoI * NoI;
    float k = 1.0 - eta * eta * (1.0 - NoI2);
    if (k < 0.0)
    {
        return 0.0;
    }
    else
    {
        return eta * i - (eta * NoI + sqrt(k)) * n;
    }
}

float4 main(float2 _21)
{
    float4 result = spvRefract(600000015226585740692422656.0f, 2.0f, 2.0f).xxxx;
    result.x = spvRefract(_7_a, _7_b, _7_c);
    result = refract(_7_d, _7_e, _7_c);
    result = float4(float2(0.5f, -0.866025388240814208984375f).x, float2(0.5f, -0.866025388240814208984375f).y, result.z, result.w);
    result = float4(float3(0.5f, 0.0f, -0.866025388240814208984375f).x, float3(0.5f, 0.0f, -0.866025388240814208984375f).y, float3(0.5f, 0.0f, -0.866025388240814208984375f).z, result.w);
    result = float4(0.5f, 0.0f, 0.0f, -0.866025388240814208984375f);
    return float4(0.5f, 0.0f, 0.0f, -0.866025388240814208984375f);
}

void frag_main()
{
    float2 _17 = 0.0f.xx;
    sk_FragColor = main(_17);
}

SPIRV_Cross_Output main()
{
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.sk_FragColor = sk_FragColor;
    return stage_output;
}
