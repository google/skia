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
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %85 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %87 RelaxedPrecision
OpDecorate %93 RelaxedPrecision
OpDecorate %94 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %101 RelaxedPrecision
OpDecorate %111 RelaxedPrecision
OpDecorate %114 RelaxedPrecision
OpDecorate %115 RelaxedPrecision
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
%float_n1 = OpConstant %float -1
%float_1 = OpConstant %float 1
%30 = OpConstantComposite %v4float %float_n1 %float_0 %float_1 %float_1
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%85 = OpConstantComposite %v2float %float_n1 %float_0
%93 = OpConstantComposite %v3float %float_n1 %float_0 %float_1
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
%105 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %30
%33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%37 = OpLoad %v4float %33
%38 = OpCompositeExtract %float %37 0
%32 = OpExtInst %float %1 FSign %38
%39 = OpLoad %v4float %expected
%40 = OpCompositeExtract %float %39 0
%41 = OpFOrdEqual %bool %32 %40
OpSelectionMerge %43 None
OpBranchConditional %41 %42 %43
%42 = OpLabel
%45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%46 = OpLoad %v4float %45
%47 = OpVectorShuffle %v2float %46 %46 0 1
%44 = OpExtInst %v2float %1 FSign %47
%48 = OpLoad %v4float %expected
%49 = OpVectorShuffle %v2float %48 %48 0 1
%50 = OpFOrdEqual %v2bool %44 %49
%52 = OpAll %bool %50
OpBranch %43
%43 = OpLabel
%53 = OpPhi %bool %false %25 %52 %42
OpSelectionMerge %55 None
OpBranchConditional %53 %54 %55
%54 = OpLabel
%57 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%58 = OpLoad %v4float %57
%59 = OpVectorShuffle %v3float %58 %58 0 1 2
%56 = OpExtInst %v3float %1 FSign %59
%61 = OpLoad %v4float %expected
%62 = OpVectorShuffle %v3float %61 %61 0 1 2
%63 = OpFOrdEqual %v3bool %56 %62
%65 = OpAll %bool %63
OpBranch %55
%55 = OpLabel
%66 = OpPhi %bool %false %43 %65 %54
OpSelectionMerge %68 None
OpBranchConditional %66 %67 %68
%67 = OpLabel
%70 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%71 = OpLoad %v4float %70
%69 = OpExtInst %v4float %1 FSign %71
%72 = OpLoad %v4float %expected
%73 = OpFOrdEqual %v4bool %69 %72
%75 = OpAll %bool %73
OpBranch %68
%68 = OpLabel
%76 = OpPhi %bool %false %55 %75 %67
OpSelectionMerge %78 None
OpBranchConditional %76 %77 %78
%77 = OpLabel
%79 = OpLoad %v4float %expected
%80 = OpCompositeExtract %float %79 0
%81 = OpFOrdEqual %bool %float_n1 %80
OpBranch %78
%78 = OpLabel
%82 = OpPhi %bool %false %68 %81 %77
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%86 = OpLoad %v4float %expected
%87 = OpVectorShuffle %v2float %86 %86 0 1
%88 = OpFOrdEqual %v2bool %85 %87
%89 = OpAll %bool %88
OpBranch %84
%84 = OpLabel
%90 = OpPhi %bool %false %78 %89 %83
OpSelectionMerge %92 None
OpBranchConditional %90 %91 %92
%91 = OpLabel
%94 = OpLoad %v4float %expected
%95 = OpVectorShuffle %v3float %94 %94 0 1 2
%96 = OpFOrdEqual %v3bool %93 %95
%97 = OpAll %bool %96
OpBranch %92
%92 = OpLabel
%98 = OpPhi %bool %false %84 %97 %91
OpSelectionMerge %100 None
OpBranchConditional %98 %99 %100
%99 = OpLabel
%101 = OpLoad %v4float %expected
%102 = OpFOrdEqual %v4bool %30 %101
%103 = OpAll %bool %102
OpBranch %100
%100 = OpLabel
%104 = OpPhi %bool %false %92 %103 %99
OpSelectionMerge %108 None
OpBranchConditional %104 %106 %107
%106 = OpLabel
%109 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%111 = OpLoad %v4float %109
OpStore %105 %111
OpBranch %108
%107 = OpLabel
%112 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%114 = OpLoad %v4float %112
OpStore %105 %114
OpBranch %108
%108 = OpLabel
%115 = OpLoad %v4float %105
OpReturnValue %115
OpFunctionEnd
