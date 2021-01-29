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
OpDecorate %37 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %84 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %99 RelaxedPrecision
OpDecorate %108 RelaxedPrecision
OpDecorate %110 RelaxedPrecision
OpDecorate %121 RelaxedPrecision
OpDecorate %124 RelaxedPrecision
OpDecorate %125 RelaxedPrecision
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
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%39 = OpConstantComposite %v2float %float_0 %float_0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%52 = OpConstantComposite %v3float %float_0 %float_0 %float_0
%v3bool = OpTypeVector %bool 3
%63 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
%v4bool = OpTypeVector %bool 4
%int_3 = OpConstant %int 3
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
%114 = OpVariable %_ptr_Function_v4float Function
%23 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%27 = OpLoad %v4float %23
%28 = OpCompositeExtract %float %27 0
%21 = OpExtInst %float %1 Step %float_1 %28
%30 = OpFOrdEqual %bool %21 %float_0
OpSelectionMerge %32 None
OpBranchConditional %30 %31 %32
%31 = OpLabel
%34 = OpCompositeConstruct %v2float %float_1 %float_1
%36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%37 = OpLoad %v4float %36
%38 = OpVectorShuffle %v2float %37 %37 0 1
%33 = OpExtInst %v2float %1 Step %34 %38
%40 = OpFOrdEqual %v2bool %33 %39
%42 = OpAll %bool %40
OpBranch %32
%32 = OpLabel
%43 = OpPhi %bool %false %19 %42 %31
OpSelectionMerge %45 None
OpBranchConditional %43 %44 %45
%44 = OpLabel
%47 = OpCompositeConstruct %v3float %float_1 %float_1 %float_1
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %49
%51 = OpVectorShuffle %v3float %50 %50 0 1 2
%46 = OpExtInst %v3float %1 Step %47 %51
%53 = OpFOrdEqual %v3bool %46 %52
%55 = OpAll %bool %53
OpBranch %45
%45 = OpLabel
%56 = OpPhi %bool %false %32 %55 %44
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%60 = OpCompositeConstruct %v4float %float_1 %float_1 %float_1 %float_1
%61 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%62 = OpLoad %v4float %61
%59 = OpExtInst %v4float %1 Step %60 %62
%64 = OpFOrdEqual %v4bool %59 %63
%66 = OpAll %bool %64
OpBranch %58
%58 = OpLabel
%67 = OpPhi %bool %false %45 %66 %57
OpSelectionMerge %69 None
OpBranchConditional %67 %68 %69
%68 = OpLabel
%71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%73 = OpLoad %v4float %71
%74 = OpCompositeExtract %float %73 0
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%76 = OpLoad %v4float %75
%77 = OpCompositeExtract %float %76 0
%70 = OpExtInst %float %1 Step %74 %77
%78 = OpFOrdEqual %bool %70 %float_0
OpBranch %69
%69 = OpLabel
%79 = OpPhi %bool %false %58 %78 %68
OpSelectionMerge %81 None
OpBranchConditional %79 %80 %81
%80 = OpLabel
%83 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%84 = OpLoad %v4float %83
%85 = OpVectorShuffle %v2float %84 %84 0 1
%86 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%87 = OpLoad %v4float %86
%88 = OpVectorShuffle %v2float %87 %87 0 1
%82 = OpExtInst %v2float %1 Step %85 %88
%89 = OpFOrdEqual %v2bool %82 %39
%90 = OpAll %bool %89
OpBranch %81
%81 = OpLabel
%91 = OpPhi %bool %false %69 %90 %80
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%95 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%96 = OpLoad %v4float %95
%97 = OpVectorShuffle %v3float %96 %96 0 1 2
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%99 = OpLoad %v4float %98
%100 = OpVectorShuffle %v3float %99 %99 0 1 2
%94 = OpExtInst %v3float %1 Step %97 %100
%101 = OpFOrdEqual %v3bool %94 %52
%102 = OpAll %bool %101
OpBranch %93
%93 = OpLabel
%103 = OpPhi %bool %false %81 %102 %92
OpSelectionMerge %105 None
OpBranchConditional %103 %104 %105
%104 = OpLabel
%107 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%108 = OpLoad %v4float %107
%109 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%110 = OpLoad %v4float %109
%106 = OpExtInst %v4float %1 Step %108 %110
%111 = OpFOrdEqual %v4bool %106 %63
%112 = OpAll %bool %111
OpBranch %105
%105 = OpLabel
%113 = OpPhi %bool %false %93 %112 %104
OpSelectionMerge %118 None
OpBranchConditional %113 %116 %117
%116 = OpLabel
%119 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%121 = OpLoad %v4float %119
OpStore %114 %121
OpBranch %118
%117 = OpLabel
%122 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%124 = OpLoad %v4float %122
OpStore %114 %124
OpBranch %118
%118 = OpLabel
%125 = OpLoad %v4float %114
OpReturnValue %125
OpFunctionEnd
