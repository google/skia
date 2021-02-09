OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %intValues "intValues"
OpName %intGreen "intGreen"
OpName %expectedA "expectedA"
OpName %expectedB "expectedB"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
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
OpDecorate %27 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %159 RelaxedPrecision
OpDecorate %162 RelaxedPrecision
OpDecorate %163 RelaxedPrecision
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
%18 = OpTypeFunction %v4float
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
%57 = OpConstantComposite %v4int %int_50 %int_50 %int_75 %int_225
%int_100 = OpConstant %int 100
%60 = OpConstantComposite %v4int %int_0 %int_100 %int_75 %int_225
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_2 = OpConstant %int 2
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%intValues = OpVariable %_ptr_Function_v4int Function
%intGreen = OpVariable %_ptr_Function_v4int Function
%expectedA = OpVariable %_ptr_Function_v4int Function
%expectedB = OpVariable %_ptr_Function_v4int Function
%153 = OpVariable %_ptr_Function_v4float Function
%24 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%27 = OpLoad %v4float %24
%29 = OpVectorTimesScalar %v4float %27 %float_100
%30 = OpCompositeExtract %float %29 0
%31 = OpConvertFToS %int %30
%32 = OpCompositeExtract %float %29 1
%33 = OpConvertFToS %int %32
%34 = OpCompositeExtract %float %29 2
%35 = OpConvertFToS %int %34
%36 = OpCompositeExtract %float %29 3
%37 = OpConvertFToS %int %36
%38 = OpCompositeConstruct %v4int %31 %33 %35 %37
OpStore %intValues %38
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%42 = OpLoad %v4float %40
%43 = OpVectorTimesScalar %v4float %42 %float_100
%44 = OpCompositeExtract %float %43 0
%45 = OpConvertFToS %int %44
%46 = OpCompositeExtract %float %43 1
%47 = OpConvertFToS %int %46
%48 = OpCompositeExtract %float %43 2
%49 = OpConvertFToS %int %48
%50 = OpCompositeExtract %float %43 3
%51 = OpConvertFToS %int %50
%52 = OpCompositeConstruct %v4int %45 %47 %49 %51
OpStore %intGreen %52
OpStore %expectedA %57
OpStore %expectedB %60
%63 = OpLoad %v4int %intValues
%64 = OpCompositeExtract %int %63 0
%62 = OpExtInst %int %1 SMax %64 %int_50
%65 = OpLoad %v4int %expectedA
%66 = OpCompositeExtract %int %65 0
%67 = OpIEqual %bool %62 %66
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpLoad %v4int %intValues
%72 = OpVectorShuffle %v2int %71 %71 0 1
%74 = OpCompositeConstruct %v2int %int_50 %int_50
%70 = OpExtInst %v2int %1 SMax %72 %74
%75 = OpLoad %v4int %expectedA
%76 = OpVectorShuffle %v2int %75 %75 0 1
%77 = OpIEqual %v2bool %70 %76
%79 = OpAll %bool %77
OpBranch %69
%69 = OpLabel
%80 = OpPhi %bool %false %19 %79 %68
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%84 = OpLoad %v4int %intValues
%85 = OpVectorShuffle %v3int %84 %84 0 1 2
%87 = OpCompositeConstruct %v3int %int_50 %int_50 %int_50
%83 = OpExtInst %v3int %1 SMax %85 %87
%88 = OpLoad %v4int %expectedA
%89 = OpVectorShuffle %v3int %88 %88 0 1 2
%90 = OpIEqual %v3bool %83 %89
%92 = OpAll %bool %90
OpBranch %82
%82 = OpLabel
%93 = OpPhi %bool %false %69 %92 %81
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%97 = OpLoad %v4int %intValues
%98 = OpVectorShuffle %v4int %97 %97 0 1 2 3
%99 = OpCompositeConstruct %v4int %int_50 %int_50 %int_50 %int_50
%96 = OpExtInst %v4int %1 SMax %98 %99
%100 = OpLoad %v4int %expectedA
%101 = OpVectorShuffle %v4int %100 %100 0 1 2 3
%102 = OpIEqual %v4bool %96 %101
%104 = OpAll %bool %102
OpBranch %95
%95 = OpLabel
%105 = OpPhi %bool %false %82 %104 %94
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%109 = OpLoad %v4int %intValues
%110 = OpCompositeExtract %int %109 0
%111 = OpLoad %v4int %intGreen
%112 = OpCompositeExtract %int %111 0
%108 = OpExtInst %int %1 SMax %110 %112
%113 = OpLoad %v4int %expectedB
%114 = OpCompositeExtract %int %113 0
%115 = OpIEqual %bool %108 %114
OpBranch %107
%107 = OpLabel
%116 = OpPhi %bool %false %95 %115 %106
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%120 = OpLoad %v4int %intValues
%121 = OpVectorShuffle %v2int %120 %120 0 1
%122 = OpLoad %v4int %intGreen
%123 = OpVectorShuffle %v2int %122 %122 0 1
%119 = OpExtInst %v2int %1 SMax %121 %123
%124 = OpLoad %v4int %expectedB
%125 = OpVectorShuffle %v2int %124 %124 0 1
%126 = OpIEqual %v2bool %119 %125
%127 = OpAll %bool %126
OpBranch %118
%118 = OpLabel
%128 = OpPhi %bool %false %107 %127 %117
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%132 = OpLoad %v4int %intValues
%133 = OpVectorShuffle %v3int %132 %132 0 1 2
%134 = OpLoad %v4int %intGreen
%135 = OpVectorShuffle %v3int %134 %134 0 1 2
%131 = OpExtInst %v3int %1 SMax %133 %135
%136 = OpLoad %v4int %expectedB
%137 = OpVectorShuffle %v3int %136 %136 0 1 2
%138 = OpIEqual %v3bool %131 %137
%139 = OpAll %bool %138
OpBranch %130
%130 = OpLabel
%140 = OpPhi %bool %false %118 %139 %129
OpSelectionMerge %142 None
OpBranchConditional %140 %141 %142
%141 = OpLabel
%144 = OpLoad %v4int %intValues
%145 = OpVectorShuffle %v4int %144 %144 0 1 2 3
%146 = OpLoad %v4int %intGreen
%147 = OpVectorShuffle %v4int %146 %146 0 1 2 3
%143 = OpExtInst %v4int %1 SMax %145 %147
%148 = OpLoad %v4int %expectedB
%149 = OpVectorShuffle %v4int %148 %148 0 1 2 3
%150 = OpIEqual %v4bool %143 %149
%151 = OpAll %bool %150
OpBranch %142
%142 = OpLabel
%152 = OpPhi %bool %false %130 %151 %141
OpSelectionMerge %157 None
OpBranchConditional %152 %155 %156
%155 = OpLabel
%158 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%159 = OpLoad %v4float %158
OpStore %153 %159
OpBranch %157
%156 = OpLabel
%160 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%162 = OpLoad %v4float %160
OpStore %153 %162
OpBranch %157
%157 = OpLabel
%163 = OpLoad %v4float %153
OpReturnValue %163
OpFunctionEnd
