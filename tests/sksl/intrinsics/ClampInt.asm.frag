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
OpDecorate %134 RelaxedPrecision
OpDecorate %137 RelaxedPrecision
OpDecorate %138 RelaxedPrecision
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
%v2int = OpTypeVector %int 2
%54 = OpConstantComposite %v2int %int_n100 %int_0
%v2bool = OpTypeVector %bool 2
%v3int = OpTypeVector %int 3
%int_75 = OpConstant %int 75
%68 = OpConstantComposite %v3int %int_n100 %int_0 %int_75
%v3bool = OpTypeVector %bool 3
%79 = OpConstantComposite %v4int %int_n100 %int_0 %int_75 %int_100
%v4bool = OpTypeVector %bool 4
%int_n200 = OpConstant %int -200
%97 = OpConstantComposite %v2int %int_n100 %int_n200
%int_200 = OpConstant %int 200
%99 = OpConstantComposite %v2int %int_100 %int_200
%108 = OpConstantComposite %v3int %int_n100 %int_n200 %int_n200
%int_50 = OpConstant %int 50
%110 = OpConstantComposite %v3int %int_100 %int_200 %int_50
%111 = OpConstantComposite %v3int %int_n100 %int_0 %int_50
%119 = OpConstantComposite %v4int %int_n100 %int_n200 %int_n200 %int_100
%int_300 = OpConstant %int 300
%121 = OpConstantComposite %v4int %int_100 %int_200 %int_50 %int_300
%int_225 = OpConstant %int 225
%123 = OpConstantComposite %v4int %int_n100 %int_0 %int_50 %int_225
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
%127 = OpVariable %_ptr_Function_v4float Function
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
%45 = OpIEqual %bool %40 %int_n100
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%49 = OpLoad %v4int %intValues
%50 = OpVectorShuffle %v2int %49 %49 0 1
%52 = OpCompositeConstruct %v2int %int_n100 %int_n100
%53 = OpCompositeConstruct %v2int %int_100 %int_100
%48 = OpExtInst %v2int %1 SClamp %50 %52 %53
%55 = OpIEqual %v2bool %48 %54
%57 = OpAll %bool %55
OpBranch %47
%47 = OpLabel
%58 = OpPhi %bool %false %19 %57 %46
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%62 = OpLoad %v4int %intValues
%63 = OpVectorShuffle %v3int %62 %62 0 1 2
%65 = OpCompositeConstruct %v3int %int_n100 %int_n100 %int_n100
%66 = OpCompositeConstruct %v3int %int_100 %int_100 %int_100
%61 = OpExtInst %v3int %1 SClamp %63 %65 %66
%69 = OpIEqual %v3bool %61 %68
%71 = OpAll %bool %69
OpBranch %60
%60 = OpLabel
%72 = OpPhi %bool %false %47 %71 %59
OpSelectionMerge %74 None
OpBranchConditional %72 %73 %74
%73 = OpLabel
%76 = OpLoad %v4int %intValues
%77 = OpCompositeConstruct %v4int %int_n100 %int_n100 %int_n100 %int_n100
%78 = OpCompositeConstruct %v4int %int_100 %int_100 %int_100 %int_100
%75 = OpExtInst %v4int %1 SClamp %76 %77 %78
%80 = OpIEqual %v4bool %75 %79
%82 = OpAll %bool %80
OpBranch %74
%74 = OpLabel
%83 = OpPhi %bool %false %60 %82 %73
OpSelectionMerge %85 None
OpBranchConditional %83 %84 %85
%84 = OpLabel
%87 = OpLoad %v4int %intValues
%88 = OpCompositeExtract %int %87 0
%86 = OpExtInst %int %1 SClamp %88 %int_n100 %int_100
%89 = OpIEqual %bool %86 %int_n100
OpBranch %85
%85 = OpLabel
%90 = OpPhi %bool %false %74 %89 %84
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%94 = OpLoad %v4int %intValues
%95 = OpVectorShuffle %v2int %94 %94 0 1
%93 = OpExtInst %v2int %1 SClamp %95 %97 %99
%100 = OpIEqual %v2bool %93 %54
%101 = OpAll %bool %100
OpBranch %92
%92 = OpLabel
%102 = OpPhi %bool %false %85 %101 %91
OpSelectionMerge %104 None
OpBranchConditional %102 %103 %104
%103 = OpLabel
%106 = OpLoad %v4int %intValues
%107 = OpVectorShuffle %v3int %106 %106 0 1 2
%105 = OpExtInst %v3int %1 SClamp %107 %108 %110
%112 = OpIEqual %v3bool %105 %111
%113 = OpAll %bool %112
OpBranch %104
%104 = OpLabel
%114 = OpPhi %bool %false %92 %113 %103
OpSelectionMerge %116 None
OpBranchConditional %114 %115 %116
%115 = OpLabel
%118 = OpLoad %v4int %intValues
%117 = OpExtInst %v4int %1 SClamp %118 %119 %121
%124 = OpIEqual %v4bool %117 %123
%125 = OpAll %bool %124
OpBranch %116
%116 = OpLabel
%126 = OpPhi %bool %false %104 %125 %115
OpSelectionMerge %131 None
OpBranchConditional %126 %129 %130
%129 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%134 = OpLoad %v4float %132
OpStore %127 %134
OpBranch %131
%130 = OpLabel
%135 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%137 = OpLoad %v4float %135
OpStore %127 %137
OpBranch %131
%131 = OpLabel
%138 = OpLoad %v4float %127
OpReturnValue %138
OpFunctionEnd
