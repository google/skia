### Compilation failed:

error: SPIR-V validation error: ID 4294967295[%4294967295] has not been defined
  %253 = OpFunctionCall %void %out_half2 %4294967295

OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "colorWhite"
OpName %_entrypoint "_entrypoint"
OpName %out_half "out_half"
OpName %out_half2 "out_half2"
OpName %out_half3 "out_half3"
OpName %out_half4 "out_half4"
OpName %out_half2x2 "out_half2x2"
OpName %out_half3x3 "out_half3x3"
OpName %out_half4x4 "out_half4x4"
OpName %out_int "out_int"
OpName %out_int2 "out_int2"
OpName %out_int3 "out_int3"
OpName %out_int4 "out_int4"
OpName %out_float "out_float"
OpName %out_float2 "out_float2"
OpName %out_float3 "out_float3"
OpName %out_float4 "out_float4"
OpName %out_float2x2 "out_float2x2"
OpName %out_float3x3 "out_float3x3"
OpName %out_float4x4 "out_float4x4"
OpName %out_bool "out_bool"
OpName %out_bool2 "out_bool2"
OpName %out_bool3 "out_bool3"
OpName %out_bool4 "out_bool4"
OpName %main "main"
OpName %result "result"
OpName %h "h"
OpName %h2 "h2"
OpName %h3 "h3"
OpName %h4 "h4"
OpName %h2x2 "h2x2"
OpName %h3x3 "h3x3"
OpName %h4x4 "h4x4"
OpName %i "i"
OpName %i2 "i2"
OpName %i3 "i3"
OpName %i4 "i4"
OpName %f "f"
OpName %f2 "f2"
OpName %f3 "f3"
OpName %f4 "f4"
OpName %f2x2 "f2x2"
OpName %f3x3 "f3x3"
OpName %f4x4 "f4x4"
OpName %b "b"
OpName %b2 "b2"
OpName %b3 "b3"
OpName %b4 "b4"
OpName %ok "ok"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %32 Binding 0
OpDecorate %32 DescriptorSet 0
OpDecorate %48 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %316 RelaxedPrecision
OpDecorate %320 RelaxedPrecision
OpDecorate %321 RelaxedPrecision
OpDecorate %323 RelaxedPrecision
OpDecorate %324 RelaxedPrecision
OpDecorate %326 RelaxedPrecision
OpDecorate %327 RelaxedPrecision
OpDecorate %329 RelaxedPrecision
OpDecorate %331 RelaxedPrecision
OpDecorate %333 RelaxedPrecision
OpDecorate %335 RelaxedPrecision
OpDecorate %337 RelaxedPrecision
OpDecorate %339 RelaxedPrecision
OpDecorate %341 RelaxedPrecision
OpDecorate %344 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %386 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %402 RelaxedPrecision
OpDecorate %406 RelaxedPrecision
OpDecorate %412 RelaxedPrecision
OpDecorate %414 RelaxedPrecision
OpDecorate %415 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%32 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%37 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%40 = OpTypeFunction %void %_ptr_Function_float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%v2float = OpTypeVector %float 2
%_ptr_Function_v2float = OpTypePointer Function %v2float
%51 = OpTypeFunction %void %_ptr_Function_v2float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%60 = OpTypeFunction %void %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%68 = OpTypeFunction %void %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%77 = OpTypeFunction %void %_ptr_Function_mat2v2float
%float_0 = OpConstant %float 0
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%89 = OpTypeFunction %void %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%101 = OpTypeFunction %void %_ptr_Function_mat4v4float
%_ptr_Function_int = OpTypePointer Function %int
%113 = OpTypeFunction %void %_ptr_Function_int
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%122 = OpTypeFunction %void %_ptr_Function_v2int
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%132 = OpTypeFunction %void %_ptr_Function_v3int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%142 = OpTypeFunction %void %_ptr_Function_v4int
%_ptr_Function_bool = OpTypePointer Function %bool
%201 = OpTypeFunction %void %_ptr_Function_bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%210 = OpTypeFunction %void %_ptr_Function_v2bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%220 = OpTypeFunction %void %_ptr_Function_v3bool
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%230 = OpTypeFunction %void %_ptr_Function_v4bool
%239 = OpTypeFunction %v4float
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%int_0 = OpConstant %int 0
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%_entrypoint = OpFunction %void None %37
%38 = OpLabel
%39 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %39
OpReturn
OpFunctionEnd
%out_half = OpFunction %void None %40
%42 = OpFunctionParameter %_ptr_Function_float
%43 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%48 = OpLoad %v4float %44
%49 = OpCompositeExtract %float %48 0
OpStore %42 %49
OpReturn
OpFunctionEnd
%out_half2 = OpFunction %void None %51
%53 = OpFunctionParameter %_ptr_Function_v2float
%54 = OpLabel
%55 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%56 = OpLoad %v4float %55
%57 = OpCompositeExtract %float %56 1
%58 = OpCompositeConstruct %v2float %57 %57
OpStore %53 %58
OpReturn
OpFunctionEnd
%out_half3 = OpFunction %void None %60
%62 = OpFunctionParameter %_ptr_Function_v3float
%63 = OpLabel
%64 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%65 = OpLoad %v4float %64
%66 = OpCompositeExtract %float %65 2
%67 = OpCompositeConstruct %v3float %66 %66 %66
OpStore %62 %67
OpReturn
OpFunctionEnd
%out_half4 = OpFunction %void None %68
%70 = OpFunctionParameter %_ptr_Function_v4float
%71 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%73 = OpLoad %v4float %72
%74 = OpCompositeExtract %float %73 3
%75 = OpCompositeConstruct %v4float %74 %74 %74 %74
OpStore %70 %75
OpReturn
OpFunctionEnd
%out_half2x2 = OpFunction %void None %77
%79 = OpFunctionParameter %_ptr_Function_mat2v2float
%80 = OpLabel
%81 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%82 = OpLoad %v4float %81
%83 = OpCompositeExtract %float %82 0
%86 = OpCompositeConstruct %v2float %83 %float_0
%87 = OpCompositeConstruct %v2float %float_0 %83
%84 = OpCompositeConstruct %mat2v2float %86 %87
OpStore %79 %84
OpReturn
OpFunctionEnd
%out_half3x3 = OpFunction %void None %89
%91 = OpFunctionParameter %_ptr_Function_mat3v3float
%92 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%94 = OpLoad %v4float %93
%95 = OpCompositeExtract %float %94 1
%97 = OpCompositeConstruct %v3float %95 %float_0 %float_0
%98 = OpCompositeConstruct %v3float %float_0 %95 %float_0
%99 = OpCompositeConstruct %v3float %float_0 %float_0 %95
%96 = OpCompositeConstruct %mat3v3float %97 %98 %99
OpStore %91 %96
OpReturn
OpFunctionEnd
%out_half4x4 = OpFunction %void None %101
%103 = OpFunctionParameter %_ptr_Function_mat4v4float
%104 = OpLabel
%105 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%106 = OpLoad %v4float %105
%107 = OpCompositeExtract %float %106 2
%109 = OpCompositeConstruct %v4float %107 %float_0 %float_0 %float_0
%110 = OpCompositeConstruct %v4float %float_0 %107 %float_0 %float_0
%111 = OpCompositeConstruct %v4float %float_0 %float_0 %107 %float_0
%112 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %107
%108 = OpCompositeConstruct %mat4v4float %109 %110 %111 %112
OpStore %103 %108
OpReturn
OpFunctionEnd
%out_int = OpFunction %void None %113
%115 = OpFunctionParameter %_ptr_Function_int
%116 = OpLabel
%117 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%118 = OpLoad %v4float %117
%119 = OpCompositeExtract %float %118 0
%120 = OpConvertFToS %int %119
OpStore %115 %120
OpReturn
OpFunctionEnd
%out_int2 = OpFunction %void None %122
%124 = OpFunctionParameter %_ptr_Function_v2int
%125 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%127 = OpLoad %v4float %126
%128 = OpCompositeExtract %float %127 1
%129 = OpConvertFToS %int %128
%130 = OpCompositeConstruct %v2int %129 %129
OpStore %124 %130
OpReturn
OpFunctionEnd
%out_int3 = OpFunction %void None %132
%134 = OpFunctionParameter %_ptr_Function_v3int
%135 = OpLabel
%136 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%137 = OpLoad %v4float %136
%138 = OpCompositeExtract %float %137 2
%139 = OpConvertFToS %int %138
%140 = OpCompositeConstruct %v3int %139 %139 %139
OpStore %134 %140
OpReturn
OpFunctionEnd
%out_int4 = OpFunction %void None %142
%144 = OpFunctionParameter %_ptr_Function_v4int
%145 = OpLabel
%146 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%147 = OpLoad %v4float %146
%148 = OpCompositeExtract %float %147 3
%149 = OpConvertFToS %int %148
%150 = OpCompositeConstruct %v4int %149 %149 %149 %149
OpStore %144 %150
OpReturn
OpFunctionEnd
%out_float = OpFunction %void None %40
%151 = OpFunctionParameter %_ptr_Function_float
%152 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%154 = OpLoad %v4float %153
%155 = OpCompositeExtract %float %154 0
OpStore %151 %155
OpReturn
OpFunctionEnd
%out_float2 = OpFunction %void None %51
%156 = OpFunctionParameter %_ptr_Function_v2float
%157 = OpLabel
%158 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%159 = OpLoad %v4float %158
%160 = OpCompositeExtract %float %159 1
%161 = OpCompositeConstruct %v2float %160 %160
OpStore %156 %161
OpReturn
OpFunctionEnd
%out_float3 = OpFunction %void None %60
%162 = OpFunctionParameter %_ptr_Function_v3float
%163 = OpLabel
%164 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%165 = OpLoad %v4float %164
%166 = OpCompositeExtract %float %165 2
%167 = OpCompositeConstruct %v3float %166 %166 %166
OpStore %162 %167
OpReturn
OpFunctionEnd
%out_float4 = OpFunction %void None %68
%168 = OpFunctionParameter %_ptr_Function_v4float
%169 = OpLabel
%170 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%171 = OpLoad %v4float %170
%172 = OpCompositeExtract %float %171 3
%173 = OpCompositeConstruct %v4float %172 %172 %172 %172
OpStore %168 %173
OpReturn
OpFunctionEnd
%out_float2x2 = OpFunction %void None %77
%174 = OpFunctionParameter %_ptr_Function_mat2v2float
%175 = OpLabel
%176 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%177 = OpLoad %v4float %176
%178 = OpCompositeExtract %float %177 0
%180 = OpCompositeConstruct %v2float %178 %float_0
%181 = OpCompositeConstruct %v2float %float_0 %178
%179 = OpCompositeConstruct %mat2v2float %180 %181
OpStore %174 %179
OpReturn
OpFunctionEnd
%out_float3x3 = OpFunction %void None %89
%182 = OpFunctionParameter %_ptr_Function_mat3v3float
%183 = OpLabel
%184 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%185 = OpLoad %v4float %184
%186 = OpCompositeExtract %float %185 1
%188 = OpCompositeConstruct %v3float %186 %float_0 %float_0
%189 = OpCompositeConstruct %v3float %float_0 %186 %float_0
%190 = OpCompositeConstruct %v3float %float_0 %float_0 %186
%187 = OpCompositeConstruct %mat3v3float %188 %189 %190
OpStore %182 %187
OpReturn
OpFunctionEnd
%out_float4x4 = OpFunction %void None %101
%191 = OpFunctionParameter %_ptr_Function_mat4v4float
%192 = OpLabel
%193 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%194 = OpLoad %v4float %193
%195 = OpCompositeExtract %float %194 2
%197 = OpCompositeConstruct %v4float %195 %float_0 %float_0 %float_0
%198 = OpCompositeConstruct %v4float %float_0 %195 %float_0 %float_0
%199 = OpCompositeConstruct %v4float %float_0 %float_0 %195 %float_0
%200 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %195
%196 = OpCompositeConstruct %mat4v4float %197 %198 %199 %200
OpStore %191 %196
OpReturn
OpFunctionEnd
%out_bool = OpFunction %void None %201
%203 = OpFunctionParameter %_ptr_Function_bool
%204 = OpLabel
%205 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%206 = OpLoad %v4float %205
%207 = OpCompositeExtract %float %206 0
%208 = OpFUnordNotEqual %bool %207 %float_0
OpStore %203 %208
OpReturn
OpFunctionEnd
%out_bool2 = OpFunction %void None %210
%212 = OpFunctionParameter %_ptr_Function_v2bool
%213 = OpLabel
%214 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%215 = OpLoad %v4float %214
%216 = OpCompositeExtract %float %215 1
%217 = OpFUnordNotEqual %bool %216 %float_0
%218 = OpCompositeConstruct %v2bool %217 %217
OpStore %212 %218
OpReturn
OpFunctionEnd
%out_bool3 = OpFunction %void None %220
%222 = OpFunctionParameter %_ptr_Function_v3bool
%223 = OpLabel
%224 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%225 = OpLoad %v4float %224
%226 = OpCompositeExtract %float %225 2
%227 = OpFUnordNotEqual %bool %226 %float_0
%228 = OpCompositeConstruct %v3bool %227 %227 %227
OpStore %222 %228
OpReturn
OpFunctionEnd
%out_bool4 = OpFunction %void None %230
%232 = OpFunctionParameter %_ptr_Function_v4bool
%233 = OpLabel
%234 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%235 = OpLoad %v4float %234
%236 = OpCompositeExtract %float %235 3
%237 = OpFUnordNotEqual %bool %236 %float_0
%238 = OpCompositeConstruct %v4bool %237 %237 %237 %237
OpStore %232 %238
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %239
%240 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%h = OpVariable %_ptr_Function_float Function
%h2 = OpVariable %_ptr_Function_v2float Function
%h3 = OpVariable %_ptr_Function_v3float Function
%h4 = OpVariable %_ptr_Function_v4float Function
%h2x2 = OpVariable %_ptr_Function_mat2v2float Function
%h3x3 = OpVariable %_ptr_Function_mat3v3float Function
%h4x4 = OpVariable %_ptr_Function_mat4v4float Function
%i = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_v2int Function
%i3 = OpVariable %_ptr_Function_v3int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%f = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_v2float Function
%f3 = OpVariable %_ptr_Function_v3float Function
%f4 = OpVariable %_ptr_Function_v4float Function
%f2x2 = OpVariable %_ptr_Function_mat2v2float Function
%f3x3 = OpVariable %_ptr_Function_mat3v3float Function
%f4x4 = OpVariable %_ptr_Function_mat4v4float Function
%b = OpVariable %_ptr_Function_bool Function
%b2 = OpVariable %_ptr_Function_v2bool Function
%b3 = OpVariable %_ptr_Function_v3bool Function
%b4 = OpVariable %_ptr_Function_v4bool Function
%ok = OpVariable %_ptr_Function_bool Function
%407 = OpVariable %_ptr_Function_v4float Function
%243 = OpFunctionCall %void %out_half %h
%245 = OpFunctionCall %void %out_half2 %h2
%247 = OpFunctionCall %void %out_half3 %h3
%249 = OpFunctionCall %void %out_half4 %h4
%250 = OpAccessChain %_ptr_Function_float %h3 %int_1
%252 = OpFunctionCall %void %out_half %250
%253 = OpFunctionCall %void %out_half2 %4294967295
%254 = OpFunctionCall %void %out_half4 %4294967295
%256 = OpFunctionCall %void %out_half2x2 %h2x2
%258 = OpFunctionCall %void %out_half3x3 %h3x3
%260 = OpFunctionCall %void %out_half4x4 %h4x4
%261 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
%262 = OpFunctionCall %void %out_half3 %261
%264 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
%265 = OpAccessChain %_ptr_Function_float %264 %int_3
%266 = OpFunctionCall %void %out_half %265
%268 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%269 = OpAccessChain %_ptr_Function_float %268 %int_0
%270 = OpFunctionCall %void %out_half %269
%272 = OpFunctionCall %void %out_int %i
%274 = OpFunctionCall %void %out_int2 %i2
%276 = OpFunctionCall %void %out_int3 %i3
%278 = OpFunctionCall %void %out_int4 %i4
%279 = OpFunctionCall %void %out_int3 %4294967295
%280 = OpAccessChain %_ptr_Function_int %i2 %int_1
%281 = OpFunctionCall %void %out_int %280
%283 = OpFunctionCall %void %out_float %f
%285 = OpFunctionCall %void %out_float2 %f2
%287 = OpFunctionCall %void %out_float3 %f3
%289 = OpFunctionCall %void %out_float4 %f4
%290 = OpFunctionCall %void %out_float2 %4294967295
%291 = OpAccessChain %_ptr_Function_float %f2 %int_0
%292 = OpFunctionCall %void %out_float %291
%294 = OpFunctionCall %void %out_float2x2 %f2x2
%296 = OpFunctionCall %void %out_float3x3 %f3x3
%298 = OpFunctionCall %void %out_float4x4 %f4x4
%299 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%300 = OpAccessChain %_ptr_Function_float %299 %int_0
%301 = OpFunctionCall %void %out_float %300
%303 = OpFunctionCall %void %out_bool %b
%305 = OpFunctionCall %void %out_bool2 %b2
%307 = OpFunctionCall %void %out_bool3 %b3
%309 = OpFunctionCall %void %out_bool4 %b4
%310 = OpFunctionCall %void %out_bool2 %4294967295
%311 = OpAccessChain %_ptr_Function_bool %b3 %int_2
%312 = OpFunctionCall %void %out_bool %311
OpStore %ok %true
%316 = OpLoad %bool %ok
OpSelectionMerge %318 None
OpBranchConditional %316 %317 %318
%317 = OpLabel
%320 = OpLoad %float %h
%321 = OpLoad %v2float %h2
%322 = OpCompositeExtract %float %321 0
%323 = OpFMul %float %320 %322
%324 = OpLoad %v3float %h3
%325 = OpCompositeExtract %float %324 0
%326 = OpFMul %float %323 %325
%327 = OpLoad %v4float %h4
%328 = OpCompositeExtract %float %327 0
%329 = OpFMul %float %326 %328
%330 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%331 = OpLoad %v2float %330
%332 = OpCompositeExtract %float %331 0
%333 = OpFMul %float %329 %332
%334 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
%335 = OpLoad %v3float %334
%336 = OpCompositeExtract %float %335 0
%337 = OpFMul %float %333 %336
%338 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
%339 = OpLoad %v4float %338
%340 = OpCompositeExtract %float %339 0
%341 = OpFMul %float %337 %340
%342 = OpFOrdEqual %bool %float_1 %341
OpBranch %318
%318 = OpLabel
%343 = OpPhi %bool %false %240 %342 %317
OpStore %ok %343
%344 = OpLoad %bool %ok
OpSelectionMerge %346 None
OpBranchConditional %344 %345 %346
%345 = OpLabel
%347 = OpLoad %float %f
%348 = OpLoad %v2float %f2
%349 = OpCompositeExtract %float %348 0
%350 = OpFMul %float %347 %349
%351 = OpLoad %v3float %f3
%352 = OpCompositeExtract %float %351 0
%353 = OpFMul %float %350 %352
%354 = OpLoad %v4float %f4
%355 = OpCompositeExtract %float %354 0
%356 = OpFMul %float %353 %355
%357 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%358 = OpLoad %v2float %357
%359 = OpCompositeExtract %float %358 0
%360 = OpFMul %float %356 %359
%361 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
%362 = OpLoad %v3float %361
%363 = OpCompositeExtract %float %362 0
%364 = OpFMul %float %360 %363
%365 = OpAccessChain %_ptr_Function_v4float %f4x4 %int_0
%366 = OpLoad %v4float %365
%367 = OpCompositeExtract %float %366 0
%368 = OpFMul %float %364 %367
%369 = OpFOrdEqual %bool %float_1 %368
OpBranch %346
%346 = OpLabel
%370 = OpPhi %bool %false %318 %369 %345
OpStore %ok %370
%371 = OpLoad %bool %ok
OpSelectionMerge %373 None
OpBranchConditional %371 %372 %373
%372 = OpLabel
%374 = OpLoad %int %i
%375 = OpLoad %v2int %i2
%376 = OpCompositeExtract %int %375 0
%377 = OpIMul %int %374 %376
%378 = OpLoad %v3int %i3
%379 = OpCompositeExtract %int %378 0
%380 = OpIMul %int %377 %379
%381 = OpLoad %v4int %i4
%382 = OpCompositeExtract %int %381 0
%383 = OpIMul %int %380 %382
%384 = OpIEqual %bool %int_1 %383
OpBranch %373
%373 = OpLabel
%385 = OpPhi %bool %false %346 %384 %372
OpStore %ok %385
%386 = OpLoad %bool %ok
OpSelectionMerge %388 None
OpBranchConditional %386 %387 %388
%387 = OpLabel
%389 = OpLoad %bool %b
OpSelectionMerge %391 None
OpBranchConditional %389 %390 %391
%390 = OpLabel
%392 = OpLoad %v2bool %b2
%393 = OpCompositeExtract %bool %392 0
OpBranch %391
%391 = OpLabel
%394 = OpPhi %bool %false %387 %393 %390
OpSelectionMerge %396 None
OpBranchConditional %394 %395 %396
%395 = OpLabel
%397 = OpLoad %v3bool %b3
%398 = OpCompositeExtract %bool %397 0
OpBranch %396
%396 = OpLabel
%399 = OpPhi %bool %false %391 %398 %395
OpSelectionMerge %401 None
OpBranchConditional %399 %400 %401
%400 = OpLabel
%402 = OpLoad %v4bool %b4
%403 = OpCompositeExtract %bool %402 0
OpBranch %401
%401 = OpLabel
%404 = OpPhi %bool %false %396 %403 %400
OpBranch %388
%388 = OpLabel
%405 = OpPhi %bool %false %373 %404 %401
OpStore %ok %405
%406 = OpLoad %bool %ok
OpSelectionMerge %410 None
OpBranchConditional %406 %408 %409
%408 = OpLabel
%411 = OpAccessChain %_ptr_Uniform_v4float %32 %int_0
%412 = OpLoad %v4float %411
OpStore %407 %412
OpBranch %410
%409 = OpLabel
%413 = OpAccessChain %_ptr_Uniform_v4float %32 %int_1
%414 = OpLoad %v4float %413
OpStore %407 %414
OpBranch %410
%410 = OpLabel
%415 = OpLoad %v4float %407
OpReturnValue %415
OpFunctionEnd

1 error
