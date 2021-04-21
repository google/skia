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
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %test1 "test1"
OpName %test2 "test2"
OpName %test3 "test3"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %_arr_v2float_int_2 ArrayStride 16
OpDecorate %_arr_mat4v4float_int_1 ArrayStride 64
OpDecorate %77 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %80 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%int_2 = OpConstant %int 2
%_arr_v2float_int_2 = OpTypeArray %v2float %int_2
%_ptr_Function__arr_v2float_int_2 = OpTypePointer Function %_arr_v2float_int_2
%40 = OpConstantComposite %v2float %float_1 %float_2
%41 = OpConstantComposite %v2float %float_3 %float_4
%mat4v4float = OpTypeMatrix %v4float 4
%int_1 = OpConstant %int 1
%_arr_mat4v4float_int_1 = OpTypeArray %mat4v4float %int_1
%_ptr_Function__arr_mat4v4float_int_1 = OpTypePointer Function %_arr_mat4v4float_int_1
%float_16 = OpConstant %float 16
%int_3 = OpConstant %int 3
%_ptr_Function_float = OpTypePointer Function %float
%int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_24 = OpConstant %float 24
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
%test1 = OpVariable %_ptr_Function__arr_float_int_4 Function
%test2 = OpVariable %_ptr_Function__arr_v2float_int_2 Function
%test3 = OpVariable %_ptr_Function__arr_mat4v4float_int_1 Function
%71 = OpVariable %_ptr_Function_v4float Function
%35 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %test1 %35
%42 = OpCompositeConstruct %_arr_v2float_int_2 %40 %41
OpStore %test2 %42
%50 = OpCompositeConstruct %v4float %float_16 %float_0 %float_0 %float_0
%51 = OpCompositeConstruct %v4float %float_0 %float_16 %float_0 %float_0
%52 = OpCompositeConstruct %v4float %float_0 %float_0 %float_16 %float_0
%53 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_16
%49 = OpCompositeConstruct %mat4v4float %50 %51 %52 %53
%54 = OpCompositeConstruct %_arr_mat4v4float_int_1 %49
OpStore %test3 %54
%56 = OpAccessChain %_ptr_Function_float %test1 %int_3
%58 = OpLoad %float %56
%59 = OpAccessChain %_ptr_Function_v2float %test2 %int_1
%60 = OpLoad %v2float %59
%61 = OpCompositeExtract %float %60 1
%62 = OpFAdd %float %58 %61
%64 = OpAccessChain %_ptr_Function_v4float %test3 %int_0 %int_3
%66 = OpLoad %v4float %64
%67 = OpCompositeExtract %float %66 3
%68 = OpFAdd %float %62 %67
%70 = OpFOrdEqual %bool %68 %float_24
OpSelectionMerge %74 None
OpBranchConditional %70 %72 %73
%72 = OpLabel
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%77 = OpLoad %v4float %75
OpStore %71 %77
OpBranch %74
%73 = OpLabel
%78 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%79 = OpLoad %v4float %78
OpStore %71 %79
OpBranch %74
%74 = OpLabel
%80 = OpLoad %v4float %71
OpReturnValue %80
OpFunctionEnd
