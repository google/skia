OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "testArray"
OpMemberName %_UniformBuffer 1 "colorGreen"
OpMemberName %_UniformBuffer 2 "colorRed"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %index "index"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %_arr_float_int_5 ArrayStride 16
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 1 Offset 80
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 2 Offset 96
OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %55 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%int = OpTypeInt 32 1
%int_5 = OpConstant %int 5
%_arr_float_int_5 = OpTypeArray %float %int_5
%_UniformBuffer = OpTypeStruct %_arr_float_int_5 %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%18 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%26 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%_ptr_Uniform__arr_float_int_5 = OpTypePointer Uniform %_arr_float_int_5
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_1 = OpConstant %int 1
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %18
%19 = OpLabel
%23 = OpVariable %_ptr_Function_v2float Function
OpStore %23 %22
%25 = OpFunctionCall %v4float %main %23
OpStore %sk_FragColor %25
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %26
%27 = OpFunctionParameter %_ptr_Function_v2float
%28 = OpLabel
%index = OpVariable %_ptr_Function_int Function
OpStore %index %int_0
OpBranch %32
%32 = OpLabel
OpLoopMerge %36 %35 None
OpBranch %33
%33 = OpLabel
%37 = OpLoad %int %index
%38 = OpSLessThan %bool %37 %int_5
OpBranchConditional %38 %34 %36
%34 = OpLabel
%39 = OpAccessChain %_ptr_Uniform__arr_float_int_5 %10 %int_0
%41 = OpLoad %int %index
%42 = OpAccessChain %_ptr_Uniform_float %39 %41
%44 = OpLoad %float %42
%45 = OpLoad %int %index
%47 = OpIAdd %int %45 %int_1
%48 = OpConvertSToF %float %47
%49 = OpFOrdNotEqual %bool %44 %48
OpSelectionMerge %51 None
OpBranchConditional %49 %50 %51
%50 = OpLabel
%52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
%55 = OpLoad %v4float %52
OpReturnValue %55
%51 = OpLabel
OpBranch %35
%35 = OpLabel
%56 = OpLoad %int %index
%57 = OpIAdd %int %56 %int_1
OpStore %index %57
OpBranch %32
%36 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
%59 = OpLoad %v4float %58
OpReturnValue %59
OpFunctionEnd
