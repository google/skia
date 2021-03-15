OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpMemberName %_UniformBuffer 1 "colorRed"
OpName %_entrypoint "_entrypoint"
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
OpDecorate %74 RelaxedPrecision
OpDecorate %76 RelaxedPrecision
OpDecorate %77 RelaxedPrecision
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
%18 = OpTypeFunction %v4float
%int = OpTypeInt 32 1
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%v2float = OpTypeVector %float 2
%int_2 = OpConstant %int 2
%_arr_v2float_int_2 = OpTypeArray %v2float %int_2
%_ptr_Function__arr_v2float_int_2 = OpTypePointer Function %_arr_v2float_int_2
%35 = OpConstantComposite %v2float %float_1 %float_2
%36 = OpConstantComposite %v2float %float_3 %float_4
%mat4v4float = OpTypeMatrix %v4float 4
%int_1 = OpConstant %int 1
%_arr_mat4v4float_int_1 = OpTypeArray %mat4v4float %int_1
%_ptr_Function__arr_mat4v4float_int_1 = OpTypePointer Function %_arr_mat4v4float_int_1
%float_16 = OpConstant %float 16
%float_0 = OpConstant %float 0
%int_3 = OpConstant %int 3
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int_0 = OpConstant %int 0
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_24 = OpConstant %float 24
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_entrypoint = OpFunction %void None %15
%16 = OpLabel
%17 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %17
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %18
%19 = OpLabel
%test1 = OpVariable %_ptr_Function__arr_float_int_4 Function
%test2 = OpVariable %_ptr_Function__arr_v2float_int_2 Function
%test3 = OpVariable %_ptr_Function__arr_mat4v4float_int_1 Function
%68 = OpVariable %_ptr_Function_v4float Function
%29 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %test1 %29
%37 = OpCompositeConstruct %_arr_v2float_int_2 %35 %36
OpStore %test2 %37
%46 = OpCompositeConstruct %v4float %float_16 %float_0 %float_0 %float_0
%47 = OpCompositeConstruct %v4float %float_0 %float_16 %float_0 %float_0
%48 = OpCompositeConstruct %v4float %float_0 %float_0 %float_16 %float_0
%49 = OpCompositeConstruct %v4float %float_0 %float_0 %float_0 %float_16
%44 = OpCompositeConstruct %mat4v4float %46 %47 %48 %49
%50 = OpCompositeConstruct %_arr_mat4v4float_int_1 %44
OpStore %test3 %50
%52 = OpAccessChain %_ptr_Function_float %test1 %int_3
%54 = OpLoad %float %52
%55 = OpAccessChain %_ptr_Function_v2float %test2 %int_1
%57 = OpLoad %v2float %55
%58 = OpCompositeExtract %float %57 1
%59 = OpFAdd %float %54 %58
%61 = OpAccessChain %_ptr_Function_v4float %test3 %int_0 %int_3
%63 = OpLoad %v4float %61
%64 = OpCompositeExtract %float %63 3
%65 = OpFAdd %float %59 %64
%67 = OpFOrdEqual %bool %65 %float_24
OpSelectionMerge %71 None
OpBranchConditional %67 %69 %70
%69 = OpLabel
%72 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
%74 = OpLoad %v4float %72
OpStore %68 %74
OpBranch %71
%70 = OpLabel
%75 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%76 = OpLoad %v4float %75
OpStore %68 %76
OpBranch %71
%71 = OpLabel
%77 = OpLoad %v4float %68
OpReturnValue %77
OpFunctionEnd
