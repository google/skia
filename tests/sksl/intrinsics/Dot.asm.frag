OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "inputA"
OpMemberName %_UniformBuffer 1 "inputB"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
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
OpMemberDecorate %_UniformBuffer 3 Offset 48
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %expected RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %73 RelaxedPrecision
OpDecorate %75 RelaxedPrecision
OpDecorate %100 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_5 = OpConstant %float 5
%float_17 = OpConstant %float 17
%float_38 = OpConstant %float 38
%float_70 = OpConstant %float 70
%32 = OpConstantComposite %v4float %float_5 %float_17 %float_38 %float_70
%false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%v3float = OpTypeVector %float 3
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%94 = OpVariable %_ptr_Function_v4float Function
OpStore %expected %32
%35 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%39 = OpLoad %v4float %35
%40 = OpCompositeExtract %float %39 0
%41 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%43 = OpLoad %v4float %41
%44 = OpCompositeExtract %float %43 0
%34 = OpFMul %float %40 %44
%45 = OpFOrdEqual %bool %34 %float_5
OpSelectionMerge %47 None
OpBranchConditional %45 %46 %47
%46 = OpLabel
%49 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%50 = OpLoad %v4float %49
%51 = OpVectorShuffle %v2float %50 %50 0 1
%52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%53 = OpLoad %v4float %52
%54 = OpVectorShuffle %v2float %53 %53 0 1
%48 = OpDot %float %51 %54
%55 = OpFOrdEqual %bool %48 %float_17
OpBranch %47
%47 = OpLabel
%56 = OpPhi %bool %false %25 %55 %46
OpSelectionMerge %58 None
OpBranchConditional %56 %57 %58
%57 = OpLabel
%60 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%61 = OpLoad %v4float %60
%62 = OpVectorShuffle %v3float %61 %61 0 1 2
%64 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%65 = OpLoad %v4float %64
%66 = OpVectorShuffle %v3float %65 %65 0 1 2
%59 = OpDot %float %62 %66
%67 = OpFOrdEqual %bool %59 %float_38
OpBranch %58
%58 = OpLabel
%68 = OpPhi %bool %false %47 %67 %57
OpSelectionMerge %70 None
OpBranchConditional %68 %69 %70
%69 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%73 = OpLoad %v4float %72
%74 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%75 = OpLoad %v4float %74
%71 = OpDot %float %73 %75
%76 = OpFOrdEqual %bool %71 %float_70
OpBranch %70
%70 = OpLabel
%77 = OpPhi %bool %false %58 %76 %69
OpSelectionMerge %79 None
OpBranchConditional %77 %78 %79
%78 = OpLabel
%80 = OpFOrdEqual %bool %float_5 %float_5
OpBranch %79
%79 = OpLabel
%81 = OpPhi %bool %false %70 %80 %78
OpSelectionMerge %83 None
OpBranchConditional %81 %82 %83
%82 = OpLabel
%84 = OpFOrdEqual %bool %float_17 %float_17
OpBranch %83
%83 = OpLabel
%85 = OpPhi %bool %false %79 %84 %82
OpSelectionMerge %87 None
OpBranchConditional %85 %86 %87
%86 = OpLabel
%88 = OpFOrdEqual %bool %float_38 %float_38
OpBranch %87
%87 = OpLabel
%89 = OpPhi %bool %false %83 %88 %86
OpSelectionMerge %91 None
OpBranchConditional %89 %90 %91
%90 = OpLabel
%92 = OpFOrdEqual %bool %float_70 %float_70
OpBranch %91
%91 = OpLabel
%93 = OpPhi %bool %false %87 %92 %90
OpSelectionMerge %97 None
OpBranchConditional %93 %95 %96
%95 = OpLabel
%98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%100 = OpLoad %v4float %98
OpStore %94 %100
OpBranch %97
%96 = OpLabel
%101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%103 = OpLoad %v4float %101
OpStore %94 %103
OpBranch %97
%97 = OpLabel
%104 = OpLoad %v4float %94
OpReturnValue %104
OpFunctionEnd
