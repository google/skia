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
OpMemberName %_UniformBuffer 3 "u_skRTFlip"
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
OpMemberDecorate %_UniformBuffer 3 Offset 16384
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %expected RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
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
%v2float = OpTypeVector %float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%29 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%int_3 = OpConstant %int 3
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%float_1 = OpConstant %float 1
%104 = OpConstantComposite %v2float %float_1 %float_1
%118 = OpConstantComposite %v2float %float_0 %float_1
%int_1 = OpConstant %int 1
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %24
%25 = OpFunctionParameter %_ptr_Function_v2float
%26 = OpLabel
%expected = OpVariable %_ptr_Function_v4float Function
%122 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %29
%32 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%36 = OpLoad %v4float %32
%37 = OpCompositeExtract %float %36 0
%31 = OpDPdx %float %37
%38 = OpLoad %v4float %expected
%39 = OpCompositeExtract %float %38 0
%40 = OpFOrdEqual %bool %31 %39
OpSelectionMerge %42 None
OpBranchConditional %40 %41 %42
%41 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%45 = OpLoad %v4float %44
%46 = OpVectorShuffle %v2float %45 %45 0 1
%43 = OpDPdx %v2float %46
%47 = OpLoad %v4float %expected
%48 = OpVectorShuffle %v2float %47 %47 0 1
%49 = OpFOrdEqual %v2bool %43 %48
%51 = OpAll %bool %49
OpBranch %42
%42 = OpLabel
%52 = OpPhi %bool %false %26 %51 %41
OpSelectionMerge %54 None
OpBranchConditional %52 %53 %54
%53 = OpLabel
%56 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%57 = OpLoad %v4float %56
%58 = OpVectorShuffle %v3float %57 %57 0 1 2
%55 = OpDPdx %v3float %58
%60 = OpLoad %v4float %expected
%61 = OpVectorShuffle %v3float %60 %60 0 1 2
%62 = OpFOrdEqual %v3bool %55 %61
%64 = OpAll %bool %62
OpBranch %54
%54 = OpLabel
%65 = OpPhi %bool %false %42 %64 %53
OpSelectionMerge %67 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
%69 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%70 = OpLoad %v4float %69
%68 = OpDPdx %v4float %70
%71 = OpLoad %v4float %expected
%72 = OpFOrdEqual %v4bool %68 %71
%74 = OpAll %bool %72
OpBranch %67
%67 = OpLabel
%75 = OpPhi %bool %false %54 %74 %66
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%80 = OpLoad %v2float %25
%81 = OpVectorShuffle %v2float %80 %80 0 0
%79 = OpDPdy %v2float %81
%83 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%85 = OpLoad %v2float %83
%86 = OpCompositeExtract %float %85 1
%87 = OpCompositeConstruct %v2float %86 %86
%88 = OpFMul %v2float %79 %87
%78 = OpExtInst %v2float %1 FSign %88
%89 = OpFOrdEqual %v2bool %78 %20
%90 = OpAll %bool %89
OpBranch %77
%77 = OpLabel
%91 = OpPhi %bool %false %67 %90 %76
OpSelectionMerge %93 None
OpBranchConditional %91 %92 %93
%92 = OpLabel
%96 = OpLoad %v2float %25
%97 = OpVectorShuffle %v2float %96 %96 1 1
%95 = OpDPdy %v2float %97
%98 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%99 = OpLoad %v2float %98
%100 = OpCompositeExtract %float %99 1
%101 = OpCompositeConstruct %v2float %100 %100
%102 = OpFMul %v2float %95 %101
%94 = OpExtInst %v2float %1 FSign %102
%105 = OpFOrdEqual %v2bool %94 %104
%106 = OpAll %bool %105
OpBranch %93
%93 = OpLabel
%107 = OpPhi %bool %false %77 %106 %92
OpSelectionMerge %109 None
OpBranchConditional %107 %108 %109
%108 = OpLabel
%112 = OpLoad %v2float %25
%111 = OpDPdy %v2float %112
%113 = OpAccessChain %_ptr_Uniform_v2float %11 %int_3
%114 = OpLoad %v2float %113
%115 = OpCompositeExtract %float %114 1
%116 = OpCompositeConstruct %v2float %115 %115
%117 = OpFMul %v2float %111 %116
%110 = OpExtInst %v2float %1 FSign %117
%119 = OpFOrdEqual %v2bool %110 %118
%120 = OpAll %bool %119
OpBranch %109
%109 = OpLabel
%121 = OpPhi %bool %false %93 %120 %108
OpSelectionMerge %125 None
OpBranchConditional %121 %123 %124
%123 = OpLabel
%126 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%128 = OpLoad %v4float %126
OpStore %122 %128
OpBranch %125
%124 = OpLabel
%129 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
%131 = OpLoad %v4float %129
OpStore %122 %131
OpBranch %125
%125 = OpLabel
%132 = OpLoad %v4float %122
OpReturnValue %132
OpFunctionEnd
