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
OpName %out_params_are_distinct_bhh "out_params_are_distinct_bhh"
OpName %main "main"
OpName %x "x"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %x RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
%24 = OpTypeFunction %bool %_ptr_Function_float %_ptr_Function_float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%39 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%out_params_are_distinct_bhh = OpFunction %bool None %24
%26 = OpFunctionParameter %_ptr_Function_float
%27 = OpFunctionParameter %_ptr_Function_float
%28 = OpLabel
OpStore %26 %float_1
OpStore %27 %float_2
%32 = OpLoad %float %26
%33 = OpFOrdEqual %bool %32 %float_1
OpSelectionMerge %35 None
OpBranchConditional %33 %34 %35
%34 = OpLabel
%36 = OpLoad %float %27
%37 = OpFOrdEqual %bool %36 %float_2
OpBranch %35
%35 = OpLabel
%38 = OpPhi %bool %false %28 %37 %34
OpReturnValue %38
OpFunctionEnd
%main = OpFunction %v4float None %39
%40 = OpFunctionParameter %_ptr_Function_v2float
%41 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%43 = OpVariable %_ptr_Function_float Function
%44 = OpVariable %_ptr_Function_float Function
%48 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_0
%45 = OpFunctionCall %bool %out_params_are_distinct_bhh %43 %44
%46 = OpLoad %float %43
OpStore %x %46
%47 = OpLoad %float %44
OpStore %x %47
OpSelectionMerge %52 None
OpBranchConditional %45 %50 %51
%50 = OpLabel
%53 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%57 = OpLoad %v4float %53
OpStore %48 %57
OpBranch %52
%51 = OpLabel
%58 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%60 = OpLoad %v4float %58
OpStore %48 %60
OpBranch %52
%52 = OpLabel
%61 = OpLoad %v4float %48
OpReturnValue %61
OpFunctionEnd
