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
OpDecorate %139 RelaxedPrecision
OpDecorate %142 RelaxedPrecision
OpDecorate %143 RelaxedPrecision
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
%false = OpConstantFalse %bool
%int_n100 = OpConstant %int -100
%int_100 = OpConstant %int 100
%int_75 = OpConstant %int 75
%46 = OpConstantComposite %v4int %int_n100 %int_0 %int_75 %int_100
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%int_n200 = OpConstant %int -200
%91 = OpConstantComposite %v4int %int_n100 %int_n200 %int_n200 %int_100
%int_200 = OpConstant %int 200
%int_50 = OpConstant %int 50
%int_300 = OpConstant %int 300
%96 = OpConstantComposite %v4int %int_100 %int_200 %int_50 %int_300
%int_225 = OpConstant %int 225
%99 = OpConstantComposite %v4int %int_n100 %int_0 %int_50 %int_225
%_ptr_Function_v4float = OpTypePointer Function %v4float
%int_1 = OpConstant %int 1
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
%132 = OpVariable %_ptr_Function_v4float Function
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
%41 = OpLoad %v4int %intValues
%42 = OpCompositeExtract %int %41 0
%40 = OpExtInst %int %1 SClamp %42 %int_n100 %int_100
%47 = OpCompositeExtract %int %46 0
%48 = OpIEqual %bool %40 %47
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%52 = OpLoad %v4int %intValues
%53 = OpVectorShuffle %v2int %52 %52 0 1
%55 = OpCompositeConstruct %v2int %int_n100 %int_n100
%56 = OpCompositeConstruct %v2int %int_100 %int_100
%51 = OpExtInst %v2int %1 SClamp %53 %55 %56
%57 = OpVectorShuffle %v2int %46 %46 0 1
%58 = OpIEqual %v2bool %51 %57
%60 = OpAll %bool %58
OpBranch %50
%50 = OpLabel
%61 = OpPhi %bool %false %19 %60 %49
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpLoad %v4int %intValues
%66 = OpVectorShuffle %v3int %65 %65 0 1 2
%68 = OpCompositeConstruct %v3int %int_n100 %int_n100 %int_n100
%69 = OpCompositeConstruct %v3int %int_100 %int_100 %int_100
%64 = OpExtInst %v3int %1 SClamp %66 %68 %69
%70 = OpVectorShuffle %v3int %46 %46 0 1 2
%71 = OpIEqual %v3bool %64 %70
%73 = OpAll %bool %71
OpBranch %63
%63 = OpLabel
%74 = OpPhi %bool %false %50 %73 %62
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%78 = OpLoad %v4int %intValues
%79 = OpCompositeConstruct %v4int %int_n100 %int_n100 %int_n100 %int_n100
%80 = OpCompositeConstruct %v4int %int_100 %int_100 %int_100 %int_100
%77 = OpExtInst %v4int %1 SClamp %78 %79 %80
%81 = OpIEqual %v4bool %77 %46
%83 = OpAll %bool %81
OpBranch %76
%76 = OpLabel
%84 = OpPhi %bool %false %63 %83 %75
OpSelectionMerge %86 None
OpBranchConditional %84 %85 %86
%85 = OpLabel
%88 = OpLoad %v4int %intValues
%89 = OpCompositeExtract %int %88 0
%92 = OpCompositeExtract %int %91 0
%97 = OpCompositeExtract %int %96 0
%87 = OpExtInst %int %1 SClamp %89 %92 %97
%100 = OpCompositeExtract %int %99 0
%101 = OpIEqual %bool %87 %100
OpBranch %86
%86 = OpLabel
%102 = OpPhi %bool %false %76 %101 %85
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%106 = OpLoad %v4int %intValues
%107 = OpVectorShuffle %v2int %106 %106 0 1
%108 = OpVectorShuffle %v2int %91 %91 0 1
%109 = OpVectorShuffle %v2int %96 %96 0 1
%105 = OpExtInst %v2int %1 SClamp %107 %108 %109
%110 = OpVectorShuffle %v2int %99 %99 0 1
%111 = OpIEqual %v2bool %105 %110
%112 = OpAll %bool %111
OpBranch %104
%104 = OpLabel
%113 = OpPhi %bool %false %86 %112 %103
OpSelectionMerge %115 None
OpBranchConditional %113 %114 %115
%114 = OpLabel
%117 = OpLoad %v4int %intValues
%118 = OpVectorShuffle %v3int %117 %117 0 1 2
%119 = OpVectorShuffle %v3int %91 %91 0 1 2
%120 = OpVectorShuffle %v3int %96 %96 0 1 2
%116 = OpExtInst %v3int %1 SClamp %118 %119 %120
%121 = OpVectorShuffle %v3int %99 %99 0 1 2
%122 = OpIEqual %v3bool %116 %121
%123 = OpAll %bool %122
OpBranch %115
%115 = OpLabel
%124 = OpPhi %bool %false %104 %123 %114
OpSelectionMerge %126 None
OpBranchConditional %124 %125 %126
%125 = OpLabel
%128 = OpLoad %v4int %intValues
%127 = OpExtInst %v4int %1 SClamp %128 %91 %96
%129 = OpIEqual %v4bool %127 %99
%130 = OpAll %bool %129
OpBranch %126
%126 = OpLabel
%131 = OpPhi %bool %false %115 %130 %125
OpSelectionMerge %136 None
OpBranchConditional %131 %134 %135
%134 = OpLabel
%137 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%139 = OpLoad %v4float %137
OpStore %132 %139
OpBranch %136
%135 = OpLabel
%140 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%142 = OpLoad %v4float %140
OpStore %132 %142
OpBranch %136
%136 = OpLabel
%143 = OpLoad %v4float %132
OpReturnValue %143
OpFunctionEnd
