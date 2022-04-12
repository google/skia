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
OpDecorate %106 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %250 RelaxedPrecision
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
%59 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%60 = OpConstantComposite %v3float %float_4 %float_1 %float_2
%v3bool = OpTypeVector %bool 3
%false = OpConstantFalse %bool
%mat2v4float = OpTypeMatrix %v4float 2
%93 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%v4bool = OpTypeVector %bool 4
%mat3v3float = OpTypeMatrix %v3float 3
%129 = OpConstantComposite %v3float %float_3 %float_4 %float_1
%mat4v2float = OpTypeMatrix %v2float 4
%168 = OpConstantComposite %v2float %float_1 %float_2
%169 = OpConstantComposite %v2float %float_3 %float_4
%v2bool = OpTypeVector %bool 2
%mat4v3float = OpTypeMatrix %v3float 4
%216 = OpConstantComposite %v3float %float_2 %float_3 %float_4
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
%239 = OpVariable %_ptr_Function_v4float Function
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
%48 = OpCompositeConstruct %v3float %44 %45 %46
%49 = OpCompositeExtract %float %41 3
%50 = OpCompositeExtract %float %43 0
%51 = OpCompositeExtract %float %43 1
%52 = OpCompositeConstruct %v3float %49 %50 %51
%54 = OpCompositeConstruct %mat2v3float %48 %52
%61 = OpCompositeConstruct %mat2v3float %59 %60
%63 = OpCompositeExtract %v3float %54 0
%64 = OpCompositeExtract %v3float %61 0
%65 = OpFOrdEqual %v3bool %63 %64
%66 = OpAll %bool %65
%67 = OpCompositeExtract %v3float %54 1
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
%92 = OpCompositeConstruct %mat2v4float %86 %90
%94 = OpCompositeConstruct %mat2v4float %93 %93
%96 = OpCompositeExtract %v4float %92 0
%97 = OpCompositeExtract %v4float %94 0
%98 = OpFOrdEqual %v4bool %96 %97
%99 = OpAll %bool %98
%100 = OpCompositeExtract %v4float %92 1
%101 = OpCompositeExtract %v4float %94 1
%102 = OpFOrdEqual %v4bool %100 %101
%103 = OpAll %bool %102
%104 = OpLogicalAnd %bool %99 %103
OpBranch %75
%75 = OpLabel
%105 = OpPhi %bool %false %26 %104 %74
OpStore %ok %105
%106 = OpLoad %bool %ok
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpLoad %v4float %f4
%110 = OpVectorShuffle %v2float %109 %109 0 1
%111 = OpLoad %v4float %f4
%112 = OpVectorShuffle %v2float %111 %111 2 3
%113 = OpLoad %v4float %f4
%114 = OpLoad %v4float %f4
%115 = OpCompositeExtract %float %114 0
%116 = OpCompositeExtract %float %110 0
%117 = OpCompositeExtract %float %110 1
%118 = OpCompositeExtract %float %112 0
%119 = OpCompositeConstruct %v3float %116 %117 %118
%120 = OpCompositeExtract %float %112 1
%121 = OpCompositeExtract %float %113 0
%122 = OpCompositeExtract %float %113 1
%123 = OpCompositeConstruct %v3float %120 %121 %122
%124 = OpCompositeExtract %float %113 2
%125 = OpCompositeExtract %float %113 3
%126 = OpCompositeConstruct %v3float %124 %125 %115
%128 = OpCompositeConstruct %mat3v3float %119 %123 %126
%130 = OpCompositeConstruct %mat3v3float %59 %60 %129
%131 = OpCompositeExtract %v3float %128 0
%132 = OpCompositeExtract %v3float %130 0
%133 = OpFOrdEqual %v3bool %131 %132
%134 = OpAll %bool %133
%135 = OpCompositeExtract %v3float %128 1
%136 = OpCompositeExtract %v3float %130 1
%137 = OpFOrdEqual %v3bool %135 %136
%138 = OpAll %bool %137
%139 = OpLogicalAnd %bool %134 %138
%140 = OpCompositeExtract %v3float %128 2
%141 = OpCompositeExtract %v3float %130 2
%142 = OpFOrdEqual %v3bool %140 %141
%143 = OpAll %bool %142
%144 = OpLogicalAnd %bool %139 %143
OpBranch %108
%108 = OpLabel
%145 = OpPhi %bool %false %75 %144 %107
OpStore %ok %145
%146 = OpLoad %bool %ok
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%149 = OpLoad %v4float %f4
%150 = OpVectorShuffle %v3float %149 %149 0 1 2
%151 = OpLoad %v4float %f4
%152 = OpVectorShuffle %v4float %151 %151 3 0 1 2
%153 = OpLoad %v4float %f4
%154 = OpCompositeExtract %float %153 3
%155 = OpCompositeExtract %float %150 0
%156 = OpCompositeExtract %float %150 1
%157 = OpCompositeConstruct %v2float %155 %156
%158 = OpCompositeExtract %float %150 2
%159 = OpCompositeExtract %float %152 0
%160 = OpCompositeConstruct %v2float %158 %159
%161 = OpCompositeExtract %float %152 1
%162 = OpCompositeExtract %float %152 2
%163 = OpCompositeConstruct %v2float %161 %162
%164 = OpCompositeExtract %float %152 3
%165 = OpCompositeConstruct %v2float %164 %154
%167 = OpCompositeConstruct %mat4v2float %157 %160 %163 %165
%170 = OpCompositeConstruct %mat4v2float %168 %169 %168 %169
%172 = OpCompositeExtract %v2float %167 0
%173 = OpCompositeExtract %v2float %170 0
%174 = OpFOrdEqual %v2bool %172 %173
%175 = OpAll %bool %174
%176 = OpCompositeExtract %v2float %167 1
%177 = OpCompositeExtract %v2float %170 1
%178 = OpFOrdEqual %v2bool %176 %177
%179 = OpAll %bool %178
%180 = OpLogicalAnd %bool %175 %179
%181 = OpCompositeExtract %v2float %167 2
%182 = OpCompositeExtract %v2float %170 2
%183 = OpFOrdEqual %v2bool %181 %182
%184 = OpAll %bool %183
%185 = OpLogicalAnd %bool %180 %184
%186 = OpCompositeExtract %v2float %167 3
%187 = OpCompositeExtract %v2float %170 3
%188 = OpFOrdEqual %v2bool %186 %187
%189 = OpAll %bool %188
%190 = OpLogicalAnd %bool %185 %189
OpBranch %148
%148 = OpLabel
%191 = OpPhi %bool %false %108 %190 %147
OpStore %ok %191
%192 = OpLoad %bool %ok
OpSelectionMerge %194 None
OpBranchConditional %192 %193 %194
%193 = OpLabel
%195 = OpLoad %v4float %f4
%196 = OpCompositeExtract %float %195 0
%197 = OpLoad %v4float %f4
%198 = OpVectorShuffle %v4float %197 %197 1 2 3 0
%199 = OpLoad %v4float %f4
%200 = OpVectorShuffle %v4float %199 %199 1 2 3 0
%201 = OpLoad %v4float %f4
%202 = OpVectorShuffle %v3float %201 %201 1 2 3
%203 = OpCompositeExtract %float %198 0
%204 = OpCompositeExtract %float %198 1
%205 = OpCompositeConstruct %v3float %196 %203 %204
%206 = OpCompositeExtract %float %198 2
%207 = OpCompositeExtract %float %198 3
%208 = OpCompositeExtract %float %200 0
%209 = OpCompositeConstruct %v3float %206 %207 %208
%210 = OpCompositeExtract %float %200 1
%211 = OpCompositeExtract %float %200 2
%212 = OpCompositeExtract %float %200 3
%213 = OpCompositeConstruct %v3float %210 %211 %212
%215 = OpCompositeConstruct %mat4v3float %205 %209 %213 %202
%217 = OpCompositeConstruct %mat4v3float %59 %60 %129 %216
%218 = OpCompositeExtract %v3float %215 0
%219 = OpCompositeExtract %v3float %217 0
%220 = OpFOrdEqual %v3bool %218 %219
%221 = OpAll %bool %220
%222 = OpCompositeExtract %v3float %215 1
%223 = OpCompositeExtract %v3float %217 1
%224 = OpFOrdEqual %v3bool %222 %223
%225 = OpAll %bool %224
%226 = OpLogicalAnd %bool %221 %225
%227 = OpCompositeExtract %v3float %215 2
%228 = OpCompositeExtract %v3float %217 2
%229 = OpFOrdEqual %v3bool %227 %228
%230 = OpAll %bool %229
%231 = OpLogicalAnd %bool %226 %230
%232 = OpCompositeExtract %v3float %215 3
%233 = OpCompositeExtract %v3float %217 3
%234 = OpFOrdEqual %v3bool %232 %233
%235 = OpAll %bool %234
%236 = OpLogicalAnd %bool %231 %235
OpBranch %194
%194 = OpLabel
%237 = OpPhi %bool %false %148 %236 %193
OpStore %ok %237
%238 = OpLoad %bool %ok
OpSelectionMerge %242 None
OpBranchConditional %238 %240 %241
%240 = OpLabel
%243 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%246 = OpLoad %v4float %243
OpStore %239 %246
OpBranch %242
%241 = OpLabel
%247 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%249 = OpLoad %v4float %247
OpStore %239 %249
OpBranch %242
%242 = OpLabel
%250 = OpLoad %v4float %239
OpReturnValue %250
OpFunctionEnd
