OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testInput"
OpMemberName %_UniformBuffer 1 "testMatrix2x2"
OpMemberName %_UniformBuffer 2 "colorGreen"
OpMemberName %_UniformBuffer 3 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %inputVal "inputVal"
OpName %expectedB "expectedB"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 ColMajor
OpMemberDecorate %_UniformBuffer 1 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 Offset 48
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %99 RelaxedPrecision
OpDecorate %102 RelaxedPrecision
OpDecorate %103 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %float %mat2v2float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%float_n1 = OpConstant %float -1
%41 = OpConstantComposite %v4float %float_1 %float_1 %float_n1 %float_n1
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%int_1065353216 = OpConstant %int 1065353216
%int_1073741824 = OpConstant %int 1073741824
%int_n1069547520 = OpConstant %int -1069547520
%int_n1065353216 = OpConstant %int -1065353216
%50 = OpConstantComposite %v4int %int_1065353216 %int_1073741824 %int_n1069547520 %int_n1065353216
%false = OpConstantFalse %bool
%v2int = OpTypeVector %int 2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%v3int = OpTypeVector %int 3
%v3bool = OpTypeVector %bool 3
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_2 = OpConstant %int 2
%int_3 = OpConstant %int 3
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
%inputVal = OpVariable %_ptr_Function_v4float Function
%expectedB = OpVariable %_ptr_Function_v4int Function
%92 = OpVariable %_ptr_Function_v4float Function
%29 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_1
%33 = OpLoad %mat2v2float %29
%34 = OpCompositeExtract %float %33 0 0
%35 = OpCompositeExtract %float %33 0 1
%36 = OpCompositeExtract %float %33 1 0
%37 = OpCompositeExtract %float %33 1 1
%38 = OpCompositeConstruct %v4float %34 %35 %36 %37
%42 = OpFMul %v4float %38 %41
OpStore %inputVal %42
OpStore %expectedB %50
%52 = OpLoad %v4float %inputVal
%53 = OpCompositeExtract %float %52 0
%55 = OpLoad %v4int %expectedB
%56 = OpCompositeExtract %int %55 0
%54 = OpBitcast %float %56
%57 = OpFOrdEqual %bool %53 %54
OpSelectionMerge %59 None
OpBranchConditional %57 %58 %59
%58 = OpLabel
%60 = OpLoad %v4float %inputVal
%61 = OpVectorShuffle %v2float %60 %60 0 1
%63 = OpLoad %v4int %expectedB
%64 = OpVectorShuffle %v2int %63 %63 0 1
%62 = OpBitcast %v2float %64
%66 = OpFOrdEqual %v2bool %61 %62
%68 = OpAll %bool %66
OpBranch %59
%59 = OpLabel
%69 = OpPhi %bool %false %26 %68 %58
OpSelectionMerge %71 None
OpBranchConditional %69 %70 %71
%70 = OpLabel
%72 = OpLoad %v4float %inputVal
%73 = OpVectorShuffle %v3float %72 %72 0 1 2
%76 = OpLoad %v4int %expectedB
%77 = OpVectorShuffle %v3int %76 %76 0 1 2
%75 = OpBitcast %v3float %77
%79 = OpFOrdEqual %v3bool %73 %75
%81 = OpAll %bool %79
OpBranch %71
%71 = OpLabel
%82 = OpPhi %bool %false %59 %81 %70
OpSelectionMerge %84 None
OpBranchConditional %82 %83 %84
%83 = OpLabel
%85 = OpLoad %v4float %inputVal
%87 = OpLoad %v4int %expectedB
%86 = OpBitcast %v4float %87
%88 = OpFOrdEqual %v4bool %85 %86
%90 = OpAll %bool %88
OpBranch %84
%84 = OpLabel
%91 = OpPhi %bool %false %71 %90 %83
OpSelectionMerge %95 None
OpBranchConditional %91 %93 %94
%93 = OpLabel
%96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%99 = OpLoad %v4float %96
OpStore %92 %99
OpBranch %95
%94 = OpLabel
%100 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
%102 = OpLoad %v4float %100
OpStore %92 %102
OpBranch %95
%95 = OpLabel
%103 = OpLoad %v4float %92
OpReturnValue %103
OpFunctionEnd
