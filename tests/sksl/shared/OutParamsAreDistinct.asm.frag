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
OpDecorate %x RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
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
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
%25 = OpTypeFunction %bool %_ptr_Function_float %_ptr_Function_float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%false = OpConstantFalse %bool
%37 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%out_params_are_distinct_bhh = OpFunction %bool None %25
%26 = OpFunctionParameter %_ptr_Function_float
%27 = OpFunctionParameter %_ptr_Function_float
%28 = OpLabel
OpStore %26 %float_1
OpStore %27 %float_2
%32 = OpFOrdEqual %bool %float_1 %float_1
OpSelectionMerge %34 None
OpBranchConditional %32 %33 %34
%33 = OpLabel
%35 = OpFOrdEqual %bool %float_2 %float_2
OpBranch %34
%34 = OpLabel
%36 = OpPhi %bool %false %28 %35 %33
OpReturnValue %36
OpFunctionEnd
%main = OpFunction %v4float None %37
%38 = OpFunctionParameter %_ptr_Function_v2float
%39 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%41 = OpVariable %_ptr_Function_float Function
%42 = OpVariable %_ptr_Function_float Function
%46 = OpVariable %_ptr_Function_v4float Function
OpStore %x %float_0
%43 = OpFunctionCall %bool %out_params_are_distinct_bhh %41 %42
%44 = OpLoad %float %41
OpStore %x %44
%45 = OpLoad %float %42
OpStore %x %45
OpSelectionMerge %50 None
OpBranchConditional %43 %48 %49
%48 = OpLabel
%51 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%55 = OpLoad %v4float %51
OpStore %46 %55
OpBranch %50
%49 = OpLabel
%56 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
%58 = OpLoad %v4float %56
OpStore %46 %58
OpBranch %50
%50 = OpLabel
%59 = OpLoad %v4float %46
OpReturnValue %59
OpFunctionEnd
