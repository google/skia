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
OpDecorate %143 RelaxedPrecision
OpDecorate %146 RelaxedPrecision
OpDecorate %147 RelaxedPrecision
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
%59 = OpConstantComposite %v4int %int_n125 %int_0 %int_50 %int_50
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%int_100 = OpConstant %int 100
%103 = OpConstantComposite %v4int %int_n125 %int_0 %int_0 %int_100
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
%137 = OpVariable %_ptr_Function_v4float Function
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
%60 = OpCompositeExtract %int %59 0
%61 = OpIEqual %bool %54 %60
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpLoad %v4int %intValues
%66 = OpVectorShuffle %v2int %65 %65 0 1
%68 = OpCompositeConstruct %v2int %int_50 %int_50
%64 = OpExtInst %v2int %1 SMin %66 %68
%69 = OpVectorShuffle %v2int %59 %59 0 1
%70 = OpIEqual %v2bool %64 %69
%72 = OpAll %bool %70
OpBranch %63
%63 = OpLabel
%73 = OpPhi %bool %false %19 %72 %62
OpSelectionMerge %75 None
OpBranchConditional %73 %74 %75
%74 = OpLabel
%77 = OpLoad %v4int %intValues
%78 = OpVectorShuffle %v3int %77 %77 0 1 2
%80 = OpCompositeConstruct %v3int %int_50 %int_50 %int_50
%76 = OpExtInst %v3int %1 SMin %78 %80
%81 = OpVectorShuffle %v3int %59 %59 0 1 2
%82 = OpIEqual %v3bool %76 %81
%84 = OpAll %bool %82
OpBranch %75
%75 = OpLabel
%85 = OpPhi %bool %false %63 %84 %74
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%89 = OpLoad %v4int %intValues
%90 = OpCompositeConstruct %v4int %int_50 %int_50 %int_50 %int_50
%88 = OpExtInst %v4int %1 SMin %89 %90
%91 = OpIEqual %v4bool %88 %59
%93 = OpAll %bool %91
OpBranch %87
%87 = OpLabel
%94 = OpPhi %bool %false %75 %93 %86
OpSelectionMerge %96 None
OpBranchConditional %94 %95 %96
%95 = OpLabel
%98 = OpLoad %v4int %intValues
%99 = OpCompositeExtract %int %98 0
%100 = OpLoad %v4int %intGreen
%101 = OpCompositeExtract %int %100 0
%97 = OpExtInst %int %1 SMin %99 %101
%104 = OpCompositeExtract %int %103 0
%105 = OpIEqual %bool %97 %104
OpBranch %96
%96 = OpLabel
%106 = OpPhi %bool %false %87 %105 %95
OpSelectionMerge %108 None
OpBranchConditional %106 %107 %108
%107 = OpLabel
%110 = OpLoad %v4int %intValues
%111 = OpVectorShuffle %v2int %110 %110 0 1
%112 = OpLoad %v4int %intGreen
%113 = OpVectorShuffle %v2int %112 %112 0 1
%109 = OpExtInst %v2int %1 SMin %111 %113
%114 = OpVectorShuffle %v2int %103 %103 0 1
%115 = OpIEqual %v2bool %109 %114
%116 = OpAll %bool %115
OpBranch %108
%108 = OpLabel
%117 = OpPhi %bool %false %96 %116 %107
OpSelectionMerge %119 None
OpBranchConditional %117 %118 %119
%118 = OpLabel
%121 = OpLoad %v4int %intValues
%122 = OpVectorShuffle %v3int %121 %121 0 1 2
%123 = OpLoad %v4int %intGreen
%124 = OpVectorShuffle %v3int %123 %123 0 1 2
%120 = OpExtInst %v3int %1 SMin %122 %124
%125 = OpVectorShuffle %v3int %103 %103 0 1 2
%126 = OpIEqual %v3bool %120 %125
%127 = OpAll %bool %126
OpBranch %119
%119 = OpLabel
%128 = OpPhi %bool %false %108 %127 %118
OpSelectionMerge %130 None
OpBranchConditional %128 %129 %130
%129 = OpLabel
%132 = OpLoad %v4int %intValues
%133 = OpLoad %v4int %intGreen
%131 = OpExtInst %v4int %1 SMin %132 %133
%134 = OpIEqual %v4bool %131 %103
%135 = OpAll %bool %134
OpBranch %130
%130 = OpLabel
%136 = OpPhi %bool %false %119 %135 %129
OpSelectionMerge %141 None
OpBranchConditional %136 %139 %140
%139 = OpLabel
%142 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%143 = OpLoad %v4float %142
OpStore %137 %143
OpBranch %141
%140 = OpLabel
%144 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%146 = OpLoad %v4float %144
OpStore %137 %146
OpBranch %141
%141 = OpLabel
%147 = OpLoad %v4float %137
OpReturnValue %147
OpFunctionEnd
