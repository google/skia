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
OpDecorate %176 RelaxedPrecision
OpDecorate %179 RelaxedPrecision
OpDecorate %180 RelaxedPrecision
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
%true = OpConstantTrue %bool
%104 = OpConstantComposite %v2int %int_n100 %int_0
%111 = OpConstantComposite %v3int %int_n100 %int_0 %int_75
%128 = OpConstantComposite %v2int %int_n100 %int_n200
%129 = OpConstantComposite %v2int %int_100 %int_200
%138 = OpConstantComposite %v3int %int_n100 %int_n200 %int_n200
%139 = OpConstantComposite %v3int %int_100 %int_200 %int_50
%161 = OpConstantComposite %v3int %int_n100 %int_0 %int_50
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
%169 = OpVariable %_ptr_Function_v4float Function
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
OpBranch %99
%99 = OpLabel
%101 = OpPhi %bool %false %90 %true %98
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
OpBranch %117
%117 = OpLabel
%118 = OpPhi %bool %false %110 %true %116
OpSelectionMerge %120 None
OpBranchConditional %118 %119 %120
%119 = OpLabel
%121 = OpExtInst %int %1 SClamp %63 %int_n100 %int_100
%122 = OpIEqual %bool %121 %int_n100
OpBranch %120
%120 = OpLabel
%123 = OpPhi %bool %false %117 %122 %119
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%127 = OpVectorShuffle %v2int %44 %44 0 1
%126 = OpExtInst %v2int %1 SClamp %127 %128 %129
%130 = OpVectorShuffle %v2int %56 %56 0 1
%131 = OpIEqual %v2bool %126 %130
%132 = OpAll %bool %131
OpBranch %125
%125 = OpLabel
%133 = OpPhi %bool %false %120 %132 %124
OpSelectionMerge %135 None
OpBranchConditional %133 %134 %135
%134 = OpLabel
%137 = OpVectorShuffle %v3int %44 %44 0 1 2
%136 = OpExtInst %v3int %1 SClamp %137 %138 %139
%140 = OpVectorShuffle %v3int %56 %56 0 1 2
%141 = OpIEqual %v3bool %136 %140
%142 = OpAll %bool %141
OpBranch %135
%135 = OpLabel
%143 = OpPhi %bool %false %125 %142 %134
OpSelectionMerge %145 None
OpBranchConditional %143 %144 %145
%144 = OpLabel
%146 = OpExtInst %v4int %1 SClamp %44 %52 %60
%147 = OpIEqual %v4bool %146 %56
%148 = OpAll %bool %147
OpBranch %145
%145 = OpLabel
%149 = OpPhi %bool %false %135 %148 %144
OpSelectionMerge %151 None
OpBranchConditional %149 %150 %151
%150 = OpLabel
OpBranch %151
%151 = OpLabel
%152 = OpPhi %bool %false %145 %true %150
OpSelectionMerge %154 None
OpBranchConditional %152 %153 %154
%153 = OpLabel
%155 = OpVectorShuffle %v2int %56 %56 0 1
%156 = OpIEqual %v2bool %104 %155
%157 = OpAll %bool %156
OpBranch %154
%154 = OpLabel
%158 = OpPhi %bool %false %151 %157 %153
OpSelectionMerge %160 None
OpBranchConditional %158 %159 %160
%159 = OpLabel
%162 = OpVectorShuffle %v3int %56 %56 0 1 2
%163 = OpIEqual %v3bool %161 %162
%164 = OpAll %bool %163
OpBranch %160
%160 = OpLabel
%165 = OpPhi %bool %false %154 %164 %159
OpSelectionMerge %167 None
OpBranchConditional %165 %166 %167
%166 = OpLabel
OpBranch %167
%167 = OpLabel
%168 = OpPhi %bool %false %160 %true %166
OpSelectionMerge %173 None
OpBranchConditional %168 %171 %172
%171 = OpLabel
%174 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%176 = OpLoad %v4float %174
OpStore %169 %176
OpBranch %173
%172 = OpLabel
%177 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%179 = OpLoad %v4float %177
OpStore %169 %179
OpBranch %173
%173 = OpLabel
%180 = OpLoad %v4float %169
OpReturnValue %180
OpFunctionEnd
