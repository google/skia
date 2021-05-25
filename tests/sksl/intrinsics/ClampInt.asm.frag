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
OpName %expectedA "expectedA"
OpName %clampLow "clampLow"
OpName %expectedB "expectedB"
OpName %clampHigh "clampHigh"
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
OpDecorate %212 RelaxedPrecision
OpDecorate %215 RelaxedPrecision
OpDecorate %216 RelaxedPrecision
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
%int_n100 = OpConstant %int -100
%int_75 = OpConstant %int 75
%int_100 = OpConstant %int 100
%49 = OpConstantComposite %v4int %int_n100 %int_0 %int_75 %int_100
%int_n200 = OpConstant %int -200
%52 = OpConstantComposite %v4int %int_n100 %int_n200 %int_n200 %int_100
%int_50 = OpConstant %int 50
%int_225 = OpConstant %int 225
%56 = OpConstantComposite %v4int %int_n100 %int_0 %int_50 %int_225
%int_200 = OpConstant %int 200
%int_300 = OpConstant %int 300
%60 = OpConstantComposite %v4int %int_100 %int_200 %int_50 %int_300
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%115 = OpConstantComposite %v2int %int_n100 %int_0
%123 = OpConstantComposite %v3int %int_n100 %int_0 %int_75
%149 = OpConstantComposite %v2int %int_n100 %int_n200
%150 = OpConstantComposite %v2int %int_100 %int_200
%161 = OpConstantComposite %v3int %int_n100 %int_n200 %int_n200
%162 = OpConstantComposite %v3int %int_100 %int_200 %int_50
%193 = OpConstantComposite %v3int %int_n100 %int_0 %int_50
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
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
%expectedA = OpVariable %_ptr_Function_v4int Function
%clampLow = OpVariable %_ptr_Function_v4int Function
%expectedB = OpVariable %_ptr_Function_v4int Function
%clampHigh = OpVariable %_ptr_Function_v4int Function
%205 = OpVariable %_ptr_Function_v4float Function
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
OpStore %expectedA %49
OpStore %clampLow %52
OpStore %expectedB %56
OpStore %clampHigh %60
%63 = OpLoad %v4int %intValues
%64 = OpCompositeExtract %int %63 0
%62 = OpExtInst %int %1 SClamp %64 %int_n100 %int_100
%65 = OpLoad %v4int %expectedA
%66 = OpCompositeExtract %int %65 0
%67 = OpIEqual %bool %62 %66
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpLoad %v4int %intValues
%72 = OpVectorShuffle %v2int %71 %71 0 1
%74 = OpCompositeConstruct %v2int %int_n100 %int_n100
%75 = OpCompositeConstruct %v2int %int_100 %int_100
%70 = OpExtInst %v2int %1 SClamp %72 %74 %75
%76 = OpLoad %v4int %expectedA
%77 = OpVectorShuffle %v2int %76 %76 0 1
%78 = OpIEqual %v2bool %70 %77
%80 = OpAll %bool %78
OpBranch %69
%69 = OpLabel
%81 = OpPhi %bool %false %25 %80 %68
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%85 = OpLoad %v4int %intValues
%86 = OpVectorShuffle %v3int %85 %85 0 1 2
%88 = OpCompositeConstruct %v3int %int_n100 %int_n100 %int_n100
%89 = OpCompositeConstruct %v3int %int_100 %int_100 %int_100
%84 = OpExtInst %v3int %1 SClamp %86 %88 %89
%90 = OpLoad %v4int %expectedA
%91 = OpVectorShuffle %v3int %90 %90 0 1 2
%92 = OpIEqual %v3bool %84 %91
%94 = OpAll %bool %92
OpBranch %83
%83 = OpLabel
%95 = OpPhi %bool %false %69 %94 %82
OpSelectionMerge %97 None
OpBranchConditional %95 %96 %97
%96 = OpLabel
%99 = OpLoad %v4int %intValues
%100 = OpCompositeConstruct %v4int %int_n100 %int_n100 %int_n100 %int_n100
%101 = OpCompositeConstruct %v4int %int_100 %int_100 %int_100 %int_100
%98 = OpExtInst %v4int %1 SClamp %99 %100 %101
%102 = OpLoad %v4int %expectedA
%103 = OpIEqual %v4bool %98 %102
%105 = OpAll %bool %103
OpBranch %97
%97 = OpLabel
%106 = OpPhi %bool %false %83 %105 %96
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%109 = OpLoad %v4int %expectedA
%110 = OpCompositeExtract %int %109 0
%111 = OpIEqual %bool %int_n100 %110
OpBranch %108
%108 = OpLabel
%112 = OpPhi %bool %false %97 %111 %107
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%116 = OpLoad %v4int %expectedA
%117 = OpVectorShuffle %v2int %116 %116 0 1
%118 = OpIEqual %v2bool %115 %117
%119 = OpAll %bool %118
OpBranch %114
%114 = OpLabel
%120 = OpPhi %bool %false %108 %119 %113
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%124 = OpLoad %v4int %expectedA
%125 = OpVectorShuffle %v3int %124 %124 0 1 2
%126 = OpIEqual %v3bool %123 %125
%127 = OpAll %bool %126
OpBranch %122
%122 = OpLabel
%128 = OpPhi %bool %false %114 %127 %121
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%131 = OpLoad %v4int %expectedA
%132 = OpIEqual %v4bool %49 %131
%133 = OpAll %bool %132
OpBranch %130
%130 = OpLabel
%134 = OpPhi %bool %false %122 %133 %129
OpSelectionMerge %136 None
OpBranchConditional %134 %135 %136
%135 = OpLabel
%138 = OpLoad %v4int %intValues
%139 = OpCompositeExtract %int %138 0
%137 = OpExtInst %int %1 SClamp %139 %int_n100 %int_100
%140 = OpLoad %v4int %expectedB
%141 = OpCompositeExtract %int %140 0
%142 = OpIEqual %bool %137 %141
OpBranch %136
%136 = OpLabel
%143 = OpPhi %bool %false %130 %142 %135
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%147 = OpLoad %v4int %intValues
%148 = OpVectorShuffle %v2int %147 %147 0 1
%146 = OpExtInst %v2int %1 SClamp %148 %149 %150
%151 = OpLoad %v4int %expectedB
%152 = OpVectorShuffle %v2int %151 %151 0 1
%153 = OpIEqual %v2bool %146 %152
%154 = OpAll %bool %153
OpBranch %145
%145 = OpLabel
%155 = OpPhi %bool %false %136 %154 %144
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%159 = OpLoad %v4int %intValues
%160 = OpVectorShuffle %v3int %159 %159 0 1 2
%158 = OpExtInst %v3int %1 SClamp %160 %161 %162
%163 = OpLoad %v4int %expectedB
%164 = OpVectorShuffle %v3int %163 %163 0 1 2
%165 = OpIEqual %v3bool %158 %164
%166 = OpAll %bool %165
OpBranch %157
%157 = OpLabel
%167 = OpPhi %bool %false %145 %166 %156
OpSelectionMerge %169 None
OpBranchConditional %167 %168 %169
%168 = OpLabel
%171 = OpLoad %v4int %intValues
%172 = OpLoad %v4int %clampLow
%173 = OpLoad %v4int %clampHigh
%170 = OpExtInst %v4int %1 SClamp %171 %172 %173
%174 = OpLoad %v4int %expectedB
%175 = OpIEqual %v4bool %170 %174
%176 = OpAll %bool %175
OpBranch %169
%169 = OpLabel
%177 = OpPhi %bool %false %157 %176 %168
OpSelectionMerge %179 None
OpBranchConditional %177 %178 %179
%178 = OpLabel
%180 = OpLoad %v4int %expectedB
%181 = OpCompositeExtract %int %180 0
%182 = OpIEqual %bool %int_n100 %181
OpBranch %179
%179 = OpLabel
%183 = OpPhi %bool %false %169 %182 %178
OpSelectionMerge %185 None
OpBranchConditional %183 %184 %185
%184 = OpLabel
%186 = OpLoad %v4int %expectedB
%187 = OpVectorShuffle %v2int %186 %186 0 1
%188 = OpIEqual %v2bool %115 %187
%189 = OpAll %bool %188
OpBranch %185
%185 = OpLabel
%190 = OpPhi %bool %false %179 %189 %184
OpSelectionMerge %192 None
OpBranchConditional %190 %191 %192
%191 = OpLabel
%194 = OpLoad %v4int %expectedB
%195 = OpVectorShuffle %v3int %194 %194 0 1 2
%196 = OpIEqual %v3bool %193 %195
%197 = OpAll %bool %196
OpBranch %192
%192 = OpLabel
%198 = OpPhi %bool %false %185 %197 %191
OpSelectionMerge %200 None
OpBranchConditional %198 %199 %200
%199 = OpLabel
%201 = OpLoad %v4int %expectedB
%202 = OpIEqual %v4bool %56 %201
%203 = OpAll %bool %202
OpBranch %200
%200 = OpLabel
%204 = OpPhi %bool %false %192 %203 %199
OpSelectionMerge %209 None
OpBranchConditional %204 %207 %208
%207 = OpLabel
%210 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%212 = OpLoad %v4float %210
OpStore %205 %212
OpBranch %209
%208 = OpLabel
%213 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%215 = OpLoad %v4float %213
OpStore %205 %215
OpBranch %209
%209 = OpLabel
%216 = OpLoad %v4float %205
OpReturnValue %216
OpFunctionEnd
