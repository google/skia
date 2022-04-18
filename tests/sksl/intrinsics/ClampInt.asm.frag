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
OpDecorate %181 RelaxedPrecision
OpDecorate %184 RelaxedPrecision
OpDecorate %185 RelaxedPrecision
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
%70 = OpConstantComposite %v2int %int_n100 %int_n100
%71 = OpConstantComposite %v2int %int_100 %int_100
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%82 = OpConstantComposite %v3int %int_n100 %int_n100 %int_n100
%83 = OpConstantComposite %v3int %int_100 %int_100 %int_100
%v3bool = OpTypeVector %bool 3
%92 = OpConstantComposite %v4int %int_n100 %int_n100 %int_n100 %int_n100
%93 = OpConstantComposite %v4int %int_100 %int_100 %int_100 %int_100
%v4bool = OpTypeVector %bool 4
%104 = OpConstantComposite %v2int %int_n100 %int_0
%111 = OpConstantComposite %v3int %int_n100 %int_0 %int_75
%130 = OpConstantComposite %v2int %int_n100 %int_n200
%131 = OpConstantComposite %v2int %int_100 %int_200
%140 = OpConstantComposite %v3int %int_n100 %int_n200 %int_n200
%141 = OpConstantComposite %v3int %int_100 %int_200 %int_50
%164 = OpConstantComposite %v3int %int_n100 %int_0 %int_50
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
%174 = OpVariable %_ptr_Function_v4float Function
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
%63 = OpCompositeExtract %int %44 0
%62 = OpExtInst %int %1 SClamp %63 %int_n100 %int_100
%64 = OpIEqual %bool %62 %int_n100
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%68 = OpVectorShuffle %v2int %44 %44 0 1
%67 = OpExtInst %v2int %1 SClamp %68 %70 %71
%72 = OpVectorShuffle %v2int %49 %49 0 1
%73 = OpIEqual %v2bool %67 %72
%75 = OpAll %bool %73
OpBranch %66
%66 = OpLabel
%76 = OpPhi %bool %false %25 %75 %65
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%80 = OpVectorShuffle %v3int %44 %44 0 1 2
%79 = OpExtInst %v3int %1 SClamp %80 %82 %83
%84 = OpVectorShuffle %v3int %49 %49 0 1 2
%85 = OpIEqual %v3bool %79 %84
%87 = OpAll %bool %85
OpBranch %78
%78 = OpLabel
%88 = OpPhi %bool %false %66 %87 %77
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%91 = OpExtInst %v4int %1 SClamp %44 %92 %93
%94 = OpIEqual %v4bool %91 %49
%96 = OpAll %bool %94
OpBranch %90
%90 = OpLabel
%97 = OpPhi %bool %false %78 %96 %89
OpSelectionMerge %99 None
OpBranchConditional %97 %98 %99
%98 = OpLabel
%100 = OpIEqual %bool %int_n100 %int_n100
OpBranch %99
%99 = OpLabel
%101 = OpPhi %bool %false %90 %100 %98
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%105 = OpVectorShuffle %v2int %49 %49 0 1
%106 = OpIEqual %v2bool %104 %105
%107 = OpAll %bool %106
OpBranch %103
%103 = OpLabel
%108 = OpPhi %bool %false %99 %107 %102
OpSelectionMerge %110 None
OpBranchConditional %108 %109 %110
%109 = OpLabel
%112 = OpVectorShuffle %v3int %49 %49 0 1 2
%113 = OpIEqual %v3bool %111 %112
%114 = OpAll %bool %113
OpBranch %110
%110 = OpLabel
%115 = OpPhi %bool %false %103 %114 %109
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%118 = OpIEqual %v4bool %49 %49
%119 = OpAll %bool %118
OpBranch %117
%117 = OpLabel
%120 = OpPhi %bool %false %110 %119 %116
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%123 = OpExtInst %int %1 SClamp %63 %int_n100 %int_100
%124 = OpIEqual %bool %123 %int_n100
OpBranch %122
%122 = OpLabel
%125 = OpPhi %bool %false %117 %124 %121
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%129 = OpVectorShuffle %v2int %44 %44 0 1
%128 = OpExtInst %v2int %1 SClamp %129 %130 %131
%132 = OpVectorShuffle %v2int %56 %56 0 1
%133 = OpIEqual %v2bool %128 %132
%134 = OpAll %bool %133
OpBranch %127
%127 = OpLabel
%135 = OpPhi %bool %false %122 %134 %126
OpSelectionMerge %137 None
OpBranchConditional %135 %136 %137
%136 = OpLabel
%139 = OpVectorShuffle %v3int %44 %44 0 1 2
%138 = OpExtInst %v3int %1 SClamp %139 %140 %141
%142 = OpVectorShuffle %v3int %56 %56 0 1 2
%143 = OpIEqual %v3bool %138 %142
%144 = OpAll %bool %143
OpBranch %137
%137 = OpLabel
%145 = OpPhi %bool %false %127 %144 %136
OpSelectionMerge %147 None
OpBranchConditional %145 %146 %147
%146 = OpLabel
%148 = OpExtInst %v4int %1 SClamp %44 %52 %60
%149 = OpIEqual %v4bool %148 %56
%150 = OpAll %bool %149
OpBranch %147
%147 = OpLabel
%151 = OpPhi %bool %false %137 %150 %146
OpSelectionMerge %153 None
OpBranchConditional %151 %152 %153
%152 = OpLabel
%154 = OpIEqual %bool %int_n100 %int_n100
OpBranch %153
%153 = OpLabel
%155 = OpPhi %bool %false %147 %154 %152
OpSelectionMerge %157 None
OpBranchConditional %155 %156 %157
%156 = OpLabel
%158 = OpVectorShuffle %v2int %56 %56 0 1
%159 = OpIEqual %v2bool %104 %158
%160 = OpAll %bool %159
OpBranch %157
%157 = OpLabel
%161 = OpPhi %bool %false %153 %160 %156
OpSelectionMerge %163 None
OpBranchConditional %161 %162 %163
%162 = OpLabel
%165 = OpVectorShuffle %v3int %56 %56 0 1 2
%166 = OpIEqual %v3bool %164 %165
%167 = OpAll %bool %166
OpBranch %163
%163 = OpLabel
%168 = OpPhi %bool %false %157 %167 %162
OpSelectionMerge %170 None
OpBranchConditional %168 %169 %170
%169 = OpLabel
%171 = OpIEqual %v4bool %56 %56
%172 = OpAll %bool %171
OpBranch %170
%170 = OpLabel
%173 = OpPhi %bool %false %163 %172 %169
OpSelectionMerge %178 None
OpBranchConditional %173 %176 %177
%176 = OpLabel
%179 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%181 = OpLoad %v4float %179
OpStore %174 %181
OpBranch %178
%177 = OpLabel
%182 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%184 = OpLoad %v4float %182
OpStore %174 %184
OpBranch %178
%178 = OpLabel
%185 = OpLoad %v4float %174
OpReturnValue %185
OpFunctionEnd
