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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %_1_m3 "_1_m3"
OpName %_2_m5 "_2_m5"
OpName %_3_m6 "_3_m6"
OpName %_4_m11 "_4_m11"
OpName %_6_m3 "_6_m3"
OpName %_7_m5 "_7_m5"
OpName %_8_m6 "_8_m6"
OpName %_9_m11 "_9_m11"
OpName %_11_ok "_11_ok"
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
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %117 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %160 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %187 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %204 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %229 RelaxedPrecision
OpDecorate %230 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
OpDecorate %259 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%22 = OpTypeFunction %v4float
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_0 = OpConstant %float 0
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%mat4v4float = OpTypeMatrix %v4float 4
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int_2 = OpConstant %int 2
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int_3 = OpConstant %int 3
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%v3bool = OpTypeVector %bool 3
%float_100 = OpConstant %float 100
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_1 = OpConstant %int 1
%_entrypoint = OpFunction %void None %19
%20 = OpLabel
%21 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %21
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %22
%23 = OpLabel
%_1_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_2_m5 = OpVariable %_ptr_Function_mat2v2float Function
%40 = OpVariable %_ptr_Function_mat2v2float Function
%_3_m6 = OpVariable %_ptr_Function_mat2v2float Function
%_4_m11 = OpVariable %_ptr_Function_mat4v4float Function
%_6_m3 = OpVariable %_ptr_Function_mat2v2float Function
%_7_m5 = OpVariable %_ptr_Function_mat2v2float Function
%103 = OpVariable %_ptr_Function_mat2v2float Function
%_8_m6 = OpVariable %_ptr_Function_mat2v2float Function
%_9_m11 = OpVariable %_ptr_Function_mat4v4float Function
%_11_ok = OpVariable %_ptr_Function_bool Function
%248 = OpVariable %_ptr_Function_v4float Function
%31 = OpCompositeConstruct %v2float %float_1 %float_2
%32 = OpCompositeConstruct %v2float %float_3 %float_4
%30 = OpCompositeConstruct %mat2v2float %31 %32
OpStore %_1_m3 %30
%33 = OpLoad %mat2v2float %_1_m3
%36 = OpCompositeConstruct %v2float %float_1 %float_0
%37 = OpCompositeConstruct %v2float %float_0 %float_1
%34 = OpCompositeConstruct %mat2v2float %36 %37
%38 = OpMatrixTimesMatrix %mat2v2float %33 %34
OpStore %_1_m3 %38
%42 = OpCompositeConstruct %v2float %float_1 %float_2
%43 = OpCompositeConstruct %v2float %float_3 %float_4
%41 = OpCompositeConstruct %mat2v2float %42 %43
OpStore %40 %41
%46 = OpAccessChain %_ptr_Function_v2float %40 %int_0
%48 = OpLoad %v2float %46
%49 = OpCompositeExtract %float %48 0
%51 = OpCompositeConstruct %v2float %49 %float_0
%52 = OpCompositeConstruct %v2float %float_0 %49
%50 = OpCompositeConstruct %mat2v2float %51 %52
OpStore %_2_m5 %50
%55 = OpCompositeConstruct %v2float %float_1 %float_2
%56 = OpCompositeConstruct %v2float %float_3 %float_4
%54 = OpCompositeConstruct %mat2v2float %55 %56
OpStore %_3_m6 %54
%57 = OpLoad %mat2v2float %_3_m6
%58 = OpLoad %mat2v2float %_2_m5
%59 = OpCompositeExtract %v2float %57 0
%60 = OpCompositeExtract %v2float %58 0
%61 = OpFAdd %v2float %59 %60
%62 = OpCompositeExtract %v2float %57 1
%63 = OpCompositeExtract %v2float %58 1
%64 = OpFAdd %v2float %62 %63
%65 = OpCompositeConstruct %mat2v2float %61 %64
OpStore %_3_m6 %65
%70 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%71 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%72 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%73 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%69 = OpCompositeConstruct %mat4v4float %70 %71 %72 %73
OpStore %_4_m11 %69
%74 = OpLoad %mat4v4float %_4_m11
%76 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%77 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%78 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%79 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%75 = OpCompositeConstruct %mat4v4float %76 %77 %78 %79
%80 = OpCompositeExtract %v4float %74 0
%81 = OpCompositeExtract %v4float %75 0
%82 = OpFSub %v4float %80 %81
%83 = OpCompositeExtract %v4float %74 1
%84 = OpCompositeExtract %v4float %75 1
%85 = OpFSub %v4float %83 %84
%86 = OpCompositeExtract %v4float %74 2
%87 = OpCompositeExtract %v4float %75 2
%88 = OpFSub %v4float %86 %87
%89 = OpCompositeExtract %v4float %74 3
%90 = OpCompositeExtract %v4float %75 3
%91 = OpFSub %v4float %89 %90
%92 = OpCompositeConstruct %mat4v4float %82 %85 %88 %91
OpStore %_4_m11 %92
%95 = OpCompositeConstruct %v2float %float_1 %float_2
%96 = OpCompositeConstruct %v2float %float_3 %float_4
%94 = OpCompositeConstruct %mat2v2float %95 %96
OpStore %_6_m3 %94
%97 = OpLoad %mat2v2float %_6_m3
%99 = OpCompositeConstruct %v2float %float_1 %float_0
%100 = OpCompositeConstruct %v2float %float_0 %float_1
%98 = OpCompositeConstruct %mat2v2float %99 %100
%101 = OpMatrixTimesMatrix %mat2v2float %97 %98
OpStore %_6_m3 %101
%105 = OpCompositeConstruct %v2float %float_1 %float_2
%106 = OpCompositeConstruct %v2float %float_3 %float_4
%104 = OpCompositeConstruct %mat2v2float %105 %106
OpStore %103 %104
%107 = OpAccessChain %_ptr_Function_v2float %103 %int_0
%108 = OpLoad %v2float %107
%109 = OpCompositeExtract %float %108 0
%111 = OpCompositeConstruct %v2float %109 %float_0
%112 = OpCompositeConstruct %v2float %float_0 %109
%110 = OpCompositeConstruct %mat2v2float %111 %112
OpStore %_7_m5 %110
%115 = OpCompositeConstruct %v2float %float_1 %float_2
%116 = OpCompositeConstruct %v2float %float_3 %float_4
%114 = OpCompositeConstruct %mat2v2float %115 %116
OpStore %_8_m6 %114
%117 = OpLoad %mat2v2float %_8_m6
%118 = OpLoad %mat2v2float %_7_m5
%119 = OpCompositeExtract %v2float %117 0
%120 = OpCompositeExtract %v2float %118 0
%121 = OpFAdd %v2float %119 %120
%122 = OpCompositeExtract %v2float %117 1
%123 = OpCompositeExtract %v2float %118 1
%124 = OpFAdd %v2float %122 %123
%125 = OpCompositeConstruct %mat2v2float %121 %124
OpStore %_8_m6 %125
%128 = OpCompositeConstruct %v4float %float_2 %float_0 %float_0 %float_0
%129 = OpCompositeConstruct %v4float %float_0 %float_2 %float_0 %float_0
%130 = OpCompositeConstruct %v4float %float_0 %float_0 %float_2 %float_0
%131 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_2
%127 = OpCompositeConstruct %mat4v4float %128 %129 %130 %131
OpStore %_9_m11 %127
%132 = OpLoad %mat4v4float %_9_m11
%134 = OpCompositeConstruct %v4float %float_1 %float_0 %float_0 %float_0
%135 = OpCompositeConstruct %v4float %float_0 %float_1 %float_0 %float_0
%136 = OpCompositeConstruct %v4float %float_0 %float_0 %float_1 %float_0
%137 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_1
%133 = OpCompositeConstruct %mat4v4float %134 %135 %136 %137
%138 = OpCompositeExtract %v4float %132 0
%139 = OpCompositeExtract %v4float %133 0
%140 = OpFSub %v4float %138 %139
%141 = OpCompositeExtract %v4float %132 1
%142 = OpCompositeExtract %v4float %133 1
%143 = OpFSub %v4float %141 %142
%144 = OpCompositeExtract %v4float %132 2
%145 = OpCompositeExtract %v4float %133 2
%146 = OpFSub %v4float %144 %145
%147 = OpCompositeExtract %v4float %132 3
%148 = OpCompositeExtract %v4float %133 3
%149 = OpFSub %v4float %147 %148
%150 = OpCompositeConstruct %mat4v4float %140 %143 %146 %149
OpStore %_9_m11 %150
OpStore %_11_ok %true
%154 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%157 = OpLoad %mat2v2float %154
%159 = OpCompositeConstruct %v2float %float_1 %float_2
%160 = OpCompositeConstruct %v2float %float_3 %float_4
%158 = OpCompositeConstruct %mat2v2float %159 %160
%162 = OpCompositeExtract %v2float %157 0
%163 = OpCompositeExtract %v2float %158 0
%164 = OpFOrdEqual %v2bool %162 %163
%165 = OpAll %bool %164
%166 = OpCompositeExtract %v2float %157 1
%167 = OpCompositeExtract %v2float %158 1
%168 = OpFOrdEqual %v2bool %166 %167
%169 = OpAll %bool %168
%170 = OpLogicalAnd %bool %165 %169
OpStore %_11_ok %170
%172 = OpLoad %bool %_11_ok
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%175 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%178 = OpLoad %mat3v3float %175
%185 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%186 = OpCompositeConstruct %v3float %float_4 %float_5 %float_6
%187 = OpCompositeConstruct %v3float %float_7 %float_8 %float_9
%184 = OpCompositeConstruct %mat3v3float %185 %186 %187
%189 = OpCompositeExtract %v3float %178 0
%190 = OpCompositeExtract %v3float %184 0
%191 = OpFOrdEqual %v3bool %189 %190
%192 = OpAll %bool %191
%193 = OpCompositeExtract %v3float %178 1
%194 = OpCompositeExtract %v3float %184 1
%195 = OpFOrdEqual %v3bool %193 %194
%196 = OpAll %bool %195
%197 = OpLogicalAnd %bool %192 %196
%198 = OpCompositeExtract %v3float %178 2
%199 = OpCompositeExtract %v3float %184 2
%200 = OpFOrdEqual %v3bool %198 %199
%201 = OpAll %bool %200
%202 = OpLogicalAnd %bool %197 %201
OpBranch %174
%174 = OpLabel
%203 = OpPhi %bool %false %23 %202 %173
OpStore %_11_ok %203
%204 = OpLoad %bool %_11_ok
OpSelectionMerge %206 None
OpBranchConditional %204 %205 %206
%205 = OpLabel
%207 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%208 = OpLoad %mat2v2float %207
%211 = OpCompositeConstruct %v2float %float_100 %float_0
%212 = OpCompositeConstruct %v2float %float_0 %float_100
%210 = OpCompositeConstruct %mat2v2float %211 %212
%213 = OpCompositeExtract %v2float %208 0
%214 = OpCompositeExtract %v2float %210 0
%215 = OpFOrdNotEqual %v2bool %213 %214
%216 = OpAny %bool %215
%217 = OpCompositeExtract %v2float %208 1
%218 = OpCompositeExtract %v2float %210 1
%219 = OpFOrdNotEqual %v2bool %217 %218
%220 = OpAny %bool %219
%221 = OpLogicalOr %bool %216 %220
OpBranch %206
%206 = OpLabel
%222 = OpPhi %bool %false %174 %221 %205
OpStore %_11_ok %222
%223 = OpLoad %bool %_11_ok
OpSelectionMerge %225 None
OpBranchConditional %223 %224 %225
%224 = OpLabel
%226 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%227 = OpLoad %mat3v3float %226
%229 = OpCompositeConstruct %v3float %float_9 %float_8 %float_7
%230 = OpCompositeConstruct %v3float %float_6 %float_5 %float_4
%231 = OpCompositeConstruct %v3float %float_3 %float_2 %float_1
%228 = OpCompositeConstruct %mat3v3float %229 %230 %231
%232 = OpCompositeExtract %v3float %227 0
%233 = OpCompositeExtract %v3float %228 0
%234 = OpFOrdNotEqual %v3bool %232 %233
%235 = OpAny %bool %234
%236 = OpCompositeExtract %v3float %227 1
%237 = OpCompositeExtract %v3float %228 1
%238 = OpFOrdNotEqual %v3bool %236 %237
%239 = OpAny %bool %238
%240 = OpLogicalOr %bool %235 %239
%241 = OpCompositeExtract %v3float %227 2
%242 = OpCompositeExtract %v3float %228 2
%243 = OpFOrdNotEqual %v3bool %241 %242
%244 = OpAny %bool %243
%245 = OpLogicalOr %bool %240 %244
OpBranch %225
%225 = OpLabel
%246 = OpPhi %bool %false %206 %245 %224
OpStore %_11_ok %246
%247 = OpLoad %bool %_11_ok
OpSelectionMerge %252 None
OpBranchConditional %247 %250 %251
%250 = OpLabel
%253 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%255 = OpLoad %v4float %253
OpStore %248 %255
OpBranch %252
%251 = OpLabel
%256 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%258 = OpLoad %v4float %256
OpStore %248 %258
OpBranch %252
%252 = OpLabel
%259 = OpLoad %v4float %248
OpReturnValue %259
OpFunctionEnd
