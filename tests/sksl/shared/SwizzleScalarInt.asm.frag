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
OpName %i "i"
OpName %i4 "i4"
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
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
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
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Uniform_float = OpTypePointer Uniform %float
%int_0 = OpConstant %int 0
%v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
%v2int = OpTypeVector %int 2
%int_1 = OpConstant %int 1
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
%i = OpVariable %_ptr_Function_int Function
%i4 = OpVariable %_ptr_Function_v4int Function
%29 = OpAccessChain %_ptr_Uniform_float %10 %int_0
%32 = OpLoad %float %29
%33 = OpConvertFToS %int %32
OpStore %i %33
%37 = OpLoad %int %i
%38 = OpCompositeConstruct %v4int %37 %37 %37 %37
OpStore %i4 %38
%39 = OpLoad %int %i
%40 = OpCompositeConstruct %v2int %39 %39
%42 = OpCompositeExtract %int %40 0
%43 = OpCompositeExtract %int %40 1
%45 = OpCompositeConstruct %v4int %42 %43 %int_0 %int_1
OpStore %i4 %45
%46 = OpLoad %int %i
%47 = OpCompositeConstruct %v4int %int_0 %46 %int_1 %int_0
OpStore %i4 %47
%48 = OpLoad %int %i
%49 = OpLoad %int %i
%50 = OpCompositeConstruct %v4int %int_0 %48 %int_0 %49
OpStore %i4 %50
%51 = OpLoad %v4int %i4
%52 = OpCompositeExtract %int %51 0
%53 = OpConvertSToF %float %52
%54 = OpCompositeExtract %int %51 1
%55 = OpConvertSToF %float %54
%56 = OpCompositeExtract %int %51 2
%57 = OpConvertSToF %float %56
%58 = OpCompositeExtract %int %51 3
%59 = OpConvertSToF %float %58
%60 = OpCompositeConstruct %v4float %53 %55 %57 %59
OpReturnValue %60
OpFunctionEnd
