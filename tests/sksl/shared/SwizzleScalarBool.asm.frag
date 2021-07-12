OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "unknownInput"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %b "b"
OpName %b4 "b4"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %10 Binding 0
OpDecorate %10 DescriptorSet 0
OpDecorate %32 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_UniformBuffer = OpTypeStruct %float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%15 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%v2bool = OpTypeVector %bool 2
%false = OpConstantFalse %bool
%true = OpConstantTrue %bool
%float_1 = OpConstant %float 1
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
%b = OpVariable %_ptr_Function_bool Function
%b4 = OpVariable %_ptr_Function_v4bool Function
%28 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%32 = OpLoad %float %28
%33 = OpFUnordNotEqual %bool %32 %float_0
OpStore %b %33
%37 = OpLoad %bool %b
%38 = OpCompositeConstruct %v4bool %37 %37 %37 %37
OpStore %b4 %38
%39 = OpLoad %bool %b
%40 = OpCompositeConstruct %v2bool %39 %39
%42 = OpCompositeExtract %bool %40 0
%43 = OpCompositeExtract %bool %40 1
%46 = OpCompositeConstruct %v4bool %42 %43 %false %true
OpStore %b4 %46
%47 = OpLoad %bool %b
%48 = OpCompositeConstruct %v4bool %false %47 %true %false
OpStore %b4 %48
%49 = OpLoad %bool %b
%50 = OpLoad %bool %b
%51 = OpCompositeConstruct %v4bool %false %49 %false %50
OpStore %b4 %51
%52 = OpLoad %v4bool %b4
%53 = OpCompositeExtract %bool %52 0
%54 = OpSelect %float %53 %float_1 %float_0
%56 = OpCompositeExtract %bool %52 1
%57 = OpSelect %float %56 %float_1 %float_0
%58 = OpCompositeExtract %bool %52 2
%59 = OpSelect %float %58 %float_1 %float_0
%60 = OpCompositeExtract %bool %52 3
%61 = OpSelect %float %60 %float_1 %float_0
%62 = OpCompositeConstruct %v4float %54 %57 %59 %61
OpReturnValue %62
OpFunctionEnd
