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
OpDecorate %34 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %107 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
OpDecorate %118 RelaxedPrecision
OpDecorate %128 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %132 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%int_0 = OpConstant %int 0
%v2float = OpTypeVector %float 2
%46 = OpConstantComposite %v2float %float_0 %float_0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%float_0_31640625 = OpConstant %float 0.31640625
%61 = OpConstantComposite %v3float %float_0 %float_0 %float_0_31640625
%v3bool = OpTypeVector %bool 3
%float_1 = OpConstant %float 1
%74 = OpConstantComposite %v4float %float_0 %float_0 %float_0_31640625 %float_1
%v4bool = OpTypeVector %bool 4
%103 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%115 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
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
%122 = OpVariable %_ptr_Function_v4float Function
%23 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%27 = OpLoad %v4float %23
%28 = OpVectorTimesScalar %v4float %27 %float_2
OpStore %vector2 %28
%32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%34 = OpLoad %v4float %32
%35 = OpCompositeExtract %float %34 0
%30 = OpExtInst %float %1 SmoothStep %float_0 %float_2 %35
%36 = OpFOrdEqual %bool %30 %float_0
OpSelectionMerge %38 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
%40 = OpCompositeConstruct %v2float %float_0 %float_0
%42 = OpCompositeConstruct %v2float %float_2 %float_2
%43 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %43
%45 = OpVectorShuffle %v2float %44 %44 0 1
%39 = OpExtInst %v2float %1 SmoothStep %40 %42 %45
%47 = OpFOrdEqual %v2bool %39 %46
%49 = OpAll %bool %47
OpBranch %38
%38 = OpLabel
%50 = OpPhi %bool %false %19 %49 %37
OpSelectionMerge %52 None
OpBranchConditional %50 %51 %52
%51 = OpLabel
%54 = OpCompositeConstruct %v3float %float_0 %float_0 %float_0
%56 = OpCompositeConstruct %v3float %float_2 %float_2 %float_2
%57 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%58 = OpLoad %v4float %57
%59 = OpVectorShuffle %v3float %58 %58 0 1 2
%53 = OpExtInst %v3float %1 SmoothStep %54 %56 %59
%62 = OpFOrdEqual %v3bool %53 %61
%64 = OpAll %bool %62
OpBranch %52
%52 = OpLabel
%65 = OpPhi %bool %false %38 %64 %51
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%69 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_0
%70 = OpCompositeConstruct %v4float %float_2 %float_2 %float_2 %float_2
%71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%72 = OpLoad %v4float %71
%68 = OpExtInst %v4float %1 SmoothStep %69 %70 %72
%75 = OpFOrdEqual %v4bool %68 %74
%77 = OpAll %bool %75
OpBranch %67
%67 = OpLabel
%78 = OpPhi %bool %false %52 %77 %66
OpSelectionMerge %80 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
%82 = OpLoad %v4float %vector2
%83 = OpCompositeExtract %float %82 0
%84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%85 = OpLoad %v4float %84
%86 = OpCompositeExtract %float %85 0
%81 = OpExtInst %float %1 SmoothStep %float_0 %83 %86
%87 = OpFOrdEqual %bool %81 %float_0
OpBranch %80
%80 = OpLabel
%88 = OpPhi %bool %false %67 %87 %79
OpSelectionMerge %90 None
OpBranchConditional %88 %89 %90
%89 = OpLabel
%92 = OpLoad %v4float %vector2
%93 = OpVectorShuffle %v2float %92 %92 0 1
%94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%95 = OpLoad %v4float %94
%96 = OpVectorShuffle %v2float %95 %95 0 1
%91 = OpExtInst %v2float %1 SmoothStep %46 %93 %96
%97 = OpFOrdEqual %v2bool %91 %46
%98 = OpAll %bool %97
OpBranch %90
%90 = OpLabel
%99 = OpPhi %bool %false %80 %98 %89
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%104 = OpLoad %v4float %vector2
%105 = OpVectorShuffle %v3float %104 %104 0 1 2
%106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%107 = OpLoad %v4float %106
%108 = OpVectorShuffle %v3float %107 %107 0 1 2
%102 = OpExtInst %v3float %1 SmoothStep %103 %105 %108
%109 = OpFOrdEqual %v3bool %102 %61
%110 = OpAll %bool %109
OpBranch %101
%101 = OpLabel
%111 = OpPhi %bool %false %90 %110 %100
OpSelectionMerge %113 None
OpBranchConditional %111 %112 %113
%112 = OpLabel
%116 = OpLoad %v4float %vector2
%117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%118 = OpLoad %v4float %117
%114 = OpExtInst %v4float %1 SmoothStep %115 %116 %118
%119 = OpFOrdEqual %v4bool %114 %74
%120 = OpAll %bool %119
OpBranch %113
%113 = OpLabel
%121 = OpPhi %bool %false %101 %120 %112
OpSelectionMerge %125 None
OpBranchConditional %121 %123 %124
%123 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%128 = OpLoad %v4float %126
OpStore %122 %128
OpBranch %125
%124 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%131 = OpLoad %v4float %129
OpStore %122 %131
OpBranch %125
%125 = OpLabel
%132 = OpLoad %v4float %122
OpReturnValue %132
OpFunctionEnd
