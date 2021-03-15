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
OpDecorate %153 RelaxedPrecision
OpDecorate %156 RelaxedPrecision
OpDecorate %157 RelaxedPrecision
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
%int_n125 = OpConstant %int -125
%int_50 = OpConstant %int 50
%56 = OpConstantComposite %v4int %int_n125 %int_0 %int_50 %int_50
%int_100 = OpConstant %int 100
%59 = OpConstantComposite %v4int %int_n125 %int_0 %int_0 %int_100
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
%147 = OpVariable %_ptr_Function_v4float Function
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
OpStore %expectedA %56
OpStore %expectedB %59
%62 = OpLoad %v4int %intValues
%63 = OpCompositeExtract %int %62 0
%61 = OpExtInst %int %1 SMin %63 %int_50
%64 = OpLoad %v4int %expectedA
%65 = OpCompositeExtract %int %64 0
%66 = OpIEqual %bool %61 %65
OpSelectionMerge %68 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%70 = OpLoad %v4int %intValues
%71 = OpVectorShuffle %v2int %70 %70 0 1
%73 = OpCompositeConstruct %v2int %int_50 %int_50
%69 = OpExtInst %v2int %1 SMin %71 %73
%74 = OpLoad %v4int %expectedA
%75 = OpVectorShuffle %v2int %74 %74 0 1
%76 = OpIEqual %v2bool %69 %75
%78 = OpAll %bool %76
OpBranch %68
%68 = OpLabel
%79 = OpPhi %bool %false %19 %78 %67
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%83 = OpLoad %v4int %intValues
%84 = OpVectorShuffle %v3int %83 %83 0 1 2
%86 = OpCompositeConstruct %v3int %int_50 %int_50 %int_50
%82 = OpExtInst %v3int %1 SMin %84 %86
%87 = OpLoad %v4int %expectedA
%88 = OpVectorShuffle %v3int %87 %87 0 1 2
%89 = OpIEqual %v3bool %82 %88
%91 = OpAll %bool %89
OpBranch %81
%81 = OpLabel
%92 = OpPhi %bool %false %68 %91 %80
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpLoad %v4int %intValues
%97 = OpCompositeConstruct %v4int %int_50 %int_50 %int_50 %int_50
%95 = OpExtInst %v4int %1 SMin %96 %97
%98 = OpLoad %v4int %expectedA
%99 = OpIEqual %v4bool %95 %98
%101 = OpAll %bool %99
OpBranch %94
%94 = OpLabel
%102 = OpPhi %bool %false %81 %101 %93
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%106 = OpLoad %v4int %intValues
%107 = OpCompositeExtract %int %106 0
%108 = OpLoad %v4int %intGreen
%109 = OpCompositeExtract %int %108 0
%105 = OpExtInst %int %1 SMin %107 %109
%110 = OpLoad %v4int %expectedB
%111 = OpCompositeExtract %int %110 0
%112 = OpIEqual %bool %105 %111
OpBranch %104
%104 = OpLabel
%113 = OpPhi %bool %false %94 %112 %103
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpLoad %v4int %intValues
%118 = OpVectorShuffle %v2int %117 %117 0 1
%119 = OpLoad %v4int %intGreen
%120 = OpVectorShuffle %v2int %119 %119 0 1
%116 = OpExtInst %v2int %1 SMin %118 %120
%121 = OpLoad %v4int %expectedB
%122 = OpVectorShuffle %v2int %121 %121 0 1
%123 = OpIEqual %v2bool %116 %122
%124 = OpAll %bool %123
OpBranch %115
%115 = OpLabel
%125 = OpPhi %bool %false %104 %124 %114
OpSelectionMerge %127 None
OpBranchConditional %125 %126 %127
%126 = OpLabel
%129 = OpLoad %v4int %intValues
%130 = OpVectorShuffle %v3int %129 %129 0 1 2
%131 = OpLoad %v4int %intGreen
%132 = OpVectorShuffle %v3int %131 %131 0 1 2
%128 = OpExtInst %v3int %1 SMin %130 %132
%133 = OpLoad %v4int %expectedB
%134 = OpVectorShuffle %v3int %133 %133 0 1 2
%135 = OpIEqual %v3bool %128 %134
%136 = OpAll %bool %135
OpBranch %127
%127 = OpLabel
%137 = OpPhi %bool %false %115 %136 %126
OpSelectionMerge %139 None
OpBranchConditional %137 %138 %139
%138 = OpLabel
%141 = OpLoad %v4int %intValues
%142 = OpLoad %v4int %intGreen
%140 = OpExtInst %v4int %1 SMin %141 %142
%143 = OpLoad %v4int %expectedB
%144 = OpIEqual %v4bool %140 %143
%145 = OpAll %bool %144
OpBranch %139
%139 = OpLabel
%146 = OpPhi %bool %false %127 %145 %138
OpSelectionMerge %151 None
OpBranchConditional %146 %149 %150
%149 = OpLabel
%152 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%153 = OpLoad %v4float %152
OpStore %147 %153
OpBranch %151
%150 = OpLabel
%154 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%156 = OpLoad %v4float %154
OpStore %147 %156
OpBranch %151
%151 = OpLabel
%157 = OpLoad %v4float %147
OpReturnValue %157
OpFunctionEnd
