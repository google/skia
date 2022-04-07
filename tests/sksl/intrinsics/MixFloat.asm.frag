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
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %169 RelaxedPrecision
OpDecorate %177 RelaxedPrecision
OpDecorate %178 RelaxedPrecision
OpDecorate %181 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %191 RelaxedPrecision
OpDecorate %192 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %195 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %198 RelaxedPrecision
OpDecorate %206 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %210 RelaxedPrecision
OpDecorate %211 RelaxedPrecision
OpDecorate %212 RelaxedPrecision
OpDecorate %213 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %223 RelaxedPrecision
OpDecorate %224 RelaxedPrecision
OpDecorate %225 RelaxedPrecision
OpDecorate %231 RelaxedPrecision
OpDecorate %232 RelaxedPrecision
OpDecorate %237 RelaxedPrecision
OpDecorate %238 RelaxedPrecision
OpDecorate %239 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
OpDecorate %246 RelaxedPrecision
OpDecorate %247 RelaxedPrecision
OpDecorate %253 RelaxedPrecision
OpDecorate %262 RelaxedPrecision
OpDecorate %264 RelaxedPrecision
OpDecorate %265 RelaxedPrecision
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
%44 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%45 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_1
%v4bool = OpTypeVector %bool 4
%float_0_25 = OpConstant %float 0.25
%57 = OpConstantComposite %v4float %float_0_25 %float_0_25 %float_0_25 %float_0_25
%float_0_75 = OpConstant %float 0.75
%59 = OpConstantComposite %v4float %float_0_25 %float_0_75 %float_0 %float_1
%70 = OpConstantComposite %v4float %float_0_75 %float_0_75 %float_0_75 %float_0_75
%71 = OpConstantComposite %v4float %float_0_75 %float_0_25 %float_0 %float_1
%82 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%83 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_1
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
%111 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%128 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%142 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%int_4 = OpConstant %int 4
%196 = OpConstantComposite %v2float %float_0 %float_0_5
%211 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
%224 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
%237 = OpConstantComposite %v2float %float_1 %float_0_5
%245 = OpConstantComposite %v3float %float_1 %float_0_5 %float_1
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
%257 = OpVariable %_ptr_Function_v4float Function
OpStore %expectedBW %30
OpStore %expectedWT %33
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%40 = OpLoad %v4float %36
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%43 = OpLoad %v4float %41
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
%155 = OpLoad %v4float %expectedBW
%156 = OpVectorShuffle %v2float %155 %155 0 1
%157 = OpFOrdEqual %v2bool %111 %156
%158 = OpAll %bool %157
OpBranch %154
%154 = OpLabel
%159 = OpPhi %bool %false %148 %158 %153
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%162 = OpLoad %v4float %expectedBW
%163 = OpVectorShuffle %v3float %162 %162 0 1 2
%164 = OpFOrdEqual %v3bool %128 %163
%165 = OpAll %bool %164
OpBranch %161
%161 = OpLabel
%166 = OpPhi %bool %false %154 %165 %160
OpSelectionMerge %168 None
OpBranchConditional %166 %167 %168
%167 = OpLabel
%169 = OpLoad %v4float %expectedBW
%170 = OpFOrdEqual %v4bool %30 %169
%171 = OpAll %bool %170
OpBranch %168
%168 = OpLabel
%172 = OpPhi %bool %false %161 %171 %167
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%176 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%177 = OpLoad %v4float %176
%178 = OpCompositeExtract %float %177 0
%179 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%181 = OpLoad %v4float %179
%182 = OpCompositeExtract %float %181 0
%175 = OpExtInst %float %1 FMix %178 %182 %float_0
%183 = OpLoad %v4float %expectedWT
%184 = OpCompositeExtract %float %183 0
%185 = OpFOrdEqual %bool %175 %184
OpBranch %174
%174 = OpLabel
%186 = OpPhi %bool %false %168 %185 %173
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%190 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%191 = OpLoad %v4float %190
%192 = OpVectorShuffle %v2float %191 %191 0 1
%193 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%194 = OpLoad %v4float %193
%195 = OpVectorShuffle %v2float %194 %194 0 1
%189 = OpExtInst %v2float %1 FMix %192 %195 %196
%197 = OpLoad %v4float %expectedWT
%198 = OpVectorShuffle %v2float %197 %197 0 1
%199 = OpFOrdEqual %v2bool %189 %198
%200 = OpAll %bool %199
OpBranch %188
%188 = OpLabel
%201 = OpPhi %bool %false %174 %200 %187
OpSelectionMerge %203 None
OpBranchConditional %201 %202 %203
%202 = OpLabel
%205 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%206 = OpLoad %v4float %205
%207 = OpVectorShuffle %v3float %206 %206 0 1 2
%208 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%209 = OpLoad %v4float %208
%210 = OpVectorShuffle %v3float %209 %209 0 1 2
%204 = OpExtInst %v3float %1 FMix %207 %210 %211
%212 = OpLoad %v4float %expectedWT
%213 = OpVectorShuffle %v3float %212 %212 0 1 2
%214 = OpFOrdEqual %v3bool %204 %213
%215 = OpAll %bool %214
OpBranch %203
%203 = OpLabel
%216 = OpPhi %bool %false %188 %215 %202
OpSelectionMerge %218 None
OpBranchConditional %216 %217 %218
%217 = OpLabel
%220 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%221 = OpLoad %v4float %220
%222 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%223 = OpLoad %v4float %222
%219 = OpExtInst %v4float %1 FMix %221 %223 %224
%225 = OpLoad %v4float %expectedWT
%226 = OpFOrdEqual %v4bool %219 %225
%227 = OpAll %bool %226
OpBranch %218
%218 = OpLabel
%228 = OpPhi %bool %false %203 %227 %217
OpSelectionMerge %230 None
OpBranchConditional %228 %229 %230
%229 = OpLabel
%231 = OpLoad %v4float %expectedWT
%232 = OpCompositeExtract %float %231 0
%233 = OpFOrdEqual %bool %float_1 %232
OpBranch %230
%230 = OpLabel
%234 = OpPhi %bool %false %218 %233 %229
OpSelectionMerge %236 None
OpBranchConditional %234 %235 %236
%235 = OpLabel
%238 = OpLoad %v4float %expectedWT
%239 = OpVectorShuffle %v2float %238 %238 0 1
%240 = OpFOrdEqual %v2bool %237 %239
%241 = OpAll %bool %240
OpBranch %236
%236 = OpLabel
%242 = OpPhi %bool %false %230 %241 %235
OpSelectionMerge %244 None
OpBranchConditional %242 %243 %244
%243 = OpLabel
%246 = OpLoad %v4float %expectedWT
%247 = OpVectorShuffle %v3float %246 %246 0 1 2
%248 = OpFOrdEqual %v3bool %245 %247
%249 = OpAll %bool %248
OpBranch %244
%244 = OpLabel
%250 = OpPhi %bool %false %236 %249 %243
OpSelectionMerge %252 None
OpBranchConditional %250 %251 %252
%251 = OpLabel
%253 = OpLoad %v4float %expectedWT
%254 = OpFOrdEqual %v4bool %33 %253
%255 = OpAll %bool %254
OpBranch %252
%252 = OpLabel
%256 = OpPhi %bool %false %244 %255 %251
OpSelectionMerge %260 None
OpBranchConditional %256 %258 %259
%258 = OpLabel
%261 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%262 = OpLoad %v4float %261
OpStore %257 %262
OpBranch %260
%259 = OpLabel
%263 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%264 = OpLoad %v4float %263
OpStore %257 %264
OpBranch %260
%260 = OpLabel
%265 = OpLoad %v4float %257
OpReturnValue %265
OpFunctionEnd
