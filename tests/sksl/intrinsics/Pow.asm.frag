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
OpDecorate %31 RelaxedPrecision
OpDecorate %exponents RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %64 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
OpDecorate %82 RelaxedPrecision
OpDecorate %90 RelaxedPrecision
OpDecorate %91 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
OpDecorate %97 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
OpDecorate %105 RelaxedPrecision
OpDecorate %106 RelaxedPrecision
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
%55 = OpConstantComposite %v2float %float_2 %float_3
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%69 = OpConstantComposite %v3float %float_2 %float_3 %float_1
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%float_1_5625 = OpConstant %float 1.5625
%96 = OpConstantComposite %v2float %float_1_5625 %float_0
%104 = OpConstantComposite %v3float %float_1_5625 %float_0 %float_0_75
%112 = OpConstantComposite %v4float %float_1_5625 %float_0 %float_0_75 %float_3_375
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
%117 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %31
OpStore %exponents %37
%40 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%44 = OpLoad %v4float %40
%45 = OpCompositeExtract %float %44 0
%39 = OpExtInst %float %1 Pow %45 %float_2
%46 = OpLoad %v4float %expected
%47 = OpCompositeExtract %float %46 0
%48 = OpFOrdEqual %bool %39 %47
OpSelectionMerge %50 None
OpBranchConditional %48 %49 %50
%49 = OpLabel
%52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%53 = OpLoad %v4float %52
%54 = OpVectorShuffle %v2float %53 %53 0 1
%51 = OpExtInst %v2float %1 Pow %54 %55
%56 = OpLoad %v4float %expected
%57 = OpVectorShuffle %v2float %56 %56 0 1
%58 = OpFOrdEqual %v2bool %51 %57
%60 = OpAll %bool %58
OpBranch %50
%50 = OpLabel
%61 = OpPhi %bool %false %25 %60 %49
OpSelectionMerge %63 None
OpBranchConditional %61 %62 %63
%62 = OpLabel
%65 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%66 = OpLoad %v4float %65
%67 = OpVectorShuffle %v3float %66 %66 0 1 2
%64 = OpExtInst %v3float %1 Pow %67 %69
%70 = OpLoad %v4float %expected
%71 = OpVectorShuffle %v3float %70 %70 0 1 2
%72 = OpFOrdEqual %v3bool %64 %71
%74 = OpAll %bool %72
OpBranch %63
%63 = OpLabel
%75 = OpPhi %bool %false %50 %74 %62
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%79 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%80 = OpLoad %v4float %79
%81 = OpLoad %v4float %exponents
%78 = OpExtInst %v4float %1 Pow %80 %81
%82 = OpLoad %v4float %expected
%83 = OpFOrdEqual %v4bool %78 %82
%85 = OpAll %bool %83
OpBranch %77
%77 = OpLabel
%86 = OpPhi %bool %false %63 %85 %76
OpSelectionMerge %88 None
OpBranchConditional %86 %87 %88
%87 = OpLabel
%90 = OpLoad %v4float %expected
%91 = OpCompositeExtract %float %90 0
%92 = OpFOrdEqual %bool %float_1_5625 %91
OpBranch %88
%88 = OpLabel
%93 = OpPhi %bool %false %77 %92 %87
OpSelectionMerge %95 None
OpBranchConditional %93 %94 %95
%94 = OpLabel
%97 = OpLoad %v4float %expected
%98 = OpVectorShuffle %v2float %97 %97 0 1
%99 = OpFOrdEqual %v2bool %96 %98
%100 = OpAll %bool %99
OpBranch %95
%95 = OpLabel
%101 = OpPhi %bool %false %88 %100 %94
OpSelectionMerge %103 None
OpBranchConditional %101 %102 %103
%102 = OpLabel
%105 = OpLoad %v4float %expected
%106 = OpVectorShuffle %v3float %105 %105 0 1 2
%107 = OpFOrdEqual %v3bool %104 %106
%108 = OpAll %bool %107
OpBranch %103
%103 = OpLabel
%109 = OpPhi %bool %false %95 %108 %102
OpSelectionMerge %111 None
OpBranchConditional %109 %110 %111
%110 = OpLabel
%113 = OpLoad %v4float %expected
%114 = OpFOrdEqual %v4bool %112 %113
%115 = OpAll %bool %114
OpBranch %111
%111 = OpLabel
%116 = OpPhi %bool %false %103 %115 %110
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
