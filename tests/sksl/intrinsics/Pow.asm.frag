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
OpName %exponents "exponents"
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
OpDecorate %exponents RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %112 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
OpDecorate %116 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_n1_5625 = OpConstant %float -1.5625
%float_0_75 = OpConstant %float 0.75
%float_3_375 = OpConstant %float 3.375
%31 = OpConstantComposite %v4float %float_n1_5625 %float_0 %float_0_75 %float_3_375
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_1 = OpConstant %float 1
%float_1_5 = OpConstant %float 1.5
%37 = OpConstantComposite %v4float %float_2 %float_3 %float_1 %float_1_5
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%53 = OpConstantComposite %v2float %float_2 %float_3
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%66 = OpConstantComposite %v3float %float_2 %float_3 %float_1
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_1_5625 = OpConstant %float 1.5625
%88 = OpConstantComposite %v2float %float_1_5625 %float_0
%95 = OpConstantComposite %v3float %float_1_5625 %float_0 %float_0_75
%102 = OpConstantComposite %v4float %float_1_5625 %float_0 %float_0_75 %float_3_375
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
%exponents = OpVariable %_ptr_Function_v4float Function
%106 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %31
OpStore %exponents %37
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %40
%45 = OpCompositeExtract %float %44 0
%39 = OpExtInst %float %1 Pow %45 %float_2
%46 = OpFOrdEqual %bool %39 %float_n1_5625
OpSelectionMerge %48 None
OpBranchConditional %46 %47 %48
%47 = OpLabel
%50 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%51 = OpLoad %v4float %50
%52 = OpVectorShuffle %v2float %51 %51 0 1
%49 = OpExtInst %v2float %1 Pow %52 %53
%54 = OpVectorShuffle %v2float %31 %31 0 1
%55 = OpFOrdEqual %v2bool %49 %54
%57 = OpAll %bool %55
OpBranch %48
%48 = OpLabel
%58 = OpPhi %bool %false %25 %57 %47
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%63 = OpLoad %v4float %62
%64 = OpVectorShuffle %v3float %63 %63 0 1 2
%61 = OpExtInst %v3float %1 Pow %64 %66
%67 = OpVectorShuffle %v3float %31 %31 0 1 2
%68 = OpFOrdEqual %v3bool %61 %67
%70 = OpAll %bool %68
OpBranch %60
%60 = OpLabel
%71 = OpPhi %bool %false %48 %70 %59
OpSelectionMerge %73 None
OpBranchConditional %71 %72 %73
%72 = OpLabel
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%76 = OpLoad %v4float %75
%74 = OpExtInst %v4float %1 Pow %76 %37
%77 = OpFOrdEqual %v4bool %74 %31
%79 = OpAll %bool %77
OpBranch %73
%73 = OpLabel
%80 = OpPhi %bool %false %60 %79 %72
OpSelectionMerge %82 None
OpBranchConditional %80 %81 %82
%81 = OpLabel
%84 = OpFOrdEqual %bool %float_1_5625 %float_n1_5625
OpBranch %82
%82 = OpLabel
%85 = OpPhi %bool %false %73 %84 %81
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%89 = OpVectorShuffle %v2float %31 %31 0 1
%90 = OpFOrdEqual %v2bool %88 %89
%91 = OpAll %bool %90
OpBranch %87
%87 = OpLabel
%92 = OpPhi %bool %false %82 %91 %86
OpSelectionMerge %94 None
OpBranchConditional %92 %93 %94
%93 = OpLabel
%96 = OpVectorShuffle %v3float %31 %31 0 1 2
%97 = OpFOrdEqual %v3bool %95 %96
%98 = OpAll %bool %97
OpBranch %94
%94 = OpLabel
%99 = OpPhi %bool %false %87 %98 %93
OpSelectionMerge %101 None
OpBranchConditional %99 %100 %101
%100 = OpLabel
%103 = OpFOrdEqual %v4bool %102 %31
%104 = OpAll %bool %103
OpBranch %101
%101 = OpLabel
%105 = OpPhi %bool %false %94 %104 %100
OpSelectionMerge %109 None
OpBranchConditional %105 %107 %108
%107 = OpLabel
%110 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%112 = OpLoad %v4float %110
OpStore %106 %112
OpBranch %109
%108 = OpLabel
%113 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%115 = OpLoad %v4float %113
OpStore %106 %115
OpBranch %109
%109 = OpLabel
%116 = OpLoad %v4float %106
OpReturnValue %116
OpFunctionEnd
