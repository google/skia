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
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %263 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %269 RelaxedPrecision
OpDecorate %270 RelaxedPrecision
OpDecorate %271 RelaxedPrecision
OpDecorate %h2x2 RelaxedPrecision
OpDecorate %h3x3 RelaxedPrecision
OpDecorate %h4x4 RelaxedPrecision
OpDecorate %279 RelaxedPrecision
OpDecorate %282 RelaxedPrecision
OpDecorate %286 RelaxedPrecision
OpDecorate %289 RelaxedPrecision
OpDecorate %293 RelaxedPrecision
OpDecorate %296 RelaxedPrecision
OpDecorate %365 RelaxedPrecision
OpDecorate %372 RelaxedPrecision
OpDecorate %376 RelaxedPrecision
OpDecorate %377 RelaxedPrecision
OpDecorate %378 RelaxedPrecision
OpDecorate %379 RelaxedPrecision
OpDecorate %380 RelaxedPrecision
OpDecorate %381 RelaxedPrecision
OpDecorate %382 RelaxedPrecision
OpDecorate %383 RelaxedPrecision
OpDecorate %384 RelaxedPrecision
OpDecorate %385 RelaxedPrecision
OpDecorate %387 RelaxedPrecision
OpDecorate %388 RelaxedPrecision
OpDecorate %389 RelaxedPrecision
OpDecorate %391 RelaxedPrecision
OpDecorate %392 RelaxedPrecision
OpDecorate %393 RelaxedPrecision
OpDecorate %395 RelaxedPrecision
OpDecorate %396 RelaxedPrecision
OpDecorate %397 RelaxedPrecision
OpDecorate %400 RelaxedPrecision
OpDecorate %427 RelaxedPrecision
OpDecorate %442 RelaxedPrecision
OpDecorate %445 RelaxedPrecision
OpDecorate %448 RelaxedPrecision
OpDecorate %453 RelaxedPrecision
OpDecorate %458 RelaxedPrecision
OpDecorate %462 RelaxedPrecision
OpDecorate %468 RelaxedPrecision
OpDecorate %470 RelaxedPrecision
OpDecorate %471 RelaxedPrecision
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
%255 = OpVariable %_ptr_Function_float Function
%260 = OpVariable %_ptr_Function_v2float Function
%267 = OpVariable %_ptr_Function_v4float Function
%h2x2 = OpVariable %_ptr_Function_mat2v2float Function
%h3x3 = OpVariable %_ptr_Function_mat3v3float Function
%h4x4 = OpVariable %_ptr_Function_mat4v4float Function
%280 = OpVariable %_ptr_Function_v3float Function
%287 = OpVariable %_ptr_Function_float Function
%294 = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%i2 = OpVariable %_ptr_Function_v2int Function
%i3 = OpVariable %_ptr_Function_v3int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%307 = OpVariable %_ptr_Function_v3int Function
%314 = OpVariable %_ptr_Function_int Function
%f = OpVariable %_ptr_Function_float Function
%f2 = OpVariable %_ptr_Function_v2float Function
%f3 = OpVariable %_ptr_Function_v3float Function
%f4 = OpVariable %_ptr_Function_v4float Function
%327 = OpVariable %_ptr_Function_v2float Function
%334 = OpVariable %_ptr_Function_float Function
%f2x2 = OpVariable %_ptr_Function_mat2v2float Function
%f3x3 = OpVariable %_ptr_Function_mat3v3float Function
%f4x4 = OpVariable %_ptr_Function_mat4v4float Function
%346 = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_bool Function
%b2 = OpVariable %_ptr_Function_v2bool Function
%b3 = OpVariable %_ptr_Function_v3bool Function
%b4 = OpVariable %_ptr_Function_v4bool Function
%359 = OpVariable %_ptr_Function_v2bool Function
%366 = OpVariable %_ptr_Function_bool Function
%ok = OpVariable %_ptr_Function_bool Function
%463 = OpVariable %_ptr_Function_v4float Function
%245 = OpFunctionCall %void %out_half_vh %h
%247 = OpFunctionCall %void %out_half2_vh2 %h2
%249 = OpFunctionCall %void %out_half3_vh3 %h3
%251 = OpFunctionCall %void %out_half4_vh4 %h4
%252 = OpAccessChain %_ptr_Function_float %h3 %int_1
%254 = OpLoad %float %252
OpStore %255 %254
%256 = OpFunctionCall %void %out_half_vh %255
%257 = OpLoad %float %255
OpStore %252 %257
%258 = OpLoad %v3float %h3
%259 = OpVectorShuffle %v2float %258 %258 0 2
OpStore %260 %259
%261 = OpFunctionCall %void %out_half2_vh2 %260
%262 = OpLoad %v2float %260
%263 = OpLoad %v3float %h3
%264 = OpVectorShuffle %v3float %263 %262 3 1 4
OpStore %h3 %264
%265 = OpLoad %v4float %h4
%266 = OpVectorShuffle %v4float %265 %265 2 3 0 1
OpStore %267 %266
%268 = OpFunctionCall %void %out_half4_vh4 %267
%269 = OpLoad %v4float %267
%270 = OpLoad %v4float %h4
%271 = OpVectorShuffle %v4float %270 %269 6 7 4 5
OpStore %h4 %271
%273 = OpFunctionCall %void %out_half2x2_vh22 %h2x2
%275 = OpFunctionCall %void %out_half3x3_vh33 %h3x3
%277 = OpFunctionCall %void %out_half4x4_vh44 %h4x4
%278 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_1
%279 = OpLoad %v3float %278
OpStore %280 %279
%281 = OpFunctionCall %void %out_half3_vh3 %280
%282 = OpLoad %v3float %280
OpStore %278 %282
%284 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_3
%285 = OpAccessChain %_ptr_Function_float %284 %int_3
%286 = OpLoad %float %285
OpStore %287 %286
%288 = OpFunctionCall %void %out_half_vh %287
%289 = OpLoad %float %287
OpStore %285 %289
%291 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%292 = OpAccessChain %_ptr_Function_float %291 %int_0
%293 = OpLoad %float %292
OpStore %294 %293
%295 = OpFunctionCall %void %out_half_vh %294
%296 = OpLoad %float %294
OpStore %292 %296
%298 = OpFunctionCall %void %out_int_vi %i
%300 = OpFunctionCall %void %out_int2_vi2 %i2
%302 = OpFunctionCall %void %out_int3_vi3 %i3
%304 = OpFunctionCall %void %out_int4_vi4 %i4
%305 = OpLoad %v4int %i4
%306 = OpVectorShuffle %v3int %305 %305 0 1 2
OpStore %307 %306
%308 = OpFunctionCall %void %out_int3_vi3 %307
%309 = OpLoad %v3int %307
%310 = OpLoad %v4int %i4
%311 = OpVectorShuffle %v4int %310 %309 4 5 6 3
OpStore %i4 %311
%312 = OpAccessChain %_ptr_Function_int %i2 %int_1
%313 = OpLoad %int %312
OpStore %314 %313
%315 = OpFunctionCall %void %out_int_vi %314
%316 = OpLoad %int %314
OpStore %312 %316
%318 = OpFunctionCall %void %out_float_vf %f
%320 = OpFunctionCall %void %out_float2_vf2 %f2
%322 = OpFunctionCall %void %out_float3_vf3 %f3
%324 = OpFunctionCall %void %out_float4_vf4 %f4
%325 = OpLoad %v3float %f3
%326 = OpVectorShuffle %v2float %325 %325 0 1
OpStore %327 %326
%328 = OpFunctionCall %void %out_float2_vf2 %327
%329 = OpLoad %v2float %327
%330 = OpLoad %v3float %f3
%331 = OpVectorShuffle %v3float %330 %329 3 4 2
OpStore %f3 %331
%332 = OpAccessChain %_ptr_Function_float %f2 %int_0
%333 = OpLoad %float %332
OpStore %334 %333
%335 = OpFunctionCall %void %out_float_vf %334
%336 = OpLoad %float %334
OpStore %332 %336
%338 = OpFunctionCall %void %out_float2x2_vf22 %f2x2
%340 = OpFunctionCall %void %out_float3x3_vf33 %f3x3
%342 = OpFunctionCall %void %out_float4x4_vf44 %f4x4
%343 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%344 = OpAccessChain %_ptr_Function_float %343 %int_0
%345 = OpLoad %float %344
OpStore %346 %345
%347 = OpFunctionCall %void %out_float_vf %346
%348 = OpLoad %float %346
OpStore %344 %348
%350 = OpFunctionCall %void %out_bool_vb %b
%352 = OpFunctionCall %void %out_bool2_vb2 %b2
%354 = OpFunctionCall %void %out_bool3_vb3 %b3
%356 = OpFunctionCall %void %out_bool4_vb4 %b4
%357 = OpLoad %v4bool %b4
%358 = OpVectorShuffle %v2bool %357 %357 0 3
OpStore %359 %358
%360 = OpFunctionCall %void %out_bool2_vb2 %359
%361 = OpLoad %v2bool %359
%362 = OpLoad %v4bool %b4
%363 = OpVectorShuffle %v4bool %362 %361 4 1 2 5
OpStore %b4 %363
%364 = OpAccessChain %_ptr_Function_bool %b3 %int_2
%365 = OpLoad %bool %364
OpStore %366 %365
%367 = OpFunctionCall %void %out_bool_vb %366
%368 = OpLoad %bool %366
OpStore %364 %368
OpStore %ok %true
%372 = OpLoad %bool %ok
OpSelectionMerge %374 None
OpBranchConditional %372 %373 %374
%373 = OpLabel
%376 = OpLoad %float %h
%377 = OpLoad %v2float %h2
%378 = OpCompositeExtract %float %377 0
%379 = OpFMul %float %376 %378
%380 = OpLoad %v3float %h3
%381 = OpCompositeExtract %float %380 0
%382 = OpFMul %float %379 %381
%383 = OpLoad %v4float %h4
%384 = OpCompositeExtract %float %383 0
%385 = OpFMul %float %382 %384
%386 = OpAccessChain %_ptr_Function_v2float %h2x2 %int_0
%387 = OpLoad %v2float %386
%388 = OpCompositeExtract %float %387 0
%389 = OpFMul %float %385 %388
%390 = OpAccessChain %_ptr_Function_v3float %h3x3 %int_0
%391 = OpLoad %v3float %390
%392 = OpCompositeExtract %float %391 0
%393 = OpFMul %float %389 %392
%394 = OpAccessChain %_ptr_Function_v4float %h4x4 %int_0
%395 = OpLoad %v4float %394
%396 = OpCompositeExtract %float %395 0
%397 = OpFMul %float %393 %396
%398 = OpFOrdEqual %bool %float_1 %397
OpBranch %374
%374 = OpLabel
%399 = OpPhi %bool %false %243 %398 %373
OpStore %ok %399
%400 = OpLoad %bool %ok
OpSelectionMerge %402 None
OpBranchConditional %400 %401 %402
%401 = OpLabel
%403 = OpLoad %float %f
%404 = OpLoad %v2float %f2
%405 = OpCompositeExtract %float %404 0
%406 = OpFMul %float %403 %405
%407 = OpLoad %v3float %f3
%408 = OpCompositeExtract %float %407 0
%409 = OpFMul %float %406 %408
%410 = OpLoad %v4float %f4
%411 = OpCompositeExtract %float %410 0
%412 = OpFMul %float %409 %411
%413 = OpAccessChain %_ptr_Function_v2float %f2x2 %int_0
%414 = OpLoad %v2float %413
%415 = OpCompositeExtract %float %414 0
%416 = OpFMul %float %412 %415
%417 = OpAccessChain %_ptr_Function_v3float %f3x3 %int_0
%418 = OpLoad %v3float %417
%419 = OpCompositeExtract %float %418 0
%420 = OpFMul %float %416 %419
%421 = OpAccessChain %_ptr_Function_v4float %f4x4 %int_0
%422 = OpLoad %v4float %421
%423 = OpCompositeExtract %float %422 0
%424 = OpFMul %float %420 %423
%425 = OpFOrdEqual %bool %float_1 %424
OpBranch %402
%402 = OpLabel
%426 = OpPhi %bool %false %374 %425 %401
OpStore %ok %426
%427 = OpLoad %bool %ok
OpSelectionMerge %429 None
OpBranchConditional %427 %428 %429
%428 = OpLabel
%430 = OpLoad %int %i
%431 = OpLoad %v2int %i2
%432 = OpCompositeExtract %int %431 0
%433 = OpIMul %int %430 %432
%434 = OpLoad %v3int %i3
%435 = OpCompositeExtract %int %434 0
%436 = OpIMul %int %433 %435
%437 = OpLoad %v4int %i4
%438 = OpCompositeExtract %int %437 0
%439 = OpIMul %int %436 %438
%440 = OpIEqual %bool %int_1 %439
OpBranch %429
%429 = OpLabel
%441 = OpPhi %bool %false %402 %440 %428
OpStore %ok %441
%442 = OpLoad %bool %ok
OpSelectionMerge %444 None
OpBranchConditional %442 %443 %444
%443 = OpLabel
%445 = OpLoad %bool %b
OpSelectionMerge %447 None
OpBranchConditional %445 %446 %447
%446 = OpLabel
%448 = OpLoad %v2bool %b2
%449 = OpCompositeExtract %bool %448 0
OpBranch %447
%447 = OpLabel
%450 = OpPhi %bool %false %443 %449 %446
OpSelectionMerge %452 None
OpBranchConditional %450 %451 %452
%451 = OpLabel
%453 = OpLoad %v3bool %b3
%454 = OpCompositeExtract %bool %453 0
OpBranch %452
%452 = OpLabel
%455 = OpPhi %bool %false %447 %454 %451
OpSelectionMerge %457 None
OpBranchConditional %455 %456 %457
%456 = OpLabel
%458 = OpLoad %v4bool %b4
%459 = OpCompositeExtract %bool %458 0
OpBranch %457
%457 = OpLabel
%460 = OpPhi %bool %false %452 %459 %456
OpBranch %444
%444 = OpLabel
%461 = OpPhi %bool %false %429 %460 %457
OpStore %ok %461
%462 = OpLoad %bool %ok
OpSelectionMerge %466 None
OpBranchConditional %462 %464 %465
%464 = OpLabel
%467 = OpAccessChain %_ptr_Uniform_v4float %32 %int_0
%468 = OpLoad %v4float %467
OpStore %463 %468
OpBranch %466
%465 = OpLabel
%469 = OpAccessChain %_ptr_Uniform_v4float %32 %int_1
%470 = OpLoad %v4float %469
OpStore %463 %470
OpBranch %466
%466 = OpLabel
%471 = OpLoad %v4float %463
OpReturnValue %471
OpFunctionEnd
