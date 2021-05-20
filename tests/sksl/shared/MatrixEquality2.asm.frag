OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpMemberName %_UniformBuffer 2 "testMatrix2x2"
OpMemberName %_UniformBuffer 3 "testMatrix3x3"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %_0_ok "_0_ok"
OpName %_1_tempMatL "_1_tempMatL"
OpName %_2_tempMatR "_2_tempMatR"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 32
OpMemberDecorate %_UniformBuffer 2 ColMajor
OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 3 Offset 64
OpMemberDecorate %_UniformBuffer 3 ColMajor
OpMemberDecorate %_UniformBuffer 3 MatrixStride 16
OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %_1_tempMatL RelaxedPrecision
OpDecorate %_2_tempMatR RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %65 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
OpDecorate %81 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float %mat3v3float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
%false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
%int = OpTypeInt 32 1
%int_2 = OpConstant %int 2
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%int_0 = OpConstant %int 0
%v2bool = OpTypeVector %bool 2
%int_1 = OpConstant %int 1
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint_v = OpFunction %void None %19
%20 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%_0_ok = OpVariable %_ptr_Function_bool Function
%_1_tempMatL = OpVariable %_ptr_Function_mat2v2float Function
%_2_tempMatR = OpVariable %_ptr_Function_mat2v2float Function
%71 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%36 = OpLoad %bool %_0_ok
OpSelectionMerge %38 None
OpBranchConditional %36 %37 %38
%37 = OpLabel
%39 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
%43 = OpLoad %mat2v2float %39
OpStore %_1_tempMatL %43
%49 = OpCompositeConstruct %v2float %float_1 %float_2
%50 = OpCompositeConstruct %v2float %float_3 %float_4
%48 = OpCompositeConstruct %mat2v2float %49 %50
OpStore %_2_tempMatR %48
%52 = OpAccessChain %_ptr_Function_v2float %_1_tempMatL %int_0
%53 = OpLoad %v2float %52
%54 = OpAccessChain %_ptr_Function_v2float %_2_tempMatR %int_0
%55 = OpLoad %v2float %54
%56 = OpFOrdEqual %v2bool %53 %55
%58 = OpAll %bool %56
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%62 = OpAccessChain %_ptr_Function_v2float %_1_tempMatL %int_1
%63 = OpLoad %v2float %62
%64 = OpAccessChain %_ptr_Function_v2float %_2_tempMatR %int_1
%65 = OpLoad %v2float %64
%66 = OpFOrdEqual %v2bool %63 %65
%67 = OpAll %bool %66
OpBranch %60
%60 = OpLabel
%68 = OpPhi %bool %false %37 %67 %59
OpBranch %38
%38 = OpLabel
%69 = OpPhi %bool %false %28 %68 %60
OpStore %_0_ok %69
%70 = OpLoad %bool %_0_ok
OpSelectionMerge %75 None
OpBranchConditional %70 %73 %74
%73 = OpLabel
%76 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%78 = OpLoad %v4float %76
OpStore %71 %78
OpBranch %75
%74 = OpLabel
%79 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%80 = OpLoad %v4float %79
OpStore %71 %80
OpBranch %75
%75 = OpLabel
%81 = OpLoad %v4float %71
OpReturnValue %81
OpFunctionEnd
