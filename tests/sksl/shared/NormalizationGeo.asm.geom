OpCapability Geometry
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main" %3 %8 %sk_InvocationID
OpExecutionMode %main InputPoints
OpExecutionMode %main OutputLineStrip
OpExecutionMode %main OutputVertices 2
OpExecutionMode %main Invocations 2
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %sk_InvocationID "sk_InvocationID"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "sk_RTAdjust"
OpName %main "main"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %_arr_sk_PerVertex_int_1 ArrayStride 32
OpDecorate %sk_InvocationID BuiltIn InvocationId
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpDecorate %_UniformBuffer Block
OpDecorate %15 Binding 0
OpDecorate %15 DescriptorSet 0
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%3 = OpVariable %_ptr_Output_sk_PerVertex Output
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_arr_sk_PerVertex_int_1 = OpTypeArray %sk_PerVertex %int_1
%_ptr_Input__arr_sk_PerVertex_int_1 = OpTypePointer Input %_arr_sk_PerVertex_int_1
%8 = OpVariable %_ptr_Input__arr_sk_PerVertex_int_1 Input
%_ptr_Input_int = OpTypePointer Input %int
%sk_InvocationID = OpVariable %_ptr_Input_int Input
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%15 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%19 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%float_n0_5 = OpConstant %float -0.5
%float_0 = OpConstant %float 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%v2float = OpTypeVector %float 2
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%float_0_5 = OpConstant %float 0.5
%main = OpFunction %void None %19
%20 = OpLabel
%22 = OpAccessChain %_ptr_Input_v4float %8 %int_0 %int_0
%24 = OpLoad %v4float %22
%27 = OpLoad %int %sk_InvocationID
%28 = OpConvertSToF %float %27
%29 = OpCompositeConstruct %v4float %float_n0_5 %float_0 %float_0 %28
%30 = OpFAdd %v4float %24 %29
%31 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %31 %30
%33 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%34 = OpLoad %v4float %33
%35 = OpVectorShuffle %v2float %34 %34 0 1
%37 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
%39 = OpLoad %v4float %37
%40 = OpVectorShuffle %v2float %39 %39 0 2
%41 = OpFMul %v2float %35 %40
%42 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%43 = OpLoad %v4float %42
%44 = OpVectorShuffle %v2float %43 %43 3 3
%45 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
%46 = OpLoad %v4float %45
%47 = OpVectorShuffle %v2float %46 %46 1 3
%48 = OpFMul %v2float %44 %47
%49 = OpFAdd %v2float %41 %48
%50 = OpCompositeExtract %float %49 0
%51 = OpCompositeExtract %float %49 1
%52 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%53 = OpLoad %v4float %52
%54 = OpCompositeExtract %float %53 3
%55 = OpCompositeConstruct %v4float %50 %51 %float_0 %54
%56 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %56 %55
OpEmitVertex
%58 = OpAccessChain %_ptr_Input_v4float %8 %int_0 %int_0
%59 = OpLoad %v4float %58
%61 = OpLoad %int %sk_InvocationID
%62 = OpConvertSToF %float %61
%63 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %62
%64 = OpFAdd %v4float %59 %63
%65 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %65 %64
%66 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%67 = OpLoad %v4float %66
%68 = OpVectorShuffle %v2float %67 %67 0 1
%69 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
%70 = OpLoad %v4float %69
%71 = OpVectorShuffle %v2float %70 %70 0 2
%72 = OpFMul %v2float %68 %71
%73 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%74 = OpLoad %v4float %73
%75 = OpVectorShuffle %v2float %74 %74 3 3
%76 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
%77 = OpLoad %v4float %76
%78 = OpVectorShuffle %v2float %77 %77 1 3
%79 = OpFMul %v2float %75 %78
%80 = OpFAdd %v2float %72 %79
%81 = OpCompositeExtract %float %80 0
%82 = OpCompositeExtract %float %80 1
%83 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%84 = OpLoad %v4float %83
%85 = OpCompositeExtract %float %84 3
%86 = OpCompositeConstruct %v4float %81 %82 %float_0 %85
%87 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %87 %86
OpEmitVertex
OpEndPrimitive
OpReturn
OpFunctionEnd
