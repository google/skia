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
OpDecorate %154 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
OpDecorate %158 RelaxedPrecision
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
%148 = OpVariable %_ptr_Function_v4float Function
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
%98 = OpCompositeConstruct %v4int %int_50 %int_50 %int_50 %int_50
%96 = OpExtInst %v4int %1 SMax %97 %98
%99 = OpLoad %v4int %expectedA
%100 = OpIEqual %v4bool %96 %99
%102 = OpAll %bool %100
OpBranch %95
%95 = OpLabel
%103 = OpPhi %bool %false %82 %102 %94
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%107 = OpLoad %v4int %intValues
%108 = OpCompositeExtract %int %107 0
%109 = OpLoad %v4int %intGreen
%110 = OpCompositeExtract %int %109 0
%106 = OpExtInst %int %1 SMax %108 %110
%111 = OpLoad %v4int %expectedB
%112 = OpCompositeExtract %int %111 0
%113 = OpIEqual %bool %106 %112
OpBranch %105
%105 = OpLabel
%114 = OpPhi %bool %false %95 %113 %104
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%118 = OpLoad %v4int %intValues
%119 = OpVectorShuffle %v2int %118 %118 0 1
%120 = OpLoad %v4int %intGreen
%121 = OpVectorShuffle %v2int %120 %120 0 1
%117 = OpExtInst %v2int %1 SMax %119 %121
%122 = OpLoad %v4int %expectedB
%123 = OpVectorShuffle %v2int %122 %122 0 1
%124 = OpIEqual %v2bool %117 %123
%125 = OpAll %bool %124
OpBranch %116
%116 = OpLabel
%126 = OpPhi %bool %false %105 %125 %115
OpSelectionMerge %128 None
OpBranchConditional %126 %127 %128
%127 = OpLabel
%130 = OpLoad %v4int %intValues
%131 = OpVectorShuffle %v3int %130 %130 0 1 2
%132 = OpLoad %v4int %intGreen
%133 = OpVectorShuffle %v3int %132 %132 0 1 2
%129 = OpExtInst %v3int %1 SMax %131 %133
%134 = OpLoad %v4int %expectedB
%135 = OpVectorShuffle %v3int %134 %134 0 1 2
%136 = OpIEqual %v3bool %129 %135
%137 = OpAll %bool %136
OpBranch %128
%128 = OpLabel
%138 = OpPhi %bool %false %116 %137 %127
OpSelectionMerge %140 None
OpBranchConditional %138 %139 %140
%139 = OpLabel
%142 = OpLoad %v4int %intValues
%143 = OpLoad %v4int %intGreen
%141 = OpExtInst %v4int %1 SMax %142 %143
%144 = OpLoad %v4int %expectedB
%145 = OpIEqual %v4bool %141 %144
%146 = OpAll %bool %145
OpBranch %140
%140 = OpLabel
%147 = OpPhi %bool %false %128 %146 %139
OpSelectionMerge %152 None
OpBranchConditional %147 %150 %151
%150 = OpLabel
%153 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%154 = OpLoad %v4float %153
OpStore %148 %154
OpBranch %152
%151 = OpLabel
%155 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%157 = OpLoad %v4float %155
OpStore %148 %157
OpBranch %152
%152 = OpLabel
%158 = OpLoad %v4float %148
OpReturnValue %158
OpFunctionEnd
