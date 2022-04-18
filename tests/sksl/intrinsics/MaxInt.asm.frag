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
OpDecorate %182 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
OpDecorate %186 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
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
%76 = OpConstantComposite %v2int %int_50 %int_50
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%87 = OpConstantComposite %v3int %int_50 %int_50 %int_50
%v3bool = OpTypeVector %bool 3
%96 = OpConstantComposite %v4int %int_50 %int_50 %int_50 %int_50
%v4bool = OpTypeVector %bool 4
%113 = OpConstantComposite %v3int %int_50 %int_50 %int_75
%159 = OpConstantComposite %v2int %int_0 %int_100
%166 = OpConstantComposite %v3int %int_0 %int_100 %int_75
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
%176 = OpVariable %_ptr_Function_v4float Function
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
%69 = OpCompositeExtract %int %44 0
%68 = OpExtInst %int %1 SMax %69 %int_50
%70 = OpIEqual %bool %68 %int_50
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%74 = OpVectorShuffle %v2int %44 %44 0 1
%73 = OpExtInst %v2int %1 SMax %74 %76
%77 = OpVectorShuffle %v2int %63 %63 0 1
%78 = OpIEqual %v2bool %73 %77
%80 = OpAll %bool %78
OpBranch %72
%72 = OpLabel
%81 = OpPhi %bool %false %25 %80 %71
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%85 = OpVectorShuffle %v3int %44 %44 0 1 2
%84 = OpExtInst %v3int %1 SMax %85 %87
%88 = OpVectorShuffle %v3int %63 %63 0 1 2
%89 = OpIEqual %v3bool %84 %88
%91 = OpAll %bool %89
OpBranch %83
%83 = OpLabel
%92 = OpPhi %bool %false %72 %91 %82
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%95 = OpExtInst %v4int %1 SMax %44 %96
%97 = OpIEqual %v4bool %95 %63
%99 = OpAll %bool %97
OpBranch %94
%94 = OpLabel
%100 = OpPhi %bool %false %83 %99 %93
OpSelectionMerge %102 None
OpBranchConditional %100 %101 %102
%101 = OpLabel
%103 = OpIEqual %bool %int_50 %int_50
OpBranch %102
%102 = OpLabel
%104 = OpPhi %bool %false %94 %103 %101
OpSelectionMerge %106 None
OpBranchConditional %104 %105 %106
%105 = OpLabel
%107 = OpVectorShuffle %v2int %63 %63 0 1
%108 = OpIEqual %v2bool %76 %107
%109 = OpAll %bool %108
OpBranch %106
%106 = OpLabel
%110 = OpPhi %bool %false %102 %109 %105
OpSelectionMerge %112 None
OpBranchConditional %110 %111 %112
%111 = OpLabel
%114 = OpVectorShuffle %v3int %63 %63 0 1 2
%115 = OpIEqual %v3bool %113 %114
%116 = OpAll %bool %115
OpBranch %112
%112 = OpLabel
%117 = OpPhi %bool %false %106 %116 %111
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%120 = OpIEqual %v4bool %63 %63
%121 = OpAll %bool %120
OpBranch %119
%119 = OpLabel
%122 = OpPhi %bool %false %112 %121 %118
OpSelectionMerge %124 None
OpBranchConditional %122 %123 %124
%123 = OpLabel
%126 = OpCompositeExtract %int %58 0
%125 = OpExtInst %int %1 SMax %69 %126
%127 = OpIEqual %bool %125 %int_0
OpBranch %124
%124 = OpLabel
%128 = OpPhi %bool %false %119 %127 %123
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%132 = OpVectorShuffle %v2int %44 %44 0 1
%133 = OpVectorShuffle %v2int %58 %58 0 1
%131 = OpExtInst %v2int %1 SMax %132 %133
%134 = OpVectorShuffle %v2int %66 %66 0 1
%135 = OpIEqual %v2bool %131 %134
%136 = OpAll %bool %135
OpBranch %130
%130 = OpLabel
%137 = OpPhi %bool %false %124 %136 %129
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%141 = OpVectorShuffle %v3int %44 %44 0 1 2
%142 = OpVectorShuffle %v3int %58 %58 0 1 2
%140 = OpExtInst %v3int %1 SMax %141 %142
%143 = OpVectorShuffle %v3int %66 %66 0 1 2
%144 = OpIEqual %v3bool %140 %143
%145 = OpAll %bool %144
OpBranch %139
%139 = OpLabel
%146 = OpPhi %bool %false %130 %145 %138
OpSelectionMerge %148 None
OpBranchConditional %146 %147 %148
%147 = OpLabel
%149 = OpExtInst %v4int %1 SMax %44 %58
%150 = OpIEqual %v4bool %149 %66
%151 = OpAll %bool %150
OpBranch %148
%148 = OpLabel
%152 = OpPhi %bool %false %139 %151 %147
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpIEqual %bool %int_0 %int_0
OpBranch %154
%154 = OpLabel
%156 = OpPhi %bool %false %148 %155 %153
OpSelectionMerge %158 None
OpBranchConditional %156 %157 %158
%157 = OpLabel
%160 = OpVectorShuffle %v2int %66 %66 0 1
%161 = OpIEqual %v2bool %159 %160
%162 = OpAll %bool %161
OpBranch %158
%158 = OpLabel
%163 = OpPhi %bool %false %154 %162 %157
OpSelectionMerge %165 None
OpBranchConditional %163 %164 %165
%164 = OpLabel
%167 = OpVectorShuffle %v3int %66 %66 0 1 2
%168 = OpIEqual %v3bool %166 %167
%169 = OpAll %bool %168
OpBranch %165
%165 = OpLabel
%170 = OpPhi %bool %false %158 %169 %164
OpSelectionMerge %172 None
OpBranchConditional %170 %171 %172
%171 = OpLabel
%173 = OpIEqual %v4bool %66 %66
%174 = OpAll %bool %173
OpBranch %172
%172 = OpLabel
%175 = OpPhi %bool %false %165 %174 %171
OpSelectionMerge %180 None
OpBranchConditional %175 %178 %179
%178 = OpLabel
%181 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%182 = OpLoad %v4float %181
OpStore %176 %182
OpBranch %180
%179 = OpLabel
%183 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%185 = OpLoad %v4float %183
OpStore %176 %185
OpBranch %180
%180 = OpLabel
%186 = OpLoad %v4float %176
OpReturnValue %186
OpFunctionEnd
