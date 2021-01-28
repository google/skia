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
OpDecorate %142 RelaxedPrecision
OpDecorate %145 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
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
%false = OpConstantFalse %bool
%int_50 = OpConstant %int 50
%v2int = OpTypeVector %int 2
%66 = OpConstantComposite %v2int %int_50 %int_50
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%int_75 = OpConstant %int 75
%79 = OpConstantComposite %v3int %int_50 %int_50 %int_75
%v3bool = OpTypeVector %bool 3
%int_225 = OpConstant %int 225
%90 = OpConstantComposite %v4int %int_50 %int_50 %int_75 %int_225
%v4bool = OpTypeVector %bool 4
%int_100 = OpConstant %int 100
%112 = OpConstantComposite %v2int %int_0 %int_100
%123 = OpConstantComposite %v3int %int_0 %int_100 %int_75
%132 = OpConstantComposite %v4int %int_0 %int_100 %int_75 %int_225
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
%136 = OpVariable %_ptr_Function_v4float Function
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
%55 = OpLoad %v4int %intValues
%56 = OpCompositeExtract %int %55 0
%54 = OpExtInst %int %1 SMax %56 %int_50
%58 = OpIEqual %bool %54 %int_50
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%62 = OpLoad %v4int %intValues
%63 = OpVectorShuffle %v2int %62 %62 0 1
%65 = OpCompositeConstruct %v2int %int_50 %int_50
%61 = OpExtInst %v2int %1 SMax %63 %65
%67 = OpIEqual %v2bool %61 %66
%69 = OpAll %bool %67
OpBranch %60
%60 = OpLabel
%70 = OpPhi %bool %false %19 %69 %59
OpSelectionMerge %72 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
%74 = OpLoad %v4int %intValues
%75 = OpVectorShuffle %v3int %74 %74 0 1 2
%77 = OpCompositeConstruct %v3int %int_50 %int_50 %int_50
%73 = OpExtInst %v3int %1 SMax %75 %77
%80 = OpIEqual %v3bool %73 %79
%82 = OpAll %bool %80
OpBranch %72
%72 = OpLabel
%83 = OpPhi %bool %false %60 %82 %71
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%87 = OpLoad %v4int %intValues
%88 = OpCompositeConstruct %v4int %int_50 %int_50 %int_50 %int_50
%86 = OpExtInst %v4int %1 SMax %87 %88
%91 = OpIEqual %v4bool %86 %90
%93 = OpAll %bool %91
OpBranch %85
%85 = OpLabel
%94 = OpPhi %bool %false %72 %93 %84
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%98 = OpLoad %v4int %intValues
%99 = OpCompositeExtract %int %98 0
%100 = OpLoad %v4int %intGreen
%101 = OpCompositeExtract %int %100 0
%97 = OpExtInst %int %1 SMax %99 %101
%102 = OpIEqual %bool %97 %int_0
OpBranch %96
%96 = OpLabel
%103 = OpPhi %bool %false %85 %102 %95
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%107 = OpLoad %v4int %intValues
%108 = OpVectorShuffle %v2int %107 %107 0 1
%109 = OpLoad %v4int %intGreen
%110 = OpVectorShuffle %v2int %109 %109 0 1
%106 = OpExtInst %v2int %1 SMax %108 %110
%113 = OpIEqual %v2bool %106 %112
%114 = OpAll %bool %113
OpBranch %105
%105 = OpLabel
%115 = OpPhi %bool %false %96 %114 %104
OpSelectionMerge %117 None
OpBranchConditional %115 %116 %117
%116 = OpLabel
%119 = OpLoad %v4int %intValues
%120 = OpVectorShuffle %v3int %119 %119 0 1 2
%121 = OpLoad %v4int %intGreen
%122 = OpVectorShuffle %v3int %121 %121 0 1 2
%118 = OpExtInst %v3int %1 SMax %120 %122
%124 = OpIEqual %v3bool %118 %123
%125 = OpAll %bool %124
OpBranch %117
%117 = OpLabel
%126 = OpPhi %bool %false %105 %125 %116
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%130 = OpLoad %v4int %intValues
%131 = OpLoad %v4int %intGreen
%129 = OpExtInst %v4int %1 SMax %130 %131
%133 = OpIEqual %v4bool %129 %132
%134 = OpAll %bool %133
OpBranch %128
%128 = OpLabel
%135 = OpPhi %bool %false %117 %134 %127
OpSelectionMerge %140 None
OpBranchConditional %135 %138 %139
%138 = OpLabel
%141 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%142 = OpLoad %v4float %141
OpStore %136 %142
OpBranch %140
%139 = OpLabel
%143 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%145 = OpLoad %v4float %143
OpStore %136 %145
OpBranch %140
%140 = OpLabel
%146 = OpLoad %v4float %136
OpReturnValue %146
OpFunctionEnd
