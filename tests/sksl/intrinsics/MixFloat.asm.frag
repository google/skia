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
OpDecorate %expectedWT RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %119 RelaxedPrecision
OpDecorate %120 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %148 RelaxedPrecision
OpDecorate %154 RelaxedPrecision
OpDecorate %167 RelaxedPrecision
OpDecorate %168 RelaxedPrecision
OpDecorate %171 RelaxedPrecision
OpDecorate %172 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
OpDecorate %182 RelaxedPrecision
OpDecorate %183 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %193 RelaxedPrecision
OpDecorate %194 RelaxedPrecision
OpDecorate %196 RelaxedPrecision
OpDecorate %197 RelaxedPrecision
OpDecorate %199 RelaxedPrecision
OpDecorate %207 RelaxedPrecision
OpDecorate %209 RelaxedPrecision
OpDecorate %221 RelaxedPrecision
OpDecorate %228 RelaxedPrecision
OpDecorate %242 RelaxedPrecision
OpDecorate %244 RelaxedPrecision
OpDecorate %245 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
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
%109 = OpConstantComposite %v2float %float_0_5 %float_0_5
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%125 = OpConstantComposite %v3float %float_0_5 %float_0_5 %float_0_5
%v3bool = OpTypeVector %bool 3
%138 = OpConstantComposite %v4float %float_0_5 %float_0_5 %float_0_5 %float_0_5
%int_4 = OpConstant %int 4
%184 = OpConstantComposite %v2float %float_0 %float_0_5
%198 = OpConstantComposite %v3float %float_0 %float_0_5 %float_0
%210 = OpConstantComposite %v4float %float_0 %float_0_5 %float_0 %float_1
%220 = OpConstantComposite %v2float %float_1 %float_0_5
%227 = OpConstantComposite %v3float %float_1 %float_0_5 %float_1
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
%237 = OpVariable %_ptr_Function_v4float Function
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
%98 = OpFOrdEqual %bool %89 %float_0_5
OpBranch %88
%88 = OpLabel
%99 = OpPhi %bool %false %76 %98 %87
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%103 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%104 = OpLoad %v4float %103
%105 = OpVectorShuffle %v2float %104 %104 0 1
%106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%107 = OpLoad %v4float %106
%108 = OpVectorShuffle %v2float %107 %107 0 1
%102 = OpExtInst %v2float %1 FMix %105 %108 %109
%110 = OpVectorShuffle %v2float %30 %30 0 1
%111 = OpFOrdEqual %v2bool %102 %110
%113 = OpAll %bool %111
OpBranch %101
%101 = OpLabel
%114 = OpPhi %bool %false %88 %113 %100
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%118 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%119 = OpLoad %v4float %118
%120 = OpVectorShuffle %v3float %119 %119 0 1 2
%122 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%123 = OpLoad %v4float %122
%124 = OpVectorShuffle %v3float %123 %123 0 1 2
%117 = OpExtInst %v3float %1 FMix %120 %124 %125
%126 = OpVectorShuffle %v3float %30 %30 0 1 2
%127 = OpFOrdEqual %v3bool %117 %126
%129 = OpAll %bool %127
OpBranch %116
%116 = OpLabel
%130 = OpPhi %bool %false %101 %129 %115
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%134 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%135 = OpLoad %v4float %134
%136 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%137 = OpLoad %v4float %136
%133 = OpExtInst %v4float %1 FMix %135 %137 %138
%139 = OpFOrdEqual %v4bool %133 %30
%140 = OpAll %bool %139
OpBranch %132
%132 = OpLabel
%141 = OpPhi %bool %false %116 %140 %131
OpSelectionMerge %143 None
OpBranchConditional %141 %142 %143
%142 = OpLabel
%144 = OpFOrdEqual %bool %float_0_5 %float_0_5
OpBranch %143
%143 = OpLabel
%145 = OpPhi %bool %false %132 %144 %142
OpSelectionMerge %147 None
OpBranchConditional %145 %146 %147
%146 = OpLabel
%148 = OpVectorShuffle %v2float %30 %30 0 1
%149 = OpFOrdEqual %v2bool %109 %148
%150 = OpAll %bool %149
OpBranch %147
%147 = OpLabel
%151 = OpPhi %bool %false %143 %150 %146
OpSelectionMerge %153 None
OpBranchConditional %151 %152 %153
%152 = OpLabel
%154 = OpVectorShuffle %v3float %30 %30 0 1 2
%155 = OpFOrdEqual %v3bool %125 %154
%156 = OpAll %bool %155
OpBranch %153
%153 = OpLabel
%157 = OpPhi %bool %false %147 %156 %152
OpSelectionMerge %159 None
OpBranchConditional %157 %158 %159
%158 = OpLabel
%160 = OpFOrdEqual %v4bool %30 %30
%161 = OpAll %bool %160
OpBranch %159
%159 = OpLabel
%162 = OpPhi %bool %false %153 %161 %158
OpSelectionMerge %164 None
OpBranchConditional %162 %163 %164
%163 = OpLabel
%166 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%167 = OpLoad %v4float %166
%168 = OpCompositeExtract %float %167 0
%169 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%171 = OpLoad %v4float %169
%172 = OpCompositeExtract %float %171 0
%165 = OpExtInst %float %1 FMix %168 %172 %float_0
%173 = OpFOrdEqual %bool %165 %float_1
OpBranch %164
%164 = OpLabel
%174 = OpPhi %bool %false %159 %173 %163
OpSelectionMerge %176 None
OpBranchConditional %174 %175 %176
%175 = OpLabel
%178 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%179 = OpLoad %v4float %178
%180 = OpVectorShuffle %v2float %179 %179 0 1
%181 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%182 = OpLoad %v4float %181
%183 = OpVectorShuffle %v2float %182 %182 0 1
%177 = OpExtInst %v2float %1 FMix %180 %183 %184
%185 = OpVectorShuffle %v2float %33 %33 0 1
%186 = OpFOrdEqual %v2bool %177 %185
%187 = OpAll %bool %186
OpBranch %176
%176 = OpLabel
%188 = OpPhi %bool %false %164 %187 %175
OpSelectionMerge %190 None
OpBranchConditional %188 %189 %190
%189 = OpLabel
%192 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%193 = OpLoad %v4float %192
%194 = OpVectorShuffle %v3float %193 %193 0 1 2
%195 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%196 = OpLoad %v4float %195
%197 = OpVectorShuffle %v3float %196 %196 0 1 2
%191 = OpExtInst %v3float %1 FMix %194 %197 %198
%199 = OpVectorShuffle %v3float %33 %33 0 1 2
%200 = OpFOrdEqual %v3bool %191 %199
%201 = OpAll %bool %200
OpBranch %190
%190 = OpLabel
%202 = OpPhi %bool %false %176 %201 %189
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%206 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%207 = OpLoad %v4float %206
%208 = OpAccessChain %_ptr_Uniform_v4float %10 %int_4
%209 = OpLoad %v4float %208
%205 = OpExtInst %v4float %1 FMix %207 %209 %210
%211 = OpFOrdEqual %v4bool %205 %33
%212 = OpAll %bool %211
OpBranch %204
%204 = OpLabel
%213 = OpPhi %bool %false %190 %212 %203
OpSelectionMerge %215 None
OpBranchConditional %213 %214 %215
%214 = OpLabel
%216 = OpFOrdEqual %bool %float_1 %float_1
OpBranch %215
%215 = OpLabel
%217 = OpPhi %bool %false %204 %216 %214
OpSelectionMerge %219 None
OpBranchConditional %217 %218 %219
%218 = OpLabel
%221 = OpVectorShuffle %v2float %33 %33 0 1
%222 = OpFOrdEqual %v2bool %220 %221
%223 = OpAll %bool %222
OpBranch %219
%219 = OpLabel
%224 = OpPhi %bool %false %215 %223 %218
OpSelectionMerge %226 None
OpBranchConditional %224 %225 %226
%225 = OpLabel
%228 = OpVectorShuffle %v3float %33 %33 0 1 2
%229 = OpFOrdEqual %v3bool %227 %228
%230 = OpAll %bool %229
OpBranch %226
%226 = OpLabel
%231 = OpPhi %bool %false %219 %230 %225
OpSelectionMerge %233 None
OpBranchConditional %231 %232 %233
%232 = OpLabel
%234 = OpFOrdEqual %v4bool %33 %33
%235 = OpAll %bool %234
OpBranch %233
%233 = OpLabel
%236 = OpPhi %bool %false %226 %235 %232
OpSelectionMerge %240 None
OpBranchConditional %236 %238 %239
%238 = OpLabel
%241 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%242 = OpLoad %v4float %241
OpStore %237 %242
OpBranch %240
%239 = OpLabel
%243 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%244 = OpLoad %v4float %243
OpStore %237 %244
OpBranch %240
%240 = OpLabel
%245 = OpLoad %v4float %237
OpReturnValue %245
OpFunctionEnd
