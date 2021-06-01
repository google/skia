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
OpMemberName %_UniformBuffer 2 "colorBlack"
OpMemberName %_UniformBuffer 3 "colorWhite"
OpMemberName %_UniformBuffer 4 "testInputs"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expectedBW "expectedBW"
OpName %expectedWT "expectedWT"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 4 Offset 64
OpMemberDecorate %_UniformBuffer 4 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %expectedBW RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %expectedWT RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %109 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %122 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %129 RelaxedPrecision
OpDecorate %130 RelaxedPrecision
OpDecorate %139 RelaxedPrecision
OpDecorate %141 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %149 RelaxedPrecision
OpDecorate %150 RelaxedPrecision
OpDecorate %155 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
OpDecorate %165 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %200 RelaxedPrecision
OpDecorate %208 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %214 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %226 RelaxedPrecision
OpDecorate %227 RelaxedPrecision
OpDecorate %233 RelaxedPrecision
OpDecorate %234 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %240 RelaxedPrecision
OpDecorate %241 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %248 RelaxedPrecision
OpDecorate %249 RelaxedPrecision
OpDecorate %255 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %266 RelaxedPrecision
OpDecorate %267 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float %v4float
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
%float_0_5 = OpConstant %float 0.5
%float_1 = OpConstant %float 1
%30 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_1
%float_2_25 = OpConstant %float 2.25
%33 = OpConstantComposite %v4float %float_1 %float_0_5 %float_1 %float_2_25
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%45 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%v4bool = OpTypeVector %bool 4
%float_0_25 = OpConstant %float 0.25
%float_0_75 = OpConstant %float 0.75
%59 = OpConstantComposite %v4float %float_0_25 %float_0_75 %float_0 %float_1
%71 = OpConstantComposite %v4float %float_0_75 %float_0_25 %float_0 %float_1
%83 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%155 = OpConstantComposite %v2float %float_0_5 %float_0_5
%163 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%int_4 = OpConstant %int 4
%198 = OpConstantComposite %v2float %float_0 %float_0_5
%213 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
%226 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
%239 = OpConstantComposite %v2float %float_1 %float_0_5
%247 = OpConstantComposite %v3float %float_1 %float_0_5 %float_1
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
%expectedBW = OpVariable %_ptr_Function_v4float Function
%expectedWT = OpVariable %_ptr_Function_v4float Function
%259 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedBW %30
OpStore %expectedWT %33
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%40 = OpLoad %v4float %36
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%43 = OpLoad %v4float %41
%44 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%35 = OpExtInst %v4float %1 FMix %40 %43 %44
%46 = OpFOrdEqual %v4bool %35 %45
%48 = OpAll %bool %46
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%53 = OpLoad %v4float %52
%54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%55 = OpLoad %v4float %54
%57 = OpCompositeConstruct %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%51 = OpExtInst %v4float %1 FMix %53 %55 %57
%60 = OpFOrdEqual %v4bool %51 %59
%61 = OpAll %bool %60
OpBranch %50
%50 = OpLabel
%62 = OpPhi %bool %false %25 %61 %49
OpSelectionMerge %64 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%67 = OpLoad %v4float %66
%68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%69 = OpLoad %v4float %68
%70 = OpCompositeConstruct %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
%65 = OpExtInst %v4float %1 FMix %67 %69 %70
%72 = OpFOrdEqual %v4bool %65 %71
%73 = OpAll %bool %72
OpBranch %64
%64 = OpLabel
%74 = OpPhi %bool %false %50 %73 %63
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%79 = OpLoad %v4float %78
%80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%81 = OpLoad %v4float %80
%82 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%77 = OpExtInst %v4float %1 FMix %79 %81 %82
%84 = OpFOrdEqual %v4bool %77 %83
%85 = OpAll %bool %84
OpBranch %76
%76 = OpLabel
%86 = OpPhi %bool %false %64 %85 %75
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%90 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%92 = OpLoad %v4float %90
%93 = OpCompositeExtract %float %92 0
%94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%96 = OpLoad %v4float %94
%97 = OpCompositeExtract %float %96 0
%89 = OpExtInst %float %1 FMix %93 %97 %float_0_5
%98 = OpLoad %v4float %expectedBW
%99 = OpCompositeExtract %float %98 0
%100 = OpFOrdEqual %bool %89 %99
OpBranch %88
%88 = OpLabel
%101 = OpPhi %bool %false %76 %100 %87
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%105 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%106 = OpLoad %v4float %105
%107 = OpVectorShuffle %v2float %106 %106 0 1
%108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%109 = OpLoad %v4float %108
%110 = OpVectorShuffle %v2float %109 %109 0 1
%111 = OpCompositeConstruct %v2float %float_0_5 %float_0_5
%104 = OpExtInst %v2float %1 FMix %107 %110 %111
%112 = OpLoad %v4float %expectedBW
%113 = OpVectorShuffle %v2float %112 %112 0 1
%114 = OpFOrdEqual %v2bool %104 %113
%116 = OpAll %bool %114
OpBranch %103
%103 = OpLabel
%117 = OpPhi %bool %false %88 %116 %102
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%122 = OpLoad %v4float %121
%123 = OpVectorShuffle %v3float %122 %122 0 1 2
%125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%126 = OpLoad %v4float %125
%127 = OpVectorShuffle %v3float %126 %126 0 1 2
%128 = OpCompositeConstruct %v3float %float_0_5 %float_0_5 %float_0_5
%120 = OpExtInst %v3float %1 FMix %123 %127 %128
%129 = OpLoad %v4float %expectedBW
%130 = OpVectorShuffle %v3float %129 %129 0 1 2
%131 = OpFOrdEqual %v3bool %120 %130
%133 = OpAll %bool %131
OpBranch %119
%119 = OpLabel
%134 = OpPhi %bool %false %103 %133 %118
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%138 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%139 = OpLoad %v4float %138
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%141 = OpLoad %v4float %140
%142 = OpCompositeConstruct %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%137 = OpExtInst %v4float %1 FMix %139 %141 %142
%143 = OpLoad %v4float %expectedBW
%144 = OpFOrdEqual %v4bool %137 %143
%145 = OpAll %bool %144
OpBranch %136
%136 = OpLabel
%146 = OpPhi %bool %false %119 %145 %135
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%149 = OpLoad %v4float %expectedBW
%150 = OpCompositeExtract %float %149 0
%151 = OpFOrdEqual %bool %float_0_5 %150
OpBranch %148
%148 = OpLabel
%152 = OpPhi %bool %false %136 %151 %147
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%156 = OpLoad %v4float %expectedBW
%157 = OpVectorShuffle %v2float %156 %156 0 1
%158 = OpFOrdEqual %v2bool %155 %157
%159 = OpAll %bool %158
OpBranch %154
%154 = OpLabel
%160 = OpPhi %bool %false %148 %159 %153
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%164 = OpLoad %v4float %expectedBW
%165 = OpVectorShuffle %v3float %164 %164 0 1 2
%166 = OpFOrdEqual %v3bool %163 %165
%167 = OpAll %bool %166
OpBranch %162
%162 = OpLabel
%168 = OpPhi %bool %false %154 %167 %161
OpSelectionMerge %170 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%171 = OpLoad %v4float %expectedBW
%172 = OpFOrdEqual %v4bool %30 %171
%173 = OpAll %bool %172
OpBranch %170
%170 = OpLabel
%174 = OpPhi %bool %false %162 %173 %169
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%179 = OpLoad %v4float %178
%180 = OpCompositeExtract %float %179 0
%181 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%183 = OpLoad %v4float %181
%184 = OpCompositeExtract %float %183 0
%177 = OpExtInst %float %1 FMix %180 %184 %float_0
%185 = OpLoad %v4float %expectedWT
%186 = OpCompositeExtract %float %185 0
%187 = OpFOrdEqual %bool %177 %186
OpBranch %176
%176 = OpLabel
%188 = OpPhi %bool %false %170 %187 %175
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%192 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%193 = OpLoad %v4float %192
%194 = OpVectorShuffle %v2float %193 %193 0 1
%195 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%196 = OpLoad %v4float %195
%197 = OpVectorShuffle %v2float %196 %196 0 1
%191 = OpExtInst %v2float %1 FMix %194 %197 %198
%199 = OpLoad %v4float %expectedWT
%200 = OpVectorShuffle %v2float %199 %199 0 1
%201 = OpFOrdEqual %v2bool %191 %200
%202 = OpAll %bool %201
OpBranch %190
%190 = OpLabel
%203 = OpPhi %bool %false %176 %202 %189
OpSelectionMerge %205 None
OpBranchConditional %203 %204 %205
%204 = OpLabel
%207 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%208 = OpLoad %v4float %207
%209 = OpVectorShuffle %v3float %208 %208 0 1 2
%210 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%211 = OpLoad %v4float %210
%212 = OpVectorShuffle %v3float %211 %211 0 1 2
%206 = OpExtInst %v3float %1 FMix %209 %212 %213
%214 = OpLoad %v4float %expectedWT
%215 = OpVectorShuffle %v3float %214 %214 0 1 2
%216 = OpFOrdEqual %v3bool %206 %215
%217 = OpAll %bool %216
OpBranch %205
%205 = OpLabel
%218 = OpPhi %bool %false %190 %217 %204
OpSelectionMerge %220 None
OpBranchConditional %218 %219 %220
%219 = OpLabel
%222 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%223 = OpLoad %v4float %222
%224 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%225 = OpLoad %v4float %224
%221 = OpExtInst %v4float %1 FMix %223 %225 %226
%227 = OpLoad %v4float %expectedWT
%228 = OpFOrdEqual %v4bool %221 %227
%229 = OpAll %bool %228
OpBranch %220
%220 = OpLabel
%230 = OpPhi %bool %false %205 %229 %219
OpSelectionMerge %232 None
OpBranchConditional %230 %231 %232
%231 = OpLabel
%233 = OpLoad %v4float %expectedWT
%234 = OpCompositeExtract %float %233 0
%235 = OpFOrdEqual %bool %float_1 %234
OpBranch %232
%232 = OpLabel
%236 = OpPhi %bool %false %220 %235 %231
OpSelectionMerge %238 None
OpBranchConditional %236 %237 %238
%237 = OpLabel
%240 = OpLoad %v4float %expectedWT
%241 = OpVectorShuffle %v2float %240 %240 0 1
%242 = OpFOrdEqual %v2bool %239 %241
%243 = OpAll %bool %242
OpBranch %238
%238 = OpLabel
%244 = OpPhi %bool %false %232 %243 %237
OpSelectionMerge %246 None
OpBranchConditional %244 %245 %246
%245 = OpLabel
%248 = OpLoad %v4float %expectedWT
%249 = OpVectorShuffle %v3float %248 %248 0 1 2
%250 = OpFOrdEqual %v3bool %247 %249
%251 = OpAll %bool %250
OpBranch %246
%246 = OpLabel
%252 = OpPhi %bool %false %238 %251 %245
OpSelectionMerge %254 None
OpBranchConditional %252 %253 %254
%253 = OpLabel
%255 = OpLoad %v4float %expectedWT
%256 = OpFOrdEqual %v4bool %33 %255
%257 = OpAll %bool %256
OpBranch %254
%254 = OpLabel
%258 = OpPhi %bool %false %246 %257 %253
OpSelectionMerge %262 None
OpBranchConditional %258 %260 %261
%260 = OpLabel
%263 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%264 = OpLoad %v4float %263
OpStore %259 %264
OpBranch %262
%261 = OpLabel
%265 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%266 = OpLoad %v4float %265
OpStore %259 %266
OpBranch %262
%262 = OpLabel
%267 = OpLoad %v4float %259
OpReturnValue %267
OpFunctionEnd
