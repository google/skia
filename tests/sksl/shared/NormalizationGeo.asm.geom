### Compilation failed:

error: SPIR-V validation error: ID 123456[%123456] has not been defined
  %37 = OpLoad %v4float %123456

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
%37 = OpLoad %v4float %123456
%38 = OpVectorShuffle %v2float %37 %37 0 2
%39 = OpFMul %v2float %35 %38
%40 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%41 = OpLoad %v4float %40
%42 = OpVectorShuffle %v2float %41 %41 3 3
%43 = OpLoad %v4float %123456
%44 = OpVectorShuffle %v2float %43 %43 1 3
%45 = OpFMul %v2float %42 %44
%46 = OpFAdd %v2float %39 %45
%47 = OpCompositeExtract %float %46 0
%48 = OpCompositeExtract %float %46 1
%49 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%50 = OpLoad %v4float %49
%51 = OpCompositeExtract %float %50 3
%52 = OpCompositeConstruct %v4float %47 %48 %float_0 %51
%53 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %53 %52
OpEmitVertex
%55 = OpAccessChain %_ptr_Input_v4float %8 %int_0 %int_0
%56 = OpLoad %v4float %55
%58 = OpLoad %int %sk_InvocationID
%59 = OpConvertSToF %float %58
%60 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %59
%61 = OpFAdd %v4float %56 %60
%62 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %62 %61
%63 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%64 = OpLoad %v4float %63
%65 = OpVectorShuffle %v2float %64 %64 0 1
%66 = OpLoad %v4float %123456
%67 = OpVectorShuffle %v2float %66 %66 0 2
%68 = OpFMul %v2float %65 %67
%69 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%70 = OpLoad %v4float %69
%71 = OpVectorShuffle %v2float %70 %70 3 3
%72 = OpLoad %v4float %123456
%73 = OpVectorShuffle %v2float %72 %72 1 3
%74 = OpFMul %v2float %71 %73
%75 = OpFAdd %v2float %68 %74
%76 = OpCompositeExtract %float %75 0
%77 = OpCompositeExtract %float %75 1
%78 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%79 = OpLoad %v4float %78
%80 = OpCompositeExtract %float %79 3
%81 = OpCompositeConstruct %v4float %76 %77 %float_0 %80
%82 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %82 %81
OpEmitVertex
OpEndPrimitive
OpReturn
OpFunctionEnd

1 error
