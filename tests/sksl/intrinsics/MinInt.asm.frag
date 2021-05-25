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
OpDecorate %214 RelaxedPrecision
OpDecorate %217 RelaxedPrecision
OpDecorate %218 RelaxedPrecision
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
%int_n125 = OpConstant %int -125
%int_50 = OpConstant %int 50
%62 = OpConstantComposite %v4int %int_n125 %int_0 %int_50 %int_50
%int_100 = OpConstant %int 100
%65 = OpConstantComposite %v4int %int_n125 %int_0 %int_0 %int_100
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%117 = OpConstantComposite %v2int %int_n125 %int_0
%125 = OpConstantComposite %v3int %int_n125 %int_0 %int_50
%196 = OpConstantComposite %v3int %int_n125 %int_0 %int_0
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
%208 = OpVariable %_ptr_Function_v4float Function
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
OpStore %expectedA %62
OpStore %expectedB %65
%68 = OpLoad %v4int %intValues
%69 = OpCompositeExtract %int %68 0
%67 = OpExtInst %int %1 SMin %69 %int_50
%70 = OpLoad %v4int %expectedA
%71 = OpCompositeExtract %int %70 0
%72 = OpIEqual %bool %67 %71
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpLoad %v4int %intValues
%77 = OpVectorShuffle %v2int %76 %76 0 1
%79 = OpCompositeConstruct %v2int %int_50 %int_50
%75 = OpExtInst %v2int %1 SMin %77 %79
%80 = OpLoad %v4int %expectedA
%81 = OpVectorShuffle %v2int %80 %80 0 1
%82 = OpIEqual %v2bool %75 %81
%84 = OpAll %bool %82
OpBranch %74
%74 = OpLabel
%85 = OpPhi %bool %false %25 %84 %73
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%89 = OpLoad %v4int %intValues
%90 = OpVectorShuffle %v3int %89 %89 0 1 2
%92 = OpCompositeConstruct %v3int %int_50 %int_50 %int_50
%88 = OpExtInst %v3int %1 SMin %90 %92
%93 = OpLoad %v4int %expectedA
%94 = OpVectorShuffle %v3int %93 %93 0 1 2
%95 = OpIEqual %v3bool %88 %94
%97 = OpAll %bool %95
OpBranch %87
%87 = OpLabel
%98 = OpPhi %bool %false %74 %97 %86
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%102 = OpLoad %v4int %intValues
%103 = OpCompositeConstruct %v4int %int_50 %int_50 %int_50 %int_50
%101 = OpExtInst %v4int %1 SMin %102 %103
%104 = OpLoad %v4int %expectedA
%105 = OpIEqual %v4bool %101 %104
%107 = OpAll %bool %105
OpBranch %100
%100 = OpLabel
%108 = OpPhi %bool %false %87 %107 %99
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%111 = OpLoad %v4int %expectedA
%112 = OpCompositeExtract %int %111 0
%113 = OpIEqual %bool %int_n125 %112
OpBranch %110
%110 = OpLabel
%114 = OpPhi %bool %false %100 %113 %109
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%118 = OpLoad %v4int %expectedA
%119 = OpVectorShuffle %v2int %118 %118 0 1
%120 = OpIEqual %v2bool %117 %119
%121 = OpAll %bool %120
OpBranch %116
%116 = OpLabel
%122 = OpPhi %bool %false %110 %121 %115
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%126 = OpLoad %v4int %expectedA
%127 = OpVectorShuffle %v3int %126 %126 0 1 2
%128 = OpIEqual %v3bool %125 %127
%129 = OpAll %bool %128
OpBranch %124
%124 = OpLabel
%130 = OpPhi %bool %false %116 %129 %123
OpSelectionMerge %132 None
OpBranchConditional %130 %131 %132
%131 = OpLabel
%133 = OpLoad %v4int %expectedA
%134 = OpIEqual %v4bool %62 %133
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
%139 = OpExtInst %int %1 SMin %141 %143
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
%150 = OpExtInst %v2int %1 SMin %152 %154
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
%162 = OpExtInst %v3int %1 SMin %164 %166
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
%174 = OpExtInst %v4int %1 SMin %175 %176
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
%185 = OpIEqual %bool %int_n125 %184
OpBranch %182
%182 = OpLabel
%186 = OpPhi %bool %false %173 %185 %181
OpSelectionMerge %188 None
OpBranchConditional %186 %187 %188
%187 = OpLabel
%189 = OpLoad %v4int %expectedB
%190 = OpVectorShuffle %v2int %189 %189 0 1
%191 = OpIEqual %v2bool %117 %190
%192 = OpAll %bool %191
OpBranch %188
%188 = OpLabel
%193 = OpPhi %bool %false %182 %192 %187
OpSelectionMerge %195 None
OpBranchConditional %193 %194 %195
%194 = OpLabel
%197 = OpLoad %v4int %expectedB
%198 = OpVectorShuffle %v3int %197 %197 0 1 2
%199 = OpIEqual %v3bool %196 %198
%200 = OpAll %bool %199
OpBranch %195
%195 = OpLabel
%201 = OpPhi %bool %false %188 %200 %194
OpSelectionMerge %203 None
OpBranchConditional %201 %202 %203
%202 = OpLabel
%204 = OpLoad %v4int %expectedB
%205 = OpIEqual %v4bool %65 %204
%206 = OpAll %bool %205
OpBranch %203
%203 = OpLabel
%207 = OpPhi %bool %false %195 %206 %202
OpSelectionMerge %212 None
OpBranchConditional %207 %210 %211
%210 = OpLabel
%213 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%214 = OpLoad %v4float %213
OpStore %208 %214
OpBranch %212
%211 = OpLabel
%215 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%217 = OpLoad %v4float %215
OpStore %208 %217
OpBranch %212
%212 = OpLabel
%218 = OpLoad %v4float %208
OpReturnValue %218
OpFunctionEnd
