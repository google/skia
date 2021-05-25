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
OpName %intValues "intValues"
OpName %intGreen "intGreen"
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
OpDecorate %33 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
OpDecorate %220 RelaxedPrecision
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
%int = OpTypeInt 32 1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_0 = OpConstant %int 0
%float_100 = OpConstant %float 100
%int_1 = OpConstant %int 1
%int_50 = OpConstant %int 50
%int_75 = OpConstant %int 75
%int_225 = OpConstant %int 225
%63 = OpConstantComposite %v4int %int_50 %int_50 %int_75 %int_225
%int_100 = OpConstant %int 100
%66 = OpConstantComposite %v4int %int_0 %int_100 %int_75 %int_225
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%118 = OpConstantComposite %v2int %int_50 %int_50
%126 = OpConstantComposite %v3int %int_50 %int_50 %int_75
%190 = OpConstantComposite %v2int %int_0 %int_100
%198 = OpConstantComposite %v3int %int_0 %int_100 %int_75
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_2 = OpConstant %int 2
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
%intValues = OpVariable %_ptr_Function_v4int Function
%intGreen = OpVariable %_ptr_Function_v4int Function
%expectedA = OpVariable %_ptr_Function_v4int Function
%expectedB = OpVariable %_ptr_Function_v4int Function
%210 = OpVariable %_ptr_Function_v4float Function
%30 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %30
%35 = OpVectorTimesScalar %v4float %33 %float_100
%36 = OpCompositeExtract %float %35 0
%37 = OpConvertFToS %int %36
%38 = OpCompositeExtract %float %35 1
%39 = OpConvertFToS %int %38
%40 = OpCompositeExtract %float %35 2
%41 = OpConvertFToS %int %40
%42 = OpCompositeExtract %float %35 3
%43 = OpConvertFToS %int %42
%44 = OpCompositeConstruct %v4int %37 %39 %41 %43
OpStore %intValues %44
%46 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%48 = OpLoad %v4float %46
%49 = OpVectorTimesScalar %v4float %48 %float_100
%50 = OpCompositeExtract %float %49 0
%51 = OpConvertFToS %int %50
%52 = OpCompositeExtract %float %49 1
%53 = OpConvertFToS %int %52
%54 = OpCompositeExtract %float %49 2
%55 = OpConvertFToS %int %54
%56 = OpCompositeExtract %float %49 3
%57 = OpConvertFToS %int %56
%58 = OpCompositeConstruct %v4int %51 %53 %55 %57
OpStore %intGreen %58
OpStore %expectedA %63
OpStore %expectedB %66
%69 = OpLoad %v4int %intValues
%70 = OpCompositeExtract %int %69 0
%68 = OpExtInst %int %1 SMax %70 %int_50
%71 = OpLoad %v4int %expectedA
%72 = OpCompositeExtract %int %71 0
%73 = OpIEqual %bool %68 %72
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%77 = OpLoad %v4int %intValues
%78 = OpVectorShuffle %v2int %77 %77 0 1
%80 = OpCompositeConstruct %v2int %int_50 %int_50
%76 = OpExtInst %v2int %1 SMax %78 %80
%81 = OpLoad %v4int %expectedA
%82 = OpVectorShuffle %v2int %81 %81 0 1
%83 = OpIEqual %v2bool %76 %82
%85 = OpAll %bool %83
OpBranch %75
%75 = OpLabel
%86 = OpPhi %bool %false %25 %85 %74
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%90 = OpLoad %v4int %intValues
%91 = OpVectorShuffle %v3int %90 %90 0 1 2
%93 = OpCompositeConstruct %v3int %int_50 %int_50 %int_50
%89 = OpExtInst %v3int %1 SMax %91 %93
%94 = OpLoad %v4int %expectedA
%95 = OpVectorShuffle %v3int %94 %94 0 1 2
%96 = OpIEqual %v3bool %89 %95
%98 = OpAll %bool %96
OpBranch %88
%88 = OpLabel
%99 = OpPhi %bool %false %75 %98 %87
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%103 = OpLoad %v4int %intValues
%104 = OpCompositeConstruct %v4int %int_50 %int_50 %int_50 %int_50
%102 = OpExtInst %v4int %1 SMax %103 %104
%105 = OpLoad %v4int %expectedA
%106 = OpIEqual %v4bool %102 %105
%108 = OpAll %bool %106
OpBranch %101
%101 = OpLabel
%109 = OpPhi %bool %false %88 %108 %100
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%112 = OpLoad %v4int %expectedA
%113 = OpCompositeExtract %int %112 0
%114 = OpIEqual %bool %int_50 %113
OpBranch %111
%111 = OpLabel
%115 = OpPhi %bool %false %101 %114 %110
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%119 = OpLoad %v4int %expectedA
%120 = OpVectorShuffle %v2int %119 %119 0 1
%121 = OpIEqual %v2bool %118 %120
%122 = OpAll %bool %121
OpBranch %117
%117 = OpLabel
%123 = OpPhi %bool %false %111 %122 %116
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%127 = OpLoad %v4int %expectedA
%128 = OpVectorShuffle %v3int %127 %127 0 1 2
%129 = OpIEqual %v3bool %126 %128
%130 = OpAll %bool %129
OpBranch %125
%125 = OpLabel
%131 = OpPhi %bool %false %117 %130 %124
OpSelectionMerge %133 None
OpBranchConditional %131 %132 %133
%132 = OpLabel
%134 = OpLoad %v4int %expectedA
%135 = OpIEqual %v4bool %63 %134
%136 = OpAll %bool %135
OpBranch %133
%133 = OpLabel
%137 = OpPhi %bool %false %125 %136 %132
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%141 = OpLoad %v4int %intValues
%142 = OpCompositeExtract %int %141 0
%143 = OpLoad %v4int %intGreen
%144 = OpCompositeExtract %int %143 0
%140 = OpExtInst %int %1 SMax %142 %144
%145 = OpLoad %v4int %expectedB
%146 = OpCompositeExtract %int %145 0
%147 = OpIEqual %bool %140 %146
OpBranch %139
%139 = OpLabel
%148 = OpPhi %bool %false %133 %147 %138
OpSelectionMerge %150 None
OpBranchConditional %148 %149 %150
%149 = OpLabel
%152 = OpLoad %v4int %intValues
%153 = OpVectorShuffle %v2int %152 %152 0 1
%154 = OpLoad %v4int %intGreen
%155 = OpVectorShuffle %v2int %154 %154 0 1
%151 = OpExtInst %v2int %1 SMax %153 %155
%156 = OpLoad %v4int %expectedB
%157 = OpVectorShuffle %v2int %156 %156 0 1
%158 = OpIEqual %v2bool %151 %157
%159 = OpAll %bool %158
OpBranch %150
%150 = OpLabel
%160 = OpPhi %bool %false %139 %159 %149
OpSelectionMerge %162 None
OpBranchConditional %160 %161 %162
%161 = OpLabel
%164 = OpLoad %v4int %intValues
%165 = OpVectorShuffle %v3int %164 %164 0 1 2
%166 = OpLoad %v4int %intGreen
%167 = OpVectorShuffle %v3int %166 %166 0 1 2
%163 = OpExtInst %v3int %1 SMax %165 %167
%168 = OpLoad %v4int %expectedB
%169 = OpVectorShuffle %v3int %168 %168 0 1 2
%170 = OpIEqual %v3bool %163 %169
%171 = OpAll %bool %170
OpBranch %162
%162 = OpLabel
%172 = OpPhi %bool %false %150 %171 %161
OpSelectionMerge %174 None
OpBranchConditional %172 %173 %174
%173 = OpLabel
%176 = OpLoad %v4int %intValues
%177 = OpLoad %v4int %intGreen
%175 = OpExtInst %v4int %1 SMax %176 %177
%178 = OpLoad %v4int %expectedB
%179 = OpIEqual %v4bool %175 %178
%180 = OpAll %bool %179
OpBranch %174
%174 = OpLabel
%181 = OpPhi %bool %false %162 %180 %173
OpSelectionMerge %183 None
OpBranchConditional %181 %182 %183
%182 = OpLabel
%184 = OpLoad %v4int %expectedB
%185 = OpCompositeExtract %int %184 0
%186 = OpIEqual %bool %int_0 %185
OpBranch %183
%183 = OpLabel
%187 = OpPhi %bool %false %174 %186 %182
OpSelectionMerge %189 None
OpBranchConditional %187 %188 %189
%188 = OpLabel
%191 = OpLoad %v4int %expectedB
%192 = OpVectorShuffle %v2int %191 %191 0 1
%193 = OpIEqual %v2bool %190 %192
%194 = OpAll %bool %193
OpBranch %189
%189 = OpLabel
%195 = OpPhi %bool %false %183 %194 %188
OpSelectionMerge %197 None
OpBranchConditional %195 %196 %197
%196 = OpLabel
%199 = OpLoad %v4int %expectedB
%200 = OpVectorShuffle %v3int %199 %199 0 1 2
%201 = OpIEqual %v3bool %198 %200
%202 = OpAll %bool %201
OpBranch %197
%197 = OpLabel
%203 = OpPhi %bool %false %189 %202 %196
OpSelectionMerge %205 None
OpBranchConditional %203 %204 %205
%204 = OpLabel
%206 = OpLoad %v4int %expectedB
%207 = OpIEqual %v4bool %66 %206
%208 = OpAll %bool %207
OpBranch %205
%205 = OpLabel
%209 = OpPhi %bool %false %197 %208 %204
OpSelectionMerge %214 None
OpBranchConditional %209 %212 %213
%212 = OpLabel
%215 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%216 = OpLoad %v4float %215
OpStore %210 %216
OpBranch %214
%213 = OpLabel
%217 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%219 = OpLoad %v4float %217
OpStore %210 %219
OpBranch %214
%214 = OpLabel
%220 = OpLoad %v4float %210
OpReturnValue %220
OpFunctionEnd
