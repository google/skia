cbuffer _UniformBuffer : register(b0, space0)
{
    float4 _10_colorGreen : packoffset(c0);
    float4 _10_colorRed : packoffset(c1);
    row_major float2x2 _10_testMatrix2x2 : packoffset(c2);
};


static float4 sk_FragColor;

struct SPIRV_Cross_Output
{
    float4 sk_FragColor : SV_Target0;
};

float4 main(float2 _25)
{
    float4 f4 = float4(_10_testMatrix2x2[0].x, _10_testMatrix2x2[0].y, _10_testMatrix2x2[1].x, _10_testMatrix2x2[1].y);
    float2x3 _53 = float2x3(float3(f4.xyz), float3(f4.w, f4.xy));
    float2x3 _61 = float2x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 1.0f, 2.0f));
    float3 _63 = _53[0];
    float3 _64 = _61[0];
    float3 _67 = _53[1];
    float3 _68 = _61[1];
    bool ok = all(bool3(_63.x == _64.x, _63.y == _64.y, _63.z == _64.z)) && all(bool3(_67.x == _68.x, _67.y == _68.y, _67.z == _68.z));
    bool _106 = false;
    if (ok)
    {
        float2x4 _91 = float2x4(float4(f4.xyz, f4.wxyz.x), float4(f4.wxyz.yzw, f4.w));
        float2x4 _95 = float2x4(float4(1.0f, 2.0f, 3.0f, 4.0f), float4(1.0f, 2.0f, 3.0f, 4.0f));
        float4 _97 = _91[0];
        float4 _98 = _95[0];
        float4 _101 = _91[1];
        float4 _102 = _95[1];
        _106 = all(bool4(_97.x == _98.x, _97.y == _98.y, _97.z == _98.z, _97.w == _98.w)) && all(bool4(_101.x == _102.x, _101.y == _102.y, _101.z == _102.z, _101.w == _102.w));
    }
    else
    {
        _106 = false;
    }
    ok = _106;
    bool _148 = false;
    if (ok)
    {
        float3x3 _128 = float3x3(float3(f4.xy, f4.zw.x), float3(f4.zw.y, f4.xy), float3(f4.zw, f4.x));
        float3x3 _133 = float3x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 1.0f, 2.0f), float3(3.0f, 4.0f, 1.0f));
        float3 _134 = _128[0];
        float3 _135 = _133[0];
        float3 _138 = _128[1];
        float3 _139 = _133[1];
        float3 _143 = _128[2];
        float3 _144 = _133[2];
        _148 = (all(bool3(_134.x == _135.x, _134.y == _135.y, _134.z == _135.z)) && all(bool3(_138.x == _139.x, _138.y == _139.y, _138.z == _139.z))) && all(bool3(_143.x == _144.x, _143.y == _144.y, _143.z == _144.z));
    }
    else
    {
        _148 = false;
    }
    ok = _148;
    bool _196 = false;
    if (ok)
    {
        float4x2 _169 = float4x2(float2(f4.xy), float2(f4.xyz.z, f4.wxyz.x), float2(f4.wxyz.yz), float2(f4.wxyz.w, f4.w));
        float4x2 _175 = float4x2(float2(1.0f, 2.0f), float2(3.0f, 4.0f), float2(1.0f, 2.0f), float2(3.0f, 4.0f));
        float2 _177 = _169[0];
        float2 _178 = _175[0];
        float2 _181 = _169[1];
        float2 _182 = _175[1];
        float2 _186 = _169[2];
        float2 _187 = _175[2];
        float2 _191 = _169[3];
        float2 _192 = _175[3];
        _196 = ((all(bool2(_177.x == _178.x, _177.y == _178.y)) && all(bool2(_181.x == _182.x, _181.y == _182.y))) && all(bool2(_186.x == _187.x, _186.y == _187.y))) && all(bool2(_191.x == _192.x, _191.y == _192.y));
    }
    else
    {
        _196 = false;
    }
    ok = _196;
    bool _245 = false;
    if (ok)
    {
        float4x3 _219 = float4x3(float3(f4.x, f4.yz), float3(f4.yzwx.zw, f4.yzwx.x), float3(f4.yzwx.yzw), f4.yzw);
        float4x3 _225 = float4x3(float3(1.0f, 2.0f, 3.0f), float3(4.0f, 1.0f, 2.0f), float3(3.0f, 4.0f, 1.0f), float3(2.0f, 3.0f, 4.0f));
        float3 _226 = _219[0];
        float3 _227 = _225[0];
        float3 _230 = _219[1];
        float3 _231 = _225[1];
        float3 _235 = _219[2];
        float3 _236 = _225[2];
        float3 _240 = _219[3];
        float3 _241 = _225[3];
        _245 = ((all(bool3(_226.x == _227.x, _226.y == _227.y, _226.z == _227.z)) && all(bool3(_230.x == _231.x, _230.y == _231.y, _230.z == _231.z))) && all(bool3(_235.x == _236.x, _235.y == _236.y, _235.z == _236.z))) && all(bool3(_240.x == _241.x, _240.y == _241.y, _240.z == _241.z));
    }
    else
    {
        _245 = false;
    }
    ok = _245;
    float4 _247 = 0.0f.xxxx;
    if (ok)
    {
        _247 = _10_colorGreen;
    }
    else
    {
        _247 = _10_colorRed;
    }
    return _247;
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
