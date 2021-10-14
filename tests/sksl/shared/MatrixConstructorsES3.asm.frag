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
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %f4 "f4"
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
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %73 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %254 RelaxedPrecision
OpDecorate %257 RelaxedPrecision
OpDecorate %258 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%_ptr_Function_bool = OpTypePointer Function %bool
%v3float = OpTypeVector %float 3
%mat2v3float = OpTypeMatrix %v3float 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%v3bool = OpTypeVector %bool 3
%false = OpConstantFalse %bool
%mat2v4float = OpTypeMatrix %v4float 2
%v4bool = OpTypeVector %bool 4
%mat3v3float = OpTypeMatrix %v3float 3
%mat4v2float = OpTypeMatrix %v2float 4
%v2bool = OpTypeVector %bool 2
%mat4v3float = OpTypeMatrix %v3float 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%f4 = OpVariable %_ptr_Function_v4float Function
%ok = OpVariable %_ptr_Function_bool Function
%247 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
OpStore %f4 %38
%41 = OpLoad %v4float %f4
%42 = OpLoad %v4float %f4
%43 = OpVectorShuffle %v2float %42 %42 0 1
%44 = OpCompositeExtract %float %41 0
%45 = OpCompositeExtract %float %41 1
%46 = OpCompositeExtract %float %41 2
%47 = OpCompositeConstruct %v3float %44 %45 %46
%49 = OpCompositeExtract %float %41 3
%50 = OpCompositeExtract %float %43 0
%51 = OpCompositeExtract %float %43 1
%52 = OpCompositeConstruct %v3float %49 %50 %51
%53 = OpCompositeConstruct %mat2v3float %47 %52
%59 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%60 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%61 = OpCompositeConstruct %mat2v3float %59 %60
%63 = OpCompositeExtract %v3float %53 0
%64 = OpCompositeExtract %v3float %61 0
%65 = OpFOrdEqual %v3bool %63 %64
%66 = OpAll %bool %65
%67 = OpCompositeExtract %v3float %53 1
%68 = OpCompositeExtract %v3float %61 1
%69 = OpFOrdEqual %v3bool %67 %68
%70 = OpAll %bool %69
%71 = OpLogicalAnd %bool %66 %70
OpStore %ok %71
%73 = OpLoad %bool %ok
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%76 = OpLoad %v4float %f4
%77 = OpVectorShuffle %v3float %76 %76 0 1 2
%78 = OpLoad %v4float %f4
%79 = OpVectorShuffle %v4float %78 %78 3 0 1 2
%80 = OpLoad %v4float %f4
%81 = OpCompositeExtract %float %80 3
%82 = OpCompositeExtract %float %77 0
%83 = OpCompositeExtract %float %77 1
%84 = OpCompositeExtract %float %77 2
%85 = OpCompositeExtract %float %79 0
%86 = OpCompositeConstruct %v4float %82 %83 %84 %85
%87 = OpCompositeExtract %float %79 1
%88 = OpCompositeExtract %float %79 2
%89 = OpCompositeExtract %float %79 3
%90 = OpCompositeConstruct %v4float %87 %88 %89 %81
%91 = OpCompositeConstruct %mat2v4float %86 %90
%93 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%94 = OpCompositeConstruct %v4float %float_1 %float_2 %float_3 %float_4
%95 = OpCompositeConstruct %mat2v4float %93 %94
%97 = OpCompositeExtract %v4float %91 0
%98 = OpCompositeExtract %v4float %95 0
%99 = OpFOrdEqual %v4bool %97 %98
%100 = OpAll %bool %99
%101 = OpCompositeExtract %v4float %91 1
%102 = OpCompositeExtract %v4float %95 1
%103 = OpFOrdEqual %v4bool %101 %102
%104 = OpAll %bool %103
%105 = OpLogicalAnd %bool %100 %104
OpBranch %75
%75 = OpLabel
%106 = OpPhi %bool %false %26 %105 %74
OpStore %ok %106
%107 = OpLoad %bool %ok
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%110 = OpLoad %v4float %f4
%111 = OpVectorShuffle %v2float %110 %110 0 1
%112 = OpLoad %v4float %f4
%113 = OpVectorShuffle %v2float %112 %112 2 3
%114 = OpLoad %v4float %f4
%115 = OpLoad %v4float %f4
%116 = OpCompositeExtract %float %115 0
%117 = OpCompositeExtract %float %111 0
%118 = OpCompositeExtract %float %111 1
%119 = OpCompositeExtract %float %113 0
%120 = OpCompositeConstruct %v3float %117 %118 %119
%121 = OpCompositeExtract %float %113 1
%122 = OpCompositeExtract %float %114 0
%123 = OpCompositeExtract %float %114 1
%124 = OpCompositeConstruct %v3float %121 %122 %123
%125 = OpCompositeExtract %float %114 2
%126 = OpCompositeExtract %float %114 3
%127 = OpCompositeConstruct %v3float %125 %126 %116
%128 = OpCompositeConstruct %mat3v3float %120 %124 %127
%130 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%131 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%132 = OpCompositeConstruct %v3float %float_3 %float_4 %float_1
%133 = OpCompositeConstruct %mat3v3float %130 %131 %132
%134 = OpCompositeExtract %v3float %128 0
%135 = OpCompositeExtract %v3float %133 0
%136 = OpFOrdEqual %v3bool %134 %135
%137 = OpAll %bool %136
%138 = OpCompositeExtract %v3float %128 1
%139 = OpCompositeExtract %v3float %133 1
%140 = OpFOrdEqual %v3bool %138 %139
%141 = OpAll %bool %140
%142 = OpLogicalAnd %bool %137 %141
%143 = OpCompositeExtract %v3float %128 2
%144 = OpCompositeExtract %v3float %133 2
%145 = OpFOrdEqual %v3bool %143 %144
%146 = OpAll %bool %145
%147 = OpLogicalAnd %bool %142 %146
OpBranch %109
%109 = OpLabel
%148 = OpPhi %bool %false %75 %147 %108
OpStore %ok %148
%149 = OpLoad %bool %ok
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
%152 = OpLoad %v4float %f4
%153 = OpVectorShuffle %v3float %152 %152 0 1 2
%154 = OpLoad %v4float %f4
%155 = OpVectorShuffle %v4float %154 %154 3 0 1 2
%156 = OpLoad %v4float %f4
%157 = OpCompositeExtract %float %156 3
%158 = OpCompositeExtract %float %153 0
%159 = OpCompositeExtract %float %153 1
%160 = OpCompositeConstruct %v2float %158 %159
%161 = OpCompositeExtract %float %153 2
%162 = OpCompositeExtract %float %155 0
%163 = OpCompositeConstruct %v2float %161 %162
%164 = OpCompositeExtract %float %155 1
%165 = OpCompositeExtract %float %155 2
%166 = OpCompositeConstruct %v2float %164 %165
%167 = OpCompositeExtract %float %155 3
%168 = OpCompositeConstruct %v2float %167 %157
%169 = OpCompositeConstruct %mat4v2float %160 %163 %166 %168
%171 = OpCompositeConstruct %v2float %float_1 %float_2
%172 = OpCompositeConstruct %v2float %float_3 %float_4
%173 = OpCompositeConstruct %v2float %float_1 %float_2
%174 = OpCompositeConstruct %v2float %float_3 %float_4
%175 = OpCompositeConstruct %mat4v2float %171 %172 %173 %174
%177 = OpCompositeExtract %v2float %169 0
%178 = OpCompositeExtract %v2float %175 0
%179 = OpFOrdEqual %v2bool %177 %178
%180 = OpAll %bool %179
%181 = OpCompositeExtract %v2float %169 1
%182 = OpCompositeExtract %v2float %175 1
%183 = OpFOrdEqual %v2bool %181 %182
%184 = OpAll %bool %183
%185 = OpLogicalAnd %bool %180 %184
%186 = OpCompositeExtract %v2float %169 2
%187 = OpCompositeExtract %v2float %175 2
%188 = OpFOrdEqual %v2bool %186 %187
%189 = OpAll %bool %188
%190 = OpLogicalAnd %bool %185 %189
%191 = OpCompositeExtract %v2float %169 3
%192 = OpCompositeExtract %v2float %175 3
%193 = OpFOrdEqual %v2bool %191 %192
%194 = OpAll %bool %193
%195 = OpLogicalAnd %bool %190 %194
OpBranch %151
%151 = OpLabel
%196 = OpPhi %bool %false %109 %195 %150
OpStore %ok %196
%197 = OpLoad %bool %ok
OpSelectionMerge %199 None
OpBranchConditional %197 %198 %199
%198 = OpLabel
%200 = OpLoad %v4float %f4
%201 = OpCompositeExtract %float %200 0
%202 = OpLoad %v4float %f4
%203 = OpVectorShuffle %v4float %202 %202 1 2 3 0
%204 = OpLoad %v4float %f4
%205 = OpVectorShuffle %v4float %204 %204 1 2 3 0
%206 = OpLoad %v4float %f4
%207 = OpVectorShuffle %v3float %206 %206 1 2 3
%208 = OpCompositeExtract %float %203 0
%209 = OpCompositeExtract %float %203 1
%210 = OpCompositeConstruct %v3float %201 %208 %209
%211 = OpCompositeExtract %float %203 2
%212 = OpCompositeExtract %float %203 3
%213 = OpCompositeExtract %float %205 0
%214 = OpCompositeConstruct %v3float %211 %212 %213
%215 = OpCompositeExtract %float %205 1
%216 = OpCompositeExtract %float %205 2
%217 = OpCompositeExtract %float %205 3
%218 = OpCompositeConstruct %v3float %215 %216 %217
%219 = OpCompositeConstruct %mat4v3float %210 %214 %218 %207
%221 = OpCompositeConstruct %v3float %float_1 %float_2 %float_3
%222 = OpCompositeConstruct %v3float %float_4 %float_1 %float_2
%223 = OpCompositeConstruct %v3float %float_3 %float_4 %float_1
%224 = OpCompositeConstruct %v3float %float_2 %float_3 %float_4
%225 = OpCompositeConstruct %mat4v3float %221 %222 %223 %224
%226 = OpCompositeExtract %v3float %219 0
%227 = OpCompositeExtract %v3float %225 0
%228 = OpFOrdEqual %v3bool %226 %227
%229 = OpAll %bool %228
%230 = OpCompositeExtract %v3float %219 1
%231 = OpCompositeExtract %v3float %225 1
%232 = OpFOrdEqual %v3bool %230 %231
%233 = OpAll %bool %232
%234 = OpLogicalAnd %bool %229 %233
%235 = OpCompositeExtract %v3float %219 2
%236 = OpCompositeExtract %v3float %225 2
%237 = OpFOrdEqual %v3bool %235 %236
%238 = OpAll %bool %237
%239 = OpLogicalAnd %bool %234 %238
%240 = OpCompositeExtract %v3float %219 3
%241 = OpCompositeExtract %v3float %225 3
%242 = OpFOrdEqual %v3bool %240 %241
%243 = OpAll %bool %242
%244 = OpLogicalAnd %bool %239 %243
OpBranch %199
%199 = OpLabel
%245 = OpPhi %bool %false %151 %244 %198
OpStore %ok %245
%246 = OpLoad %bool %ok
OpSelectionMerge %250 None
OpBranchConditional %246 %248 %249
%248 = OpLabel
%251 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%254 = OpLoad %v4float %251
OpStore %247 %254
OpBranch %250
%249 = OpLabel
%255 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%257 = OpLoad %v4float %255
OpStore %247 %257
OpBranch %250
%250 = OpLabel
%258 = OpLoad %v4float %247
OpReturnValue %258
OpFunctionEnd
