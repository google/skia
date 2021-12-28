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
OpName %fnGreen_h4bf2 "fnGreen_h4bf2"
OpName %S "S"
OpMemberName %S 0 "i"
OpName %fnRed_h4ifS "fnRed_h4ifS"
OpName %main "main"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %12 Binding 0
OpDecorate %12 DescriptorSet 0
OpDecorate %34 RelaxedPrecision
OpMemberDecorate %S 0 Offset 0
OpDecorate %46 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%12 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%17 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%21 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%25 = OpTypeFunction %v4float %_ptr_Function_bool %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%S = OpTypeStruct %int
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Function_S = OpTypePointer Function %S
%36 = OpTypeFunction %v4float %_ptr_Function_int %_ptr_Function_float %_ptr_Function_S
%int_1 = OpConstant %int 1
%47 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%true = OpConstantTrue %bool
%int_123 = OpConstant %int 123
%float_3_1400001 = OpConstant %float 3.1400001
%_entrypoint_v = OpFunction %void None %17
%18 = OpLabel
%22 = OpVariable %_ptr_Function_v2float Function
OpStore %22 %21
%24 = OpFunctionCall %v4float %main %22
OpStore %sk_FragColor %24
OpReturn
OpFunctionEnd
%fnGreen_h4bf2 = OpFunction %v4float None %25
%27 = OpFunctionParameter %_ptr_Function_bool
%28 = OpFunctionParameter %_ptr_Function_v2float
%29 = OpLabel
%30 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%34 = OpLoad %v4float %30
OpReturnValue %34
OpFunctionEnd
%fnRed_h4ifS = OpFunction %v4float None %36
%40 = OpFunctionParameter %_ptr_Function_int
%41 = OpFunctionParameter %_ptr_Function_float
%42 = OpFunctionParameter %_ptr_Function_S
%43 = OpLabel
%44 = OpAccessChain %_ptr_Uniform_v4float %12 %int_1
%46 = OpLoad %v4float %44
OpReturnValue %46
OpFunctionEnd
%main = OpFunction %v4float None %47
%48 = OpFunctionParameter %_ptr_Function_v2float
%49 = OpLabel
%54 = OpVariable %_ptr_Function_v4float Function
%60 = OpVariable %_ptr_Function_bool Function
%62 = OpVariable %_ptr_Function_v2float Function
%65 = OpVariable %_ptr_Function_int Function
%67 = OpVariable %_ptr_Function_float Function
%69 = OpVariable %_ptr_Function_S Function
%50 = OpAccessChain %_ptr_Uniform_v4float %12 %int_0
%51 = OpLoad %v4float %50
%52 = OpCompositeExtract %float %51 1
%53 = OpFUnordNotEqual %bool %52 %float_0
OpSelectionMerge %58 None
OpBranchConditional %53 %56 %57
%56 = OpLabel
OpStore %60 %true
%61 = OpLoad %v2float %48
OpStore %62 %61
%63 = OpFunctionCall %v4float %fnGreen_h4bf2 %60 %62
OpStore %54 %63
OpBranch %58
%57 = OpLabel
OpStore %65 %int_123
OpStore %67 %float_3_1400001
%68 = OpCompositeConstruct %S %int_0
OpStore %69 %68
%70 = OpFunctionCall %v4float %fnRed_h4ifS %65 %67 %69
OpStore %54 %70
OpBranch %58
%58 = OpLabel
%71 = OpLoad %v4float %54
OpReturnValue %71
OpFunctionEnd
