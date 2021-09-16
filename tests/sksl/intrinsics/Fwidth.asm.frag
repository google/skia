OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInputs"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %expected "expected"
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
OpDecorate %expected RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %131 RelaxedPrecision
OpDecorate %134 RelaxedPrecision
OpDecorate %135 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%28 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_1 = OpConstant %float 1
%82 = OpConstantComposite %v2float %float_1 %float_1
%93 = OpConstantComposite %v2float %float_1 %float_0
%113 = OpConstantComposite %v2float %float_0 %float_1
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %15
%16 = OpLabel
%20 = OpVariable %_ptr_Function_v2float Function
OpStore %20 %19
%22 = OpFunctionCall %v4float %main %20
OpStore %sk_FragColor %22
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %23
%24 = OpFunctionParameter %_ptr_Function_v2float
%25 = OpLabel
%expected = OpVariable %_ptr_Function_v4float Function
%125 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %28
%31 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%35 = OpLoad %v4float %31
%36 = OpCompositeExtract %float %35 0
%30 = OpDPdx %float %36
%37 = OpLoad %v4float %expected
%38 = OpCompositeExtract %float %37 0
%39 = OpFOrdEqual %bool %30 %38
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%43 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %43
%45 = OpVectorShuffle %v2float %44 %44 0 1
%42 = OpDPdx %v2float %45
%46 = OpLoad %v4float %expected
%47 = OpVectorShuffle %v2float %46 %46 0 1
%48 = OpFOrdEqual %v2bool %42 %47
%50 = OpAll %bool %48
OpBranch %41
%41 = OpLabel
%51 = OpPhi %bool %false %25 %50 %40
OpSelectionMerge %53 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
%55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%56 = OpLoad %v4float %55
%57 = OpVectorShuffle %v3float %56 %56 0 1 2
%54 = OpDPdx %v3float %57
%59 = OpLoad %v4float %expected
%60 = OpVectorShuffle %v3float %59 %59 0 1 2
%61 = OpFOrdEqual %v3bool %54 %60
%63 = OpAll %bool %61
OpBranch %53
%53 = OpLabel
%64 = OpPhi %bool %false %41 %63 %52
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%68 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%69 = OpLoad %v4float %68
%67 = OpDPdx %v4float %69
%70 = OpLoad %v4float %expected
%71 = OpFOrdEqual %v4bool %67 %70
%73 = OpAll %bool %71
OpBranch %66
%66 = OpLabel
%74 = OpPhi %bool %false %53 %73 %65
OpSelectionMerge %76 None
OpBranchConditional %74 %75 %76
%75 = OpLabel
%79 = OpLoad %v2float %24
%80 = OpVectorShuffle %v2float %79 %79 0 0
%78 = OpFwidth %v2float %80
%77 = OpExtInst %v2float %1 FSign %78
%83 = OpFOrdEqual %v2bool %77 %82
%84 = OpAll %bool %83
OpBranch %76
%76 = OpLabel
%85 = OpPhi %bool %false %66 %84 %75
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%90 = OpLoad %v2float %24
%91 = OpCompositeExtract %float %90 0
%92 = OpCompositeConstruct %v2float %91 %float_1
%89 = OpFwidth %v2float %92
%88 = OpExtInst %v2float %1 FSign %89
%94 = OpFOrdEqual %v2bool %88 %93
%95 = OpAll %bool %94
OpBranch %87
%87 = OpLabel
%96 = OpPhi %bool %false %76 %95 %86
OpSelectionMerge %98 None
OpBranchConditional %96 %97 %98
%97 = OpLabel
%101 = OpLoad %v2float %24
%102 = OpVectorShuffle %v2float %101 %101 1 1
%100 = OpFwidth %v2float %102
%99 = OpExtInst %v2float %1 FSign %100
%103 = OpFOrdEqual %v2bool %99 %82
%104 = OpAll %bool %103
OpBranch %98
%98 = OpLabel
%105 = OpPhi %bool %false %87 %104 %97
OpSelectionMerge %107 None
OpBranchConditional %105 %106 %107
%106 = OpLabel
%110 = OpLoad %v2float %24
%111 = OpCompositeExtract %float %110 1
%112 = OpCompositeConstruct %v2float %float_0 %111
%109 = OpFwidth %v2float %112
%108 = OpExtInst %v2float %1 FSign %109
%114 = OpFOrdEqual %v2bool %108 %113
%115 = OpAll %bool %114
OpBranch %107
%107 = OpLabel
%116 = OpPhi %bool %false %98 %115 %106
OpSelectionMerge %118 None
OpBranchConditional %116 %117 %118
%117 = OpLabel
%121 = OpLoad %v2float %24
%120 = OpFwidth %v2float %121
%119 = OpExtInst %v2float %1 FSign %120
%122 = OpFOrdEqual %v2bool %119 %82
%123 = OpAll %bool %122
OpBranch %118
%118 = OpLabel
%124 = OpPhi %bool %false %107 %123 %117
OpSelectionMerge %128 None
OpBranchConditional %124 %126 %127
%126 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%131 = OpLoad %v4float %129
OpStore %125 %131
OpBranch %128
%127 = OpLabel
%132 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%134 = OpLoad %v4float %132
OpStore %125 %134
OpBranch %128
%128 = OpLabel
%135 = OpLoad %v4float %125
OpReturnValue %135
OpFunctionEnd
