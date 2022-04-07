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
OpDecorate %215 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
OpDecorate %219 RelaxedPrecision
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
%80 = OpConstantComposite %v2int %int_50 %int_50
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%93 = OpConstantComposite %v3int %int_50 %int_50 %int_50
%v3bool = OpTypeVector %bool 3
%104 = OpConstantComposite %v4int %int_50 %int_50 %int_50 %int_50
%v4bool = OpTypeVector %bool 4
%125 = OpConstantComposite %v3int %int_50 %int_50 %int_75
%189 = OpConstantComposite %v2int %int_0 %int_100
%197 = OpConstantComposite %v3int %int_0 %int_100 %int_75
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
%209 = OpVariable %_ptr_Function_v4float Function
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
%118 = OpLoad %v4int %expectedA
%119 = OpVectorShuffle %v2int %118 %118 0 1
%120 = OpIEqual %v2bool %80 %119
%121 = OpAll %bool %120
OpBranch %117
%117 = OpLabel
%122 = OpPhi %bool %false %111 %121 %116
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%126 = OpLoad %v4int %expectedA
%127 = OpVectorShuffle %v3int %126 %126 0 1 2
%128 = OpIEqual %v3bool %125 %127
%129 = OpAll %bool %128
OpBranch %124
%124 = OpLabel
%130 = OpPhi %bool %false %117 %129 %123
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpLoad %v4int %expectedA
%134 = OpIEqual %v4bool %63 %133
%135 = OpAll %bool %134
OpBranch %132
%132 = OpLabel
%136 = OpPhi %bool %false %124 %135 %131
OpSelectionMerge %138 None
OpBranchConditional %136 %137 %138
%137 = OpLabel
%140 = OpLoad %v4int %intValues
%141 = OpCompositeExtract %int %140 0
%142 = OpLoad %v4int %intGreen
%143 = OpCompositeExtract %int %142 0
%139 = OpExtInst %int %1 SMax %141 %143
%144 = OpLoad %v4int %expectedB
%145 = OpCompositeExtract %int %144 0
%146 = OpIEqual %bool %139 %145
OpBranch %138
%138 = OpLabel
%147 = OpPhi %bool %false %132 %146 %137
OpSelectionMerge %149 None
OpBranchConditional %147 %148 %149
%148 = OpLabel
%151 = OpLoad %v4int %intValues
%152 = OpVectorShuffle %v2int %151 %151 0 1
%153 = OpLoad %v4int %intGreen
%154 = OpVectorShuffle %v2int %153 %153 0 1
%150 = OpExtInst %v2int %1 SMax %152 %154
%155 = OpLoad %v4int %expectedB
%156 = OpVectorShuffle %v2int %155 %155 0 1
%157 = OpIEqual %v2bool %150 %156
%158 = OpAll %bool %157
OpBranch %149
%149 = OpLabel
%159 = OpPhi %bool %false %138 %158 %148
OpSelectionMerge %161 None
OpBranchConditional %159 %160 %161
%160 = OpLabel
%163 = OpLoad %v4int %intValues
%164 = OpVectorShuffle %v3int %163 %163 0 1 2
%165 = OpLoad %v4int %intGreen
%166 = OpVectorShuffle %v3int %165 %165 0 1 2
%162 = OpExtInst %v3int %1 SMax %164 %166
%167 = OpLoad %v4int %expectedB
%168 = OpVectorShuffle %v3int %167 %167 0 1 2
%169 = OpIEqual %v3bool %162 %168
%170 = OpAll %bool %169
OpBranch %161
%161 = OpLabel
%171 = OpPhi %bool %false %149 %170 %160
OpSelectionMerge %173 None
OpBranchConditional %171 %172 %173
%172 = OpLabel
%175 = OpLoad %v4int %intValues
%176 = OpLoad %v4int %intGreen
%174 = OpExtInst %v4int %1 SMax %175 %176
%177 = OpLoad %v4int %expectedB
%178 = OpIEqual %v4bool %174 %177
%179 = OpAll %bool %178
OpBranch %173
%173 = OpLabel
%180 = OpPhi %bool %false %161 %179 %172
OpSelectionMerge %182 None
OpBranchConditional %180 %181 %182
%181 = OpLabel
%183 = OpLoad %v4int %expectedB
%184 = OpCompositeExtract %int %183 0
%185 = OpIEqual %bool %int_0 %184
OpBranch %182
%182 = OpLabel
%186 = OpPhi %bool %false %173 %185 %181
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%190 = OpLoad %v4int %expectedB
%191 = OpVectorShuffle %v2int %190 %190 0 1
%192 = OpIEqual %v2bool %189 %191
%193 = OpAll %bool %192
OpBranch %188
%188 = OpLabel
%194 = OpPhi %bool %false %182 %193 %187
OpSelectionMerge %196 None
OpBranchConditional %194 %195 %196
%195 = OpLabel
%198 = OpLoad %v4int %expectedB
%199 = OpVectorShuffle %v3int %198 %198 0 1 2
%200 = OpIEqual %v3bool %197 %199
%201 = OpAll %bool %200
OpBranch %196
%196 = OpLabel
%202 = OpPhi %bool %false %188 %201 %195
OpSelectionMerge %204 None
OpBranchConditional %202 %203 %204
%203 = OpLabel
%205 = OpLoad %v4int %expectedB
%206 = OpIEqual %v4bool %66 %205
%207 = OpAll %bool %206
OpBranch %204
%204 = OpLabel
%208 = OpPhi %bool %false %196 %207 %203
OpSelectionMerge %213 None
OpBranchConditional %208 %211 %212
%211 = OpLabel
%214 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%215 = OpLoad %v4float %214
OpStore %209 %215
OpBranch %213
%212 = OpLabel
%216 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%218 = OpLoad %v4float %216
OpStore %209 %218
OpBranch %213
%213 = OpLabel
%219 = OpLoad %v4float %209
OpReturnValue %219
OpFunctionEnd
