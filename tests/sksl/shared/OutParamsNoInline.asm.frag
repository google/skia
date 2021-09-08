OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "colorWhite"
OpName %_entrypoint_v "_entrypoint_v"
OpName %out_half_vh "out_half_vh"
OpName %out_half2_vh2 "out_half2_vh2"
OpName %out_half3_vh3 "out_half3_vh3"
OpName %out_half4_vh4 "out_half4_vh4"
OpName %out_half2x2_vh22 "out_half2x2_vh22"
OpName %out_half3x3_vh33 "out_half3x3_vh33"
OpName %out_half4x4_vh44 "out_half4x4_vh44"
OpName %out_int_vi "out_int_vi"
OpName %out_int2_vi2 "out_int2_vi2"
OpName %out_int3_vi3 "out_int3_vi3"
OpName %out_int4_vi4 "out_int4_vi4"
OpName %out_float_vf "out_float_vf"
OpName %out_float2_vf2 "out_float2_vf2"
OpName %out_float3_vf3 "out_float3_vf3"
OpName %out_float4_vf4 "out_float4_vf4"
OpName %out_float2x2_vf22 "out_float2x2_vf22"
OpName %out_float3x3_vf33 "out_float3x3_vf33"
OpName %out_float4x4_vf44 "out_float4x4_vf44"
OpName %out_bool_vb "out_bool_vb"
OpName %out_bool2_vb2 "out_bool2_vb2"
OpName %out_bool3_vb3 "out_bool3_vb3"
OpName %out_bool4_vb4 "out_bool4_vb4"
OpName %main "main"
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
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %161 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %173 RelaxedPrecision
OpDecorate %174 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %188 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %h RelaxedPrecision
OpDecorate %h2 RelaxedPrecision
OpDecorate %h3 RelaxedPrecision
OpDecorate %h4 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %256 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %260 RelaxedPrecision
OpDecorate %261 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %h2x2 RelaxedPrecision
OpDecorate %h3x3 RelaxedPrecision
OpDecorate %h4x4 RelaxedPrecision
OpDecorate %274 RelaxedPrecision
OpDecorate %276 RelaxedPrecision
OpDecorate %280 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %288 RelaxedPrecision
OpDecorate %354 RelaxedPrecision
OpDecorate %358 RelaxedPrecision
OpDecorate %359 RelaxedPrecision
OpDecorate %360 RelaxedPrecision
OpDecorate %361 RelaxedPrecision
OpDecorate %362 RelaxedPrecision
OpDecorate %363 RelaxedPrecision
OpDecorate %364 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %366 RelaxedPrecision
OpDecorate %367 RelaxedPrecision
OpDecorate %369 RelaxedPrecision
OpDecorate %370 RelaxedPrecision
OpDecorate %371 RelaxedPrecision
OpDecorate %373 RelaxedPrecision
OpDecorate %374 RelaxedPrecision
OpDecorate %375 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %409 RelaxedPrecision
OpDecorate %424 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %430 RelaxedPrecision
OpDecorate %435 RelaxedPrecision
OpDecorate %440 RelaxedPrecision
OpDecorate %444 RelaxedPrecision
OpDecorate %450 RelaxedPrecision
OpDecorate %452 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%41 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
%45 = OpTypeFunction %void %_ptr_Function_float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%55 = OpTypeFunction %void %_ptr_Function_v2float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%63 = OpTypeFunction %void %_ptr_Function_v3float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%71 = OpTypeFunction %void %_ptr_Function_v4float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%80 = OpTypeFunction %void %_ptr_Function_mat2v2float
%mat3v3float = OpTypeMatrix %v3float 3
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%91 = OpTypeFunction %void %_ptr_Function_mat3v3float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%103 = OpTypeFunction %void %_ptr_Function_mat4v4float
%_ptr_Function_int = OpTypePointer Function %int
%115 = OpTypeFunction %void %_ptr_Function_int
%v2int = OpTypeVector %int 2
%_ptr_Function_v2int = OpTypePointer Function %v2int
%124 = OpTypeFunction %void %_ptr_Function_v2int
%v3int = OpTypeVector %int 3
%_ptr_Function_v3int = OpTypePointer Function %v3int
%134 = OpTypeFunction %void %_ptr_Function_v3int
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%144 = OpTypeFunction %void %_ptr_Function_v4int
%_ptr_Function_bool = OpTypePointer Function %bool
%203 = OpTypeFunction %void %_ptr_Function_bool
%v2bool = OpTypeVector %bool 2
%_ptr_Function_v2bool = OpTypePointer Function %v2bool
%212 = OpTypeFunction %void %_ptr_Function_v2bool
%v3bool = OpTypeVector %bool 3
%_ptr_Function_v3bool = OpTypePointer Function %v3bool
%222 = OpTypeFunction %void %_ptr_Function_v3bool
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%232 = OpTypeFunction %void %_ptr_Function_v4bool
%241 = OpTypeFunction %v4float %_ptr_Function_v2float
%int_1 = OpConstant %int 1
%int_3 = OpConstant %int 3
%int_0 = OpConstant %int 0
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%_entrypoint_v = OpFunction %void None %37
%38 = OpLabel
%42 = OpVariable %_ptr_Function_v2float Function
OpStore %42 %41
%44 = OpFunctionCall %v4float %main %42
OpStore %sk_FragColor %44
OpReturn
OpFunctionEnd
%out_half_vh = OpFunction %void None %45
%47 = OpFunctionParameter %_ptr_Function_float
%48 = OpLabel
%49 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%53 = OpLoad %v4float %49
%54 = OpCompositeExtract %float %53 0
OpStore %47 %54
OpReturn
OpFunctionEnd
%out_half2_vh2 = OpFunction %void None %55
%56 = OpFunctionParameter %_ptr_Function_v2float
%57 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%59 = OpLoad %v4float %58
%60 = OpCompositeExtract %float %59 1
%61 = OpCompositeConstruct %v2float %60 %60
OpStore %56 %61
OpReturn
OpFunctionEnd
%out_half3_vh3 = OpFunction %void None %63
%65 = OpFunctionParameter %_ptr_Function_v3float
%66 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%68 = OpLoad %v4float %67
%69 = OpCompositeExtract %float %68 2
%70 = OpCompositeConstruct %v3float %69 %69 %69
OpStore %65 %70
OpReturn
OpFunctionEnd
%out_half4_vh4 = OpFunction %void None %71
%73 = OpFunctionParameter %_ptr_Function_v4float
%74 = OpLabel
%75 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%76 = OpLoad %v4float %75
%77 = OpCompositeExtract %float %76 3
%78 = OpCompositeConstruct %v4float %77 %77 %77 %77
OpStore %73 %78
OpReturn
OpFunctionEnd
%out_half2x2_vh22 = OpFunction %void None %80
%82 = OpFunctionParameter %_ptr_Function_mat2v2float
%83 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%85 = OpLoad %v4float %84
%86 = OpCompositeExtract %float %85 0
%88 = OpCompositeConstruct %v2float %86 %float_0
%89 = OpCompositeConstruct %v2float %float_0 %86
%87 = OpCompositeConstruct %mat2v2float %88 %89
OpStore %82 %87
OpReturn
OpFunctionEnd
%out_half3x3_vh33 = OpFunction %void None %91
%93 = OpFunctionParameter %_ptr_Function_mat3v3float
%94 = OpLabel
%95 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%96 = OpLoad %v4float %95
%97 = OpCompositeExtract %float %96 1
%99 = OpCompositeConstruct %v3float %97 %float_0 %float_0
%100 = OpCompositeConstruct %v3float %float_0 %97 %float_0
%101 = OpCompositeConstruct %v3float %float_0 %float_0 %97
%98 = OpCompositeConstruct %mat3v3float %99 %100 %101
OpStore %93 %98
OpReturn
OpFunctionEnd
%out_half4x4_vh44 = OpFunction %void None %103
%105 = OpFunctionParameter %_ptr_Function_mat4v4float
%106 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%108 = OpLoad %v4float %107
%109 = OpCompositeExtract %float %108 2
%111 = OpCompositeConstruct %v4float %109 %float_0 %float_0 %float_0
%112 = OpCompositeConstruct %v4float %float_0 %109 %float_0 %float_0
%113 = OpCompositeConstruct %v4float %float_0 %float_0 %109 %float_0
%114 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %109
%110 = OpCompositeConstruct %mat4v4float %111 %112 %113 %114
OpStore %105 %110
OpReturn
OpFunctionEnd
%out_int_vi = OpFunction %void None %115
%117 = OpFunctionParameter %_ptr_Function_int
%118 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%120 = OpLoad %v4float %119
%121 = OpCompositeExtract %float %120 0
%122 = OpConvertFToS %int %121
OpStore %117 %122
OpReturn
OpFunctionEnd
%out_int2_vi2 = OpFunction %void None %124
%126 = OpFunctionParameter %_ptr_Function_v2int
%127 = OpLabel
%128 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%129 = OpLoad %v4float %128
%130 = OpCompositeExtract %float %129 1
%131 = OpConvertFToS %int %130
%132 = OpCompositeConstruct %v2int %131 %131
OpStore %126 %132
OpReturn
OpFunctionEnd
%out_int3_vi3 = OpFunction %void None %134
%136 = OpFunctionParameter %_ptr_Function_v3int
%137 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%139 = OpLoad %v4float %138
%140 = OpCompositeExtract %float %139 2
%141 = OpConvertFToS %int %140
%142 = OpCompositeConstruct %v3int %141 %141 %141
OpStore %136 %142
OpReturn
OpFunctionEnd
%out_int4_vi4 = OpFunction %void None %144
%146 = OpFunctionParameter %_ptr_Function_v4int
%147 = OpLabel
%148 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%149 = OpLoad %v4float %148
%150 = OpCompositeExtract %float %149 3
%151 = OpConvertFToS %int %150
%152 = OpCompositeConstruct %v4int %151 %151 %151 %151
OpStore %146 %152
OpReturn
OpFunctionEnd
%out_float_vf = OpFunction %void None %45
%153 = OpFunctionParameter %_ptr_Function_float
%154 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%156 = OpLoad %v4float %155
%157 = OpCompositeExtract %float %156 0
OpStore %153 %157
OpReturn
OpFunctionEnd
%out_float2_vf2 = OpFunction %void None %55
%158 = OpFunctionParameter %_ptr_Function_v2float
%159 = OpLabel
%160 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%161 = OpLoad %v4float %160
%162 = OpCompositeExtract %float %161 1
%163 = OpCompositeConstruct %v2float %162 %162
OpStore %158 %163
OpReturn
OpFunctionEnd
%out_float3_vf3 = OpFunction %void None %63
%164 = OpFunctionParameter %_ptr_Function_v3float
%165 = OpLabel
%166 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%167 = OpLoad %v4float %166
%168 = OpCompositeExtract %float %167 2
%169 = OpCompositeConstruct %v3float %168 %168 %168
OpStore %164 %169
OpReturn
OpFunctionEnd
%out_float4_vf4 = OpFunction %void None %71
%170 = OpFunctionParameter %_ptr_Function_v4float
%171 = OpLabel
%172 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%173 = OpLoad %v4float %172
%174 = OpCompositeExtract %float %173 3
%175 = OpCompositeConstruct %v4float %174 %174 %174 %174
OpStore %170 %175
OpReturn
OpFunctionEnd
%out_float2x2_vf22 = OpFunction %void None %80
%176 = OpFunctionParameter %_ptr_Function_mat2v2float
%177 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%179 = OpLoad %v4float %178
%180 = OpCompositeExtract %float %179 0
%182 = OpCompositeConstruct %v2float %180 %float_0
%183 = OpCompositeConstruct %v2float %float_0 %180
%181 = OpCompositeConstruct %mat2v2float %182 %183
OpStore %176 %181
OpReturn
OpFunctionEnd
%out_float3x3_vf33 = OpFunction %void None %91
%184 = OpFunctionParameter %_ptr_Function_mat3v3float
%185 = OpLabel
%186 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%187 = OpLoad %v4float %186
%188 = OpCompositeExtract %float %187 1
%190 = OpCompositeConstruct %v3float %188 %float_0 %float_0
%191 = OpCompositeConstruct %v3float %float_0 %188 %float_0
%192 = OpCompositeConstruct %v3float %float_0 %float_0 %188
%189 = OpCompositeConstruct %mat3v3float %190 %191 %192
OpStore %184 %189
OpReturn
OpFunctionEnd
%out_float4x4_vf44 = OpFunction %void None %103
%193 = OpFunctionParameter %_ptr_Function_mat4v4float
%194 = OpLabel
%195 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%196 = OpLoad %v4float %195
%197 = OpCompositeExtract %float %196 2
%199 = OpCompositeConstruct %v4float %197 %float_0 %float_0 %float_0
%200 = OpCompositeConstruct %v4float %float_0 %197 %float_0 %float_0
%201 = OpCompositeConstruct %v4float %float_0 %float_0 %197 %float_0
%202 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %197
%198 = OpCompositeConstruct %mat4v4float %199 %200 %201 %202
OpStore %193 %198
OpReturn
OpFunctionEnd
%out_bool_vb = OpFunction %void None %203
%205 = OpFunctionParameter %_ptr_Function_bool
%206 = OpLabel
%207 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%208 = OpLoad %v4float %207
%209 = OpCompositeExtract %float %208 0
%210 = OpFUnordNotEqual %bool %209 %float_0
OpStore %205 %210
OpReturn
OpFunctionEnd
%out_bool2_vb2 = OpFunction %void None %212
%214 = OpFunctionParameter %_ptr_Function_v2bool
%215 = OpLabel
%216 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%217 = OpLoad %v4float %216
%218 = OpCompositeExtract %float %217 1
%219 = OpFUnordNotEqual %bool %218 %float_0
%220 = OpCompositeConstruct %v2bool %219 %219
OpStore %214 %220
OpReturn
OpFunctionEnd
%out_bool3_vb3 = OpFunction %void None %222
%224 = OpFunctionParameter %_ptr_Function_v3bool
%225 = OpLabel
%226 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%227 = OpLoad %v4float %226
%228 = OpCompositeExtract %float %227 2
%229 = OpFUnordNotEqual %bool %228 %float_0
%230 = OpCompositeConstruct %v3bool %229 %229 %229
OpStore %224 %230
OpReturn
OpFunctionEnd
%out_bool4_vb4 = OpFunction %void None %232
%234 = OpFunctionParameter %_ptr_Function_v4bool
%235 = OpLabel
%236 = OpAccessChain %_ptr_Uniform_v4float %32 %int_2
%237 = OpLoad %v4float %236
%238 = OpCompositeExtract %float %237 3
%239 = OpFUnordNotEqual %bool %238 %float_0
%240 = OpCompositeConstruct %v4bool %239 %239 %239 %239
OpStore %234 %240
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %241
%242 = OpFunctionParameter %_ptr_Function_v2float
%243 = OpLabel
%h = OpVariable %_ptr_Function_float Function
%h2 = OpVariable %_ptr_Function_v2float Function
%h3 = OpVariable %_ptr_Function_v3float Function
%h4 = OpVariable %_ptr_Function_v4float Function
%254 = OpVariable %_ptr_Function_float Function
%257 = OpVariable %_ptr_Function_v2float Function
%262 = OpVariable %_ptr_Function_v4float Function
%h2x2 = OpVariable %_ptr_Function_mat2v2float Function
%h3x3 = OpVariable %_ptr_Function_mat3v3float Function
%h4x4 = OpVariable %_ptr_Function_mat4v4float Function
%274 = OpVariable %_ptr_Function_v3float Function
%280 = OpVariable %_ptr_Function_float Function
%286 = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_v2int Function
%i3 = OpVariable %_ptr_Function_v3int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%297 = OpVariable %_ptr_Function_v3int Function
%303 = OpVariable %_ptr_Function_int Function
%f = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_v2float Function
%f3 = OpVariable %_ptr_Function_v3float Function
%f4 = OpVariable %_ptr_Function_v4float Function
%314 = OpVariable %_ptr_Function_v2float Function
%320 = OpVariable %_ptr_Function_float Function
%f2x2 = OpVariable %_ptr_Function_mat2v2float Function
%f3x3 = OpVariable %_ptr_Function_mat3v3float Function
%f4x4 = OpVariable %_ptr_Function_mat4v4float Function
%331 = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_bool Function
%b2 = OpVariable %_ptr_Function_v2bool Function
%b3 = OpVariable %_ptr_Function_v3bool Function
%b4 = OpVariable %_ptr_Function_v4bool Function
%342 = OpVariable %_ptr_Function_v2bool Function
%348 = OpVariable %_ptr_Function_bool Function
%ok = OpVariable %_ptr_Function_bool Function
%445 = OpVariable %_ptr_Function_v4float Function
%245 = OpFunctionCall %void %out_half_vh %h
%247 = OpFunctionCall %void %out_half2_vh2 %h2
%249 = OpFunctionCall %void %out_half3_vh3 %h3
%251 = OpFunctionCall %void %out_half4_vh4 %h4
%252 = OpAccessChain %_ptr_Function_float %h3 %int_1
%255 = OpFunctionCall %void %out_half_vh %254
%256 = OpLoad %float %254
OpStore %252 %256
%258 = OpFunctionCall %void %out_half2_vh2 %257
%259 = OpLoad %v2float %257
%260 = OpLoad %v3float %h3
%261 = OpVectorShuffle %v3float %260 %259 3 1 4
OpStore %h3 %261
%263 = OpFunctionCall %void %out_half4_vh4 %262
%264 = OpLoad %v4float %262
%265 = OpLoad %v4float %h4
%266 = OpVectorShuffle %v4float %265 %264 6 7 4 5
OpStore %h4 %266
%268 = OpFunctionCall %void %out_half2x2_vh22 %h2x2
%270 = OpFunctionCall %void %out_half3x3_vh33 %h3x3
%272 = OpFunctionCall %void %out_half4x4_vh44 %h4x4
%273 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
%275 = OpFunctionCall %void %out_half3_vh3 %274
%276 = OpLoad %v3float %274
OpStore %273 %276
%278 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
%279 = OpAccessChain %_ptr_Function_float %278 %int_3
%281 = OpFunctionCall %void %out_half_vh %280
%282 = OpLoad %float %280
OpStore %279 %282
%284 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%285 = OpAccessChain %_ptr_Function_float %284 %int_0
%287 = OpFunctionCall %void %out_half_vh %286
%288 = OpLoad %float %286
OpStore %285 %288
%290 = OpFunctionCall %void %out_int_vi %i
%292 = OpFunctionCall %void %out_int2_vi2 %i2
%294 = OpFunctionCall %void %out_int3_vi3 %i3
%296 = OpFunctionCall %void %out_int4_vi4 %i4
%298 = OpFunctionCall %void %out_int3_vi3 %297
%299 = OpLoad %v3int %297
%300 = OpLoad %v4int %i4
%301 = OpVectorShuffle %v4int %300 %299 4 5 6 3
OpStore %i4 %301
%302 = OpAccessChain %_ptr_Function_int %i2 %int_1
%304 = OpFunctionCall %void %out_int_vi %303
%305 = OpLoad %int %303
OpStore %302 %305
%307 = OpFunctionCall %void %out_float_vf %f
%309 = OpFunctionCall %void %out_float2_vf2 %f2
%311 = OpFunctionCall %void %out_float3_vf3 %f3
%313 = OpFunctionCall %void %out_float4_vf4 %f4
%315 = OpFunctionCall %void %out_float2_vf2 %314
%316 = OpLoad %v2float %314
%317 = OpLoad %v3float %f3
%318 = OpVectorShuffle %v3float %317 %316 3 4 2
OpStore %f3 %318
%319 = OpAccessChain %_ptr_Function_float %f2 %int_0
%321 = OpFunctionCall %void %out_float_vf %320
%322 = OpLoad %float %320
OpStore %319 %322
%324 = OpFunctionCall %void %out_float2x2_vf22 %f2x2
%326 = OpFunctionCall %void %out_float3x3_vf33 %f3x3
%328 = OpFunctionCall %void %out_float4x4_vf44 %f4x4
%329 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%330 = OpAccessChain %_ptr_Function_float %329 %int_0
%332 = OpFunctionCall %void %out_float_vf %331
%333 = OpLoad %float %331
OpStore %330 %333
%335 = OpFunctionCall %void %out_bool_vb %b
%337 = OpFunctionCall %void %out_bool2_vb2 %b2
%339 = OpFunctionCall %void %out_bool3_vb3 %b3
%341 = OpFunctionCall %void %out_bool4_vb4 %b4
%343 = OpFunctionCall %void %out_bool2_vb2 %342
%344 = OpLoad %v2bool %342
%345 = OpLoad %v4bool %b4
%346 = OpVectorShuffle %v4bool %345 %344 4 1 2 5
OpStore %b4 %346
%347 = OpAccessChain %_ptr_Function_bool %b3 %int_2
%349 = OpFunctionCall %void %out_bool_vb %348
%350 = OpLoad %bool %348
OpStore %347 %350
OpStore %ok %true
%354 = OpLoad %bool %ok
OpSelectionMerge %356 None
OpBranchConditional %354 %355 %356
%355 = OpLabel
%358 = OpLoad %float %h
%359 = OpLoad %v2float %h2
%360 = OpCompositeExtract %float %359 0
%361 = OpFMul %float %358 %360
%362 = OpLoad %v3float %h3
%363 = OpCompositeExtract %float %362 0
%364 = OpFMul %float %361 %363
%365 = OpLoad %v4float %h4
%366 = OpCompositeExtract %float %365 0
%367 = OpFMul %float %364 %366
%368 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%369 = OpLoad %v2float %368
%370 = OpCompositeExtract %float %369 0
%371 = OpFMul %float %367 %370
%372 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
%373 = OpLoad %v3float %372
%374 = OpCompositeExtract %float %373 0
%375 = OpFMul %float %371 %374
%376 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
%377 = OpLoad %v4float %376
%378 = OpCompositeExtract %float %377 0
%379 = OpFMul %float %375 %378
%380 = OpFOrdEqual %bool %float_1 %379
OpBranch %356
%356 = OpLabel
%381 = OpPhi %bool %false %243 %380 %355
OpStore %ok %381
%382 = OpLoad %bool %ok
OpSelectionMerge %384 None
OpBranchConditional %382 %383 %384
%383 = OpLabel
%385 = OpLoad %float %f
%386 = OpLoad %v2float %f2
%387 = OpCompositeExtract %float %386 0
%388 = OpFMul %float %385 %387
%389 = OpLoad %v3float %f3
%390 = OpCompositeExtract %float %389 0
%391 = OpFMul %float %388 %390
%392 = OpLoad %v4float %f4
%393 = OpCompositeExtract %float %392 0
%394 = OpFMul %float %391 %393
%395 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%396 = OpLoad %v2float %395
%397 = OpCompositeExtract %float %396 0
%398 = OpFMul %float %394 %397
%399 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
%400 = OpLoad %v3float %399
%401 = OpCompositeExtract %float %400 0
%402 = OpFMul %float %398 %401
%403 = OpAccessChain %_ptr_Function_v4float %f4x4 %int_0
%404 = OpLoad %v4float %403
%405 = OpCompositeExtract %float %404 0
%406 = OpFMul %float %402 %405
%407 = OpFOrdEqual %bool %float_1 %406
OpBranch %384
%384 = OpLabel
%408 = OpPhi %bool %false %356 %407 %383
OpStore %ok %408
%409 = OpLoad %bool %ok
OpSelectionMerge %411 None
OpBranchConditional %409 %410 %411
%410 = OpLabel
%412 = OpLoad %int %i
%413 = OpLoad %v2int %i2
%414 = OpCompositeExtract %int %413 0
%415 = OpIMul %int %412 %414
%416 = OpLoad %v3int %i3
%417 = OpCompositeExtract %int %416 0
%418 = OpIMul %int %415 %417
%419 = OpLoad %v4int %i4
%420 = OpCompositeExtract %int %419 0
%421 = OpIMul %int %418 %420
%422 = OpIEqual %bool %int_1 %421
OpBranch %411
%411 = OpLabel
%423 = OpPhi %bool %false %384 %422 %410
OpStore %ok %423
%424 = OpLoad %bool %ok
OpSelectionMerge %426 None
OpBranchConditional %424 %425 %426
%425 = OpLabel
%427 = OpLoad %bool %b
OpSelectionMerge %429 None
OpBranchConditional %427 %428 %429
%428 = OpLabel
%430 = OpLoad %v2bool %b2
%431 = OpCompositeExtract %bool %430 0
OpBranch %429
%429 = OpLabel
%432 = OpPhi %bool %false %425 %431 %428
OpSelectionMerge %434 None
OpBranchConditional %432 %433 %434
%433 = OpLabel
%435 = OpLoad %v3bool %b3
%436 = OpCompositeExtract %bool %435 0
OpBranch %434
%434 = OpLabel
%437 = OpPhi %bool %false %429 %436 %433
OpSelectionMerge %439 None
OpBranchConditional %437 %438 %439
%438 = OpLabel
%440 = OpLoad %v4bool %b4
%441 = OpCompositeExtract %bool %440 0
OpBranch %439
%439 = OpLabel
%442 = OpPhi %bool %false %434 %441 %438
OpBranch %426
%426 = OpLabel
%443 = OpPhi %bool %false %411 %442 %439
OpStore %ok %443
%444 = OpLoad %bool %ok
OpSelectionMerge %448 None
OpBranchConditional %444 %446 %447
%446 = OpLabel
%449 = OpAccessChain %_ptr_Uniform_v4float %32 %int_0
%450 = OpLoad %v4float %449
OpStore %445 %450
OpBranch %448
%447 = OpLabel
%451 = OpAccessChain %_ptr_Uniform_v4float %32 %int_1
%452 = OpLoad %v4float %451
OpStore %445 %452
OpBranch %448
%448 = OpLabel
%453 = OpLoad %v4float %445
OpReturnValue %453
OpFunctionEnd
