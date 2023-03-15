OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %SomeData "SomeData"
OpMemberName %SomeData 0 "a"
OpMemberName %SomeData 1 "b"
OpName %storageBuffer "storageBuffer"
OpMemberName %storageBuffer 0 "offset"
OpMemberName %storageBuffer 1 "inputData"
OpName %outputBuffer "outputBuffer"
OpMemberName %outputBuffer 0 "outputData"
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpMemberDecorate %SomeData 0 Offset 0
OpMemberDecorate %SomeData 1 Offset 16
OpDecorate %_runtimearr_SomeData ArrayStride 32
OpMemberDecorate %storageBuffer 0 Offset 0
OpMemberDecorate %storageBuffer 1 Offset 16
OpMemberDecorate %storageBuffer 1 RelaxedPrecision
OpDecorate %storageBuffer BufferBlock
OpDecorate %3 Binding 0
OpDecorate %3 DescriptorSet 0
OpMemberDecorate %outputBuffer 0 Offset 0
OpMemberDecorate %outputBuffer 0 RelaxedPrecision
OpDecorate %outputBuffer BufferBlock
OpDecorate %12 Binding 1
OpDecorate %12 DescriptorSet 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %40 RelaxedPrecision
%uint = OpTypeInt 32 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%v2float = OpTypeVector %float 2
%SomeData = OpTypeStruct %v4float %v2float
%_runtimearr_SomeData = OpTypeRuntimeArray %SomeData
%storageBuffer = OpTypeStruct %uint %_runtimearr_SomeData
%_ptr_Uniform_storageBuffer = OpTypePointer Uniform %storageBuffer
%3 = OpVariable %_ptr_Uniform_storageBuffer Uniform
%outputBuffer = OpTypeStruct %_runtimearr_SomeData
%_ptr_Uniform_outputBuffer = OpTypePointer Uniform %outputBuffer
%12 = OpVariable %_ptr_Uniform_outputBuffer Uniform
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%void = OpTypeVoid
%22 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%25 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%29 = OpTypeFunction %v4float %_ptr_Function_v2float
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_Uniform_SomeData = OpTypePointer Uniform %SomeData
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%_entrypoint_v = OpFunction %void None %22
%23 = OpLabel
%26 = OpVariable %_ptr_Function_v2float Function
OpStore %26 %25
%28 = OpFunctionCall %v4float %main %26
OpStore %sk_FragColor %28
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %29
%30 = OpFunctionParameter %_ptr_Function_v2float
%31 = OpLabel
%35 = OpAccessChain %_ptr_Uniform_uint %3 %int_0
%37 = OpLoad %uint %35
%38 = OpAccessChain %_ptr_Uniform_SomeData %3 %int_1 %37
%40 = OpLoad %SomeData %38
%41 = OpAccessChain %_ptr_Uniform_uint %3 %int_0
%42 = OpLoad %uint %41
%43 = OpAccessChain %_ptr_Uniform_SomeData %12 %int_0 %42
OpStore %43 %40
%44 = OpAccessChain %_ptr_Uniform_uint %3 %int_0
%45 = OpLoad %uint %44
%46 = OpAccessChain %_ptr_Uniform_v4float %3 %int_1 %45 %int_0
%48 = OpLoad %v4float %46
%49 = OpAccessChain %_ptr_Uniform_uint %3 %int_0
%50 = OpLoad %uint %49
%51 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1 %50 %int_1
%53 = OpLoad %v2float %51
%54 = OpCompositeExtract %float %53 0
%55 = OpVectorTimesScalar %v4float %48 %54
OpReturnValue %55
OpFunctionEnd
