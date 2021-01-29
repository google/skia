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
OpMemberName %_UniformBuffer 3 "colorWhite"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %vector2 "vector2"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %27 RelaxedPrecision
OpDecorate %33 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %113 RelaxedPrecision
OpDecorate %123 RelaxedPrecision
OpDecorate %126 RelaxedPrecision
OpDecorate %127 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%18 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_2 = OpConstant %float 2
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%false = OpConstantFalse %bool
%int_0 = OpConstant %int 0
%float_0_75 = OpConstant %float 0.75
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%46 = OpConstantComposite %v2float %float_0_75 %float_0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%59 = OpConstantComposite %v3float %float_0_75 %float_0 %float_0_75
%v3bool = OpTypeVector %bool 3
%float_0_25 = OpConstant %float 0.25
%71 = OpConstantComposite %v4float %float_0_75 %float_0 %float_0_75 %float_0_25
%v4bool = OpTypeVector %bool 4
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
%vector2 = OpVariable %_ptr_Function_v4float Function
%117 = OpVariable %_ptr_Function_v4float Function
%23 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%27 = OpLoad %v4float %23
%28 = OpVectorTimesScalar %v4float %27 %float_2
OpStore %vector2 %28
%31 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%33 = OpLoad %v4float %31
%34 = OpCompositeExtract %float %33 0
%30 = OpFMod %float %34 %float_2
%36 = OpFOrdEqual %bool %30 %float_0_75
OpSelectionMerge %38 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%41 = OpLoad %v4float %40
%42 = OpVectorShuffle %v2float %41 %41 0 1
%44 = OpCompositeConstruct %v2float %float_2 %float_2
%39 = OpFMod %v2float %42 %44
%47 = OpFOrdEqual %v2bool %39 %46
%49 = OpAll %bool %47
OpBranch %38
%38 = OpLabel
%50 = OpPhi %bool %false %19 %49 %37
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
%54 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%55 = OpLoad %v4float %54
%56 = OpVectorShuffle %v3float %55 %55 0 1 2
%58 = OpCompositeConstruct %v3float %float_2 %float_2 %float_2
%53 = OpFMod %v3float %56 %58
%60 = OpFOrdEqual %v3bool %53 %59
%62 = OpAll %bool %60
OpBranch %52
%52 = OpLabel
%63 = OpPhi %bool %false %38 %62 %51
OpSelectionMerge %65 None
OpBranchConditional %63 %64 %65
%64 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%68 = OpLoad %v4float %67
%69 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%66 = OpFMod %v4float %68 %69
%72 = OpFOrdEqual %v4bool %66 %71
%74 = OpAll %bool %72
OpBranch %65
%65 = OpLabel
%75 = OpPhi %bool %false %52 %74 %64
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%79 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%80 = OpLoad %v4float %79
%81 = OpCompositeExtract %float %80 0
%82 = OpLoad %v4float %vector2
%83 = OpCompositeExtract %float %82 0
%78 = OpFMod %float %81 %83
%84 = OpFOrdEqual %bool %78 %float_0_75
OpBranch %77
%77 = OpLabel
%85 = OpPhi %bool %false %65 %84 %76
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%90 = OpLoad %v4float %89
%91 = OpVectorShuffle %v2float %90 %90 0 1
%92 = OpLoad %v4float %vector2
%93 = OpVectorShuffle %v2float %92 %92 0 1
%88 = OpFMod %v2float %91 %93
%94 = OpFOrdEqual %v2bool %88 %46
%95 = OpAll %bool %94
OpBranch %87
%87 = OpLabel
%96 = OpPhi %bool %false %77 %95 %86
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%101 = OpLoad %v4float %100
%102 = OpVectorShuffle %v3float %101 %101 0 1 2
%103 = OpLoad %v4float %vector2
%104 = OpVectorShuffle %v3float %103 %103 0 1 2
%99 = OpFMod %v3float %102 %104
%105 = OpFOrdEqual %v3bool %99 %59
%106 = OpAll %bool %105
OpBranch %98
%98 = OpLabel
%107 = OpPhi %bool %false %87 %106 %97
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%111 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%112 = OpLoad %v4float %111
%113 = OpLoad %v4float %vector2
%110 = OpFMod %v4float %112 %113
%114 = OpFOrdEqual %v4bool %110 %71
%115 = OpAll %bool %114
OpBranch %109
%109 = OpLabel
%116 = OpPhi %bool %false %98 %115 %108
OpSelectionMerge %120 None
OpBranchConditional %116 %118 %119
%118 = OpLabel
%121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%123 = OpLoad %v4float %121
OpStore %117 %123
OpBranch %120
%119 = OpLabel
%124 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%126 = OpLoad %v4float %124
OpStore %117 %126
OpBranch %120
%120 = OpLabel
%127 = OpLoad %v4float %117
OpReturnValue %127
OpFunctionEnd
