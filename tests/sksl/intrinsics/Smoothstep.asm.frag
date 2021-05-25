OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %constVal "constVal"
OpName %expectedA "expectedA"
OpName %expectedB "expectedB"
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
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %constVal RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %expectedA RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %expectedB RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
OpDecorate %133 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %136 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %140 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %152 RelaxedPrecision
OpDecorate %153 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %170 RelaxedPrecision
OpDecorate %176 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %205 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
OpDecorate %222 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %235 RelaxedPrecision
OpDecorate %236 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1_25 = OpConstant %float -1.25
%float_0_75 = OpConstant %float 0.75
%float_2_25 = OpConstant %float 2.25
%31 = OpConstantComposite %v4float %float_n1_25 %float_0 %float_0_75 %float_2_25
%float_0_84375 = OpConstant %float 0.84375
%float_1 = OpConstant %float 1
%35 = OpConstantComposite %v4float %float_0 %float_0 %float_0_84375 %float_1
%37 = OpConstantComposite %v4float %float_1 %float_0 %float_1 %float_1
%false = OpConstantFalse %bool
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%53 = OpConstantComposite %v3float %float_0 %float_0 %float_0_84375
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%int_1 = OpConstant %int 1
%121 = OpConstantComposite %v2float %float_n1_25 %float_0
%138 = OpConstantComposite %v3float %float_n1_25 %float_0 %float_0_75
%168 = OpConstantComposite %v2float %float_1 %float_0
%176 = OpConstantComposite %v3float %float_1 %float_0 %float_1
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%constVal = OpVariable %_ptr_Function_v4float Function
%expectedA = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4float Function
%241 = OpVariable %_ptr_Function_v4float Function
OpStore %constVal %31
OpStore %expectedA %35
OpStore %expectedB %37
%39 = OpLoad %v4float %expectedA
%40 = OpCompositeExtract %float %39 0
%41 = OpFOrdEqual %bool %float_0 %40
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%44 = OpLoad %v4float %expectedA
%45 = OpVectorShuffle %v2float %44 %44 0 1
%46 = OpFOrdEqual %v2bool %19 %45
%48 = OpAll %bool %46
OpBranch %43
%43 = OpLabel
%49 = OpPhi %bool %false %25 %48 %42
OpSelectionMerge %51 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
%54 = OpLoad %v4float %expectedA
%55 = OpVectorShuffle %v3float %54 %54 0 1 2
%56 = OpFOrdEqual %v3bool %53 %55
%58 = OpAll %bool %56
OpBranch %51
%51 = OpLabel
%59 = OpPhi %bool %false %43 %58 %50
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%62 = OpLoad %v4float %expectedA
%63 = OpFOrdEqual %v4bool %35 %62
%65 = OpAll %bool %63
OpBranch %61
%61 = OpLabel
%66 = OpPhi %bool %false %51 %65 %60
OpSelectionMerge %68 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%69 = OpLoad %v4float %expectedA
%70 = OpCompositeExtract %float %69 0
%71 = OpFOrdEqual %bool %float_0 %70
OpBranch %68
%68 = OpLabel
%72 = OpPhi %bool %false %61 %71 %67
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%75 = OpLoad %v4float %expectedA
%76 = OpVectorShuffle %v2float %75 %75 0 1
%77 = OpFOrdEqual %v2bool %19 %76
%78 = OpAll %bool %77
OpBranch %74
%74 = OpLabel
%79 = OpPhi %bool %false %68 %78 %73
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%82 = OpLoad %v4float %expectedA
%83 = OpVectorShuffle %v3float %82 %82 0 1 2
%84 = OpFOrdEqual %v3bool %53 %83
%85 = OpAll %bool %84
OpBranch %81
%81 = OpLabel
%86 = OpPhi %bool %false %74 %85 %80
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%89 = OpLoad %v4float %expectedA
%90 = OpFOrdEqual %v4bool %35 %89
%91 = OpAll %bool %90
OpBranch %88
%88 = OpLabel
%92 = OpPhi %bool %false %81 %91 %87
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%100 = OpLoad %v4float %96
%101 = OpCompositeExtract %float %100 1
%102 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%104 = OpLoad %v4float %102
%105 = OpCompositeExtract %float %104 1
%95 = OpExtInst %float %1 SmoothStep %101 %105 %float_n1_25
%106 = OpLoad %v4float %expectedA
%107 = OpCompositeExtract %float %106 0
%108 = OpFOrdEqual %bool %95 %107
OpBranch %94
%94 = OpLabel
%109 = OpPhi %bool %false %88 %108 %93
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%114 = OpLoad %v4float %113
%115 = OpCompositeExtract %float %114 1
%116 = OpCompositeConstruct %v2float %115 %115
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%118 = OpLoad %v4float %117
%119 = OpCompositeExtract %float %118 1
%120 = OpCompositeConstruct %v2float %119 %119
%112 = OpExtInst %v2float %1 SmoothStep %116 %120 %121
%122 = OpLoad %v4float %expectedA
%123 = OpVectorShuffle %v2float %122 %122 0 1
%124 = OpFOrdEqual %v2bool %112 %123
%125 = OpAll %bool %124
OpBranch %111
%111 = OpLabel
%126 = OpPhi %bool %false %94 %125 %110
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%130 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%131 = OpLoad %v4float %130
%132 = OpCompositeExtract %float %131 1
%133 = OpCompositeConstruct %v3float %132 %132 %132
%134 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%135 = OpLoad %v4float %134
%136 = OpCompositeExtract %float %135 1
%137 = OpCompositeConstruct %v3float %136 %136 %136
%129 = OpExtInst %v3float %1 SmoothStep %133 %137 %138
%139 = OpLoad %v4float %expectedA
%140 = OpVectorShuffle %v3float %139 %139 0 1 2
%141 = OpFOrdEqual %v3bool %129 %140
%142 = OpAll %bool %141
OpBranch %128
%128 = OpLabel
%143 = OpPhi %bool %false %111 %142 %127
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%147 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%148 = OpLoad %v4float %147
%149 = OpCompositeExtract %float %148 1
%150 = OpCompositeConstruct %v4float %149 %149 %149 %149
%151 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%152 = OpLoad %v4float %151
%153 = OpCompositeExtract %float %152 1
%154 = OpCompositeConstruct %v4float %153 %153 %153 %153
%155 = OpLoad %v4float %constVal
%146 = OpExtInst %v4float %1 SmoothStep %150 %154 %155
%156 = OpLoad %v4float %expectedA
%157 = OpFOrdEqual %v4bool %146 %156
%158 = OpAll %bool %157
OpBranch %145
%145 = OpLabel
%159 = OpPhi %bool %false %128 %158 %144
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%162 = OpLoad %v4float %expectedB
%163 = OpCompositeExtract %float %162 0
%164 = OpFOrdEqual %bool %float_1 %163
OpBranch %161
%161 = OpLabel
%165 = OpPhi %bool %false %145 %164 %160
OpSelectionMerge %167 None
OpBranchConditional %165 %166 %167
%166 = OpLabel
%169 = OpLoad %v4float %expectedB
%170 = OpVectorShuffle %v2float %169 %169 0 1
%171 = OpFOrdEqual %v2bool %168 %170
%172 = OpAll %bool %171
OpBranch %167
%167 = OpLabel
%173 = OpPhi %bool %false %161 %172 %166
OpSelectionMerge %175 None
OpBranchConditional %173 %174 %175
%174 = OpLabel
%177 = OpLoad %v4float %expectedB
%178 = OpVectorShuffle %v3float %177 %177 0 1 2
%179 = OpFOrdEqual %v3bool %176 %178
%180 = OpAll %bool %179
OpBranch %175
%175 = OpLabel
%181 = OpPhi %bool %false %167 %180 %174
OpSelectionMerge %183 None
OpBranchConditional %181 %182 %183
%182 = OpLabel
%184 = OpLoad %v4float %expectedB
%185 = OpFOrdEqual %v4bool %37 %184
%186 = OpAll %bool %185
OpBranch %183
%183 = OpLabel
%187 = OpPhi %bool %false %175 %186 %182
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
%191 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%192 = OpLoad %v4float %191
%193 = OpCompositeExtract %float %192 0
%194 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%195 = OpLoad %v4float %194
%196 = OpCompositeExtract %float %195 0
%190 = OpExtInst %float %1 SmoothStep %193 %196 %float_n1_25
%197 = OpLoad %v4float %expectedB
%198 = OpCompositeExtract %float %197 0
%199 = OpFOrdEqual %bool %190 %198
OpBranch %189
%189 = OpLabel
%200 = OpPhi %bool %false %183 %199 %188
OpSelectionMerge %202 None
OpBranchConditional %200 %201 %202
%201 = OpLabel
%204 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%205 = OpLoad %v4float %204
%206 = OpVectorShuffle %v2float %205 %205 0 1
%207 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%208 = OpLoad %v4float %207
%209 = OpVectorShuffle %v2float %208 %208 0 1
%203 = OpExtInst %v2float %1 SmoothStep %206 %209 %121
%210 = OpLoad %v4float %expectedB
%211 = OpVectorShuffle %v2float %210 %210 0 1
%212 = OpFOrdEqual %v2bool %203 %211
%213 = OpAll %bool %212
OpBranch %202
%202 = OpLabel
%214 = OpPhi %bool %false %189 %213 %201
OpSelectionMerge %216 None
OpBranchConditional %214 %215 %216
%215 = OpLabel
%218 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%219 = OpLoad %v4float %218
%220 = OpVectorShuffle %v3float %219 %219 0 1 2
%221 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%222 = OpLoad %v4float %221
%223 = OpVectorShuffle %v3float %222 %222 0 1 2
%217 = OpExtInst %v3float %1 SmoothStep %220 %223 %138
%224 = OpLoad %v4float %expectedB
%225 = OpVectorShuffle %v3float %224 %224 0 1 2
%226 = OpFOrdEqual %v3bool %217 %225
%227 = OpAll %bool %226
OpBranch %216
%216 = OpLabel
%228 = OpPhi %bool %false %202 %227 %215
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%232 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%233 = OpLoad %v4float %232
%234 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%235 = OpLoad %v4float %234
%236 = OpLoad %v4float %constVal
%231 = OpExtInst %v4float %1 SmoothStep %233 %235 %236
%237 = OpLoad %v4float %expectedB
%238 = OpFOrdEqual %v4bool %231 %237
%239 = OpAll %bool %238
OpBranch %230
%230 = OpLabel
%240 = OpPhi %bool %false %216 %239 %229
OpSelectionMerge %244 None
OpBranchConditional %240 %242 %243
%242 = OpLabel
%245 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%246 = OpLoad %v4float %245
OpStore %241 %246
OpBranch %244
%243 = OpLabel
%247 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%248 = OpLoad %v4float %247
OpStore %241 %248
OpBranch %244
%244 = OpLabel
%249 = OpLoad %v4float %241
OpReturnValue %249
OpFunctionEnd
