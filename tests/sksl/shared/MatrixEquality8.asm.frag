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
OpDecorate %33 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %70 RelaxedPrecision
OpDecorate %74 RelaxedPrecision
OpDecorate %78 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %88 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
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
%false = OpConstantFalse %bool
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
%int = OpTypeInt 32 1
%int_3 = OpConstant %int 3
%int_0 = OpConstant %int 0
%_ptr_Uniform_v3float = OpTypePointer Uniform %v3float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%47 = OpConstantComposite %v3float %float_1 %float_2 %float_3
%v3bool = OpTypeVector %bool 3
%int_1 = OpConstant %int 1
%float_4 = OpConstant %float 4
%float_5 = OpConstant %float 5
%float_6 = OpConstant %float 6
%61 = OpConstantComposite %v3float %float_4 %float_5 %float_6
%int_2 = OpConstant %int 2
%float_7 = OpConstant %float 7
%float_8 = OpConstant %float 8
%float_9 = OpConstant %float 9
%74 = OpConstantComposite %v3float %float_7 %float_8 %float_9
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
%79 = OpVariable %_ptr_Function_v4float Function
OpStore %_0_ok %true
%33 = OpLoad %bool %_0_ok
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%36 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%41 = OpAccessChain %_ptr_Uniform_v3float %36 %int_0
%43 = OpLoad %v3float %41
%48 = OpFOrdEqual %v3bool %43 %47
%50 = OpAll %bool %48
OpBranch %35
%35 = OpLabel
%51 = OpPhi %bool %false %28 %50 %34
OpSelectionMerge %53 None
OpBranchConditional %51 %52 %53
%52 = OpLabel
%54 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%56 = OpAccessChain %_ptr_Uniform_v3float %54 %int_1
%57 = OpLoad %v3float %56
%62 = OpFOrdEqual %v3bool %57 %61
%63 = OpAll %bool %62
OpBranch %53
%53 = OpLabel
%64 = OpPhi %bool %false %35 %63 %52
OpSelectionMerge %66 None
OpBranchConditional %64 %65 %66
%65 = OpLabel
%67 = OpAccessChain %_ptr_Uniform_mat3v3float %10 %int_3
%69 = OpAccessChain %_ptr_Uniform_v3float %67 %int_2
%70 = OpLoad %v3float %69
%75 = OpFOrdEqual %v3bool %70 %74
%76 = OpAll %bool %75
OpBranch %66
%66 = OpLabel
%77 = OpPhi %bool %false %53 %76 %65
OpStore %_0_ok %77
%78 = OpLoad %bool %_0_ok
OpSelectionMerge %83 None
OpBranchConditional %78 %81 %82
%81 = OpLabel
%84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%86 = OpLoad %v4float %84
OpStore %79 %86
OpBranch %83
%82 = OpLabel
%87 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%88 = OpLoad %v4float %87
OpStore %79 %88
OpBranch %83
%83 = OpLabel
%89 = OpLoad %v4float %79
OpReturnValue %89
OpFunctionEnd
