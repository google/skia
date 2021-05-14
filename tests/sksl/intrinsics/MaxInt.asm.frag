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
OpDecorate %160 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
OpDecorate %164 RelaxedPrecision
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
%154 = OpVariable %_ptr_Function_v4float Function
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
%113 = OpLoad %v4int %intValues
%114 = OpCompositeExtract %int %113 0
%115 = OpLoad %v4int %intGreen
%116 = OpCompositeExtract %int %115 0
%112 = OpExtInst %int %1 SMax %114 %116
%117 = OpLoad %v4int %expectedB
%118 = OpCompositeExtract %int %117 0
%119 = OpIEqual %bool %112 %118
OpBranch %111
%111 = OpLabel
%120 = OpPhi %bool %false %101 %119 %110
OpSelectionMerge %122 None
OpBranchConditional %120 %121 %122
%121 = OpLabel
%124 = OpLoad %v4int %intValues
%125 = OpVectorShuffle %v2int %124 %124 0 1
%126 = OpLoad %v4int %intGreen
%127 = OpVectorShuffle %v2int %126 %126 0 1
%123 = OpExtInst %v2int %1 SMax %125 %127
%128 = OpLoad %v4int %expectedB
%129 = OpVectorShuffle %v2int %128 %128 0 1
%130 = OpIEqual %v2bool %123 %129
%131 = OpAll %bool %130
OpBranch %122
%122 = OpLabel
%132 = OpPhi %bool %false %111 %131 %121
OpSelectionMerge %134 None
OpBranchConditional %132 %133 %134
%133 = OpLabel
%136 = OpLoad %v4int %intValues
%137 = OpVectorShuffle %v3int %136 %136 0 1 2
%138 = OpLoad %v4int %intGreen
%139 = OpVectorShuffle %v3int %138 %138 0 1 2
%135 = OpExtInst %v3int %1 SMax %137 %139
%140 = OpLoad %v4int %expectedB
%141 = OpVectorShuffle %v3int %140 %140 0 1 2
%142 = OpIEqual %v3bool %135 %141
%143 = OpAll %bool %142
OpBranch %134
%134 = OpLabel
%144 = OpPhi %bool %false %122 %143 %133
OpSelectionMerge %146 None
OpBranchConditional %144 %145 %146
%145 = OpLabel
%148 = OpLoad %v4int %intValues
%149 = OpLoad %v4int %intGreen
%147 = OpExtInst %v4int %1 SMax %148 %149
%150 = OpLoad %v4int %expectedB
%151 = OpIEqual %v4bool %147 %150
%152 = OpAll %bool %151
OpBranch %146
%146 = OpLabel
%153 = OpPhi %bool %false %134 %152 %145
OpSelectionMerge %158 None
OpBranchConditional %153 %156 %157
%156 = OpLabel
%159 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%160 = OpLoad %v4float %159
OpStore %154 %160
OpBranch %158
%157 = OpLabel
%161 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%163 = OpLoad %v4float %161
OpStore %154 %163
OpBranch %158
%158 = OpLabel
%164 = OpLoad %v4float %154
OpReturnValue %164
OpFunctionEnd
