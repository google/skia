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
OpDecorate %140 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
OpDecorate %144 RelaxedPrecision
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
%int_n125 = OpConstant %int -125
%v2int = OpTypeVector %int 2
%67 = OpConstantComposite %v2int %int_n125 %int_0
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%79 = OpConstantComposite %v3int %int_n125 %int_0 %int_50
%v3bool = OpTypeVector %bool 3
%89 = OpConstantComposite %v4int %int_n125 %int_0 %int_50 %int_50
%v4bool = OpTypeVector %bool 4
%120 = OpConstantComposite %v3int %int_n125 %int_0 %int_0
%int_100 = OpConstant %int 100
%130 = OpConstantComposite %v4int %int_n125 %int_0 %int_0 %int_100
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
%134 = OpVariable %_ptr_Function_v4float Function
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
%54 = OpExtInst %int %1 SMin %56 %int_50
%59 = OpIEqual %bool %54 %int_n125
OpSelectionMerge %61 None
OpBranchConditional %59 %60 %61
%60 = OpLabel
%63 = OpLoad %v4int %intValues
%64 = OpVectorShuffle %v2int %63 %63 0 1
%66 = OpCompositeConstruct %v2int %int_50 %int_50
%62 = OpExtInst %v2int %1 SMin %64 %66
%68 = OpIEqual %v2bool %62 %67
%70 = OpAll %bool %68
OpBranch %61
%61 = OpLabel
%71 = OpPhi %bool %false %19 %70 %60
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%75 = OpLoad %v4int %intValues
%76 = OpVectorShuffle %v3int %75 %75 0 1 2
%78 = OpCompositeConstruct %v3int %int_50 %int_50 %int_50
%74 = OpExtInst %v3int %1 SMin %76 %78
%80 = OpIEqual %v3bool %74 %79
%82 = OpAll %bool %80
OpBranch %73
%73 = OpLabel
%83 = OpPhi %bool %false %61 %82 %72
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%87 = OpLoad %v4int %intValues
%88 = OpCompositeConstruct %v4int %int_50 %int_50 %int_50 %int_50
%86 = OpExtInst %v4int %1 SMin %87 %88
%90 = OpIEqual %v4bool %86 %89
%92 = OpAll %bool %90
OpBranch %85
%85 = OpLabel
%93 = OpPhi %bool %false %73 %92 %84
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%97 = OpLoad %v4int %intValues
%98 = OpCompositeExtract %int %97 0
%99 = OpLoad %v4int %intGreen
%100 = OpCompositeExtract %int %99 0
%96 = OpExtInst %int %1 SMin %98 %100
%101 = OpIEqual %bool %96 %int_n125
OpBranch %95
%95 = OpLabel
%102 = OpPhi %bool %false %85 %101 %94
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%106 = OpLoad %v4int %intValues
%107 = OpVectorShuffle %v2int %106 %106 0 1
%108 = OpLoad %v4int %intGreen
%109 = OpVectorShuffle %v2int %108 %108 0 1
%105 = OpExtInst %v2int %1 SMin %107 %109
%110 = OpIEqual %v2bool %105 %67
%111 = OpAll %bool %110
OpBranch %104
%104 = OpLabel
%112 = OpPhi %bool %false %95 %111 %103
OpSelectionMerge %114 None
OpBranchConditional %112 %113 %114
%113 = OpLabel
%116 = OpLoad %v4int %intValues
%117 = OpVectorShuffle %v3int %116 %116 0 1 2
%118 = OpLoad %v4int %intGreen
%119 = OpVectorShuffle %v3int %118 %118 0 1 2
%115 = OpExtInst %v3int %1 SMin %117 %119
%121 = OpIEqual %v3bool %115 %120
%122 = OpAll %bool %121
OpBranch %114
%114 = OpLabel
%123 = OpPhi %bool %false %104 %122 %113
OpSelectionMerge %125 None
OpBranchConditional %123 %124 %125
%124 = OpLabel
%127 = OpLoad %v4int %intValues
%128 = OpLoad %v4int %intGreen
%126 = OpExtInst %v4int %1 SMin %127 %128
%131 = OpIEqual %v4bool %126 %130
%132 = OpAll %bool %131
OpBranch %125
%125 = OpLabel
%133 = OpPhi %bool %false %114 %132 %124
OpSelectionMerge %138 None
OpBranchConditional %133 %136 %137
%136 = OpLabel
%139 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%140 = OpLoad %v4float %139
OpStore %134 %140
OpBranch %138
%137 = OpLabel
%141 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%143 = OpLoad %v4float %141
OpStore %134 %143
OpBranch %138
%138 = OpLabel
%144 = OpLoad %v4float %134
OpReturnValue %144
OpFunctionEnd
