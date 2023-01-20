OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testMatrix2x2"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %inputVal "inputVal"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 ColMajor
OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
OpMemberDecorate %_UniformBuffer 1 Offset 32
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 48
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %50 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
OpDecorate %96 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %mat2v2float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%24 = OpTypeFunction %v4float %_ptr_Function_v2float
%float_n1 = OpConstant %float -1
%float_n4 = OpConstant %float -4
%float_n16 = OpConstant %float -16
%float_n64 = OpConstant %float -64
%32 = OpConstantComposite %v4float %float_n1 %float_n4 %float_n16 %float_n64
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%float_2 = OpConstant %float 2
%float_6 = OpConstant %float 6
%float_12 = OpConstant %float 12
%49 = OpConstantComposite %v4float %float_0 %float_2 %float_6 %float_12
%false = OpConstantFalse %bool
%float_1 = OpConstant %float 1
%60 = OpConstantComposite %v2float %float_1 %float_2
%v2bool = OpTypeVector %bool 2
%v3float = OpTypeVector %float 3
%float_3 = OpConstant %float 3
%71 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%v3bool = OpTypeVector %bool 3
%float_4 = OpConstant %float 4
%80 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
%v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%inputVal = OpVariable %_ptr_Function_v4float Function
%85 = OpVariable %_ptr_Function_v4float Function
%27 = OpExtInst %v4float %1 Sqrt %32
%33 = OpVectorShuffle %v2float %27 %27 0 1
OpStore %25 %33
%36 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_0
%40 = OpLoad %mat2v2float %36
%41 = OpCompositeExtract %float %40 0 0
%42 = OpCompositeExtract %float %40 0 1
%43 = OpCompositeExtract %float %40 1 0
%44 = OpCompositeExtract %float %40 1 1
%45 = OpCompositeConstruct %v4float %41 %42 %43 %44
%50 = OpFAdd %v4float %45 %49
OpStore %inputVal %50
%53 = OpCompositeExtract %float %50 0
%52 = OpExtInst %float %1 Sqrt %53
%55 = OpFOrdEqual %bool %52 %float_1
OpSelectionMerge %57 None
OpBranchConditional %55 %56 %57
%56 = OpLabel
%59 = OpVectorShuffle %v2float %50 %50 0 1
%58 = OpExtInst %v2float %1 Sqrt %59
%61 = OpFOrdEqual %v2bool %58 %60
%63 = OpAll %bool %61
OpBranch %57
%57 = OpLabel
%64 = OpPhi %bool %false %26 %63 %56
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%68 = OpVectorShuffle %v3float %50 %50 0 1 2
%67 = OpExtInst %v3float %1 Sqrt %68
%72 = OpFOrdEqual %v3bool %67 %71
%74 = OpAll %bool %72
OpBranch %66
%66 = OpLabel
%75 = OpPhi %bool %false %57 %74 %65
OpSelectionMerge %77 None
OpBranchConditional %75 %76 %77
%76 = OpLabel
%78 = OpExtInst %v4float %1 Sqrt %50
%81 = OpFOrdEqual %v4bool %78 %80
%83 = OpAll %bool %81
OpBranch %77
%77 = OpLabel
%84 = OpPhi %bool %false %66 %83 %76
OpSelectionMerge %88 None
OpBranchConditional %84 %86 %87
%86 = OpLabel
%89 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%92 = OpLoad %v4float %89
OpStore %85 %92
OpBranch %88
%87 = OpLabel
%93 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%95 = OpLoad %v4float %93
OpStore %85 %95
OpBranch %88
%88 = OpLabel
%96 = OpLoad %v4float %85
OpReturnValue %96
OpFunctionEnd
