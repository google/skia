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
OpName %sk_RTAdjust "sk_RTAdjust"
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
%_ptr_Private_v4float = OpTypePointer Private %v4float
%sk_RTAdjust = OpVariable %_ptr_Private_v4float Private
%void = OpTypeVoid
%21 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%float_n0_5 = OpConstant %float -0.5
%float_0 = OpConstant %float 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%v2float = OpTypeVector %float 2
%float_0_5 = OpConstant %float 0.5
%main = OpFunction %void None %21
%22 = OpLabel
%24 = OpAccessChain %_ptr_Input_v4float %8 %int_0 %int_0
%26 = OpLoad %v4float %24
%29 = OpLoad %int %sk_InvocationID
%30 = OpConvertSToF %float %29
%31 = OpCompositeConstruct %v4float %float_n0_5 %float_0 %float_0 %30
%32 = OpFAdd %v4float %26 %31
%33 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %33 %32
%35 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%36 = OpLoad %v4float %35
%37 = OpVectorShuffle %v2float %36 %36 0 1
%39 = OpLoad %v4float %sk_RTAdjust
%40 = OpVectorShuffle %v2float %39 %39 0 2
%41 = OpFMul %v2float %37 %40
%42 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%43 = OpLoad %v4float %42
%44 = OpVectorShuffle %v2float %43 %43 3 3
%45 = OpLoad %v4float %sk_RTAdjust
%46 = OpVectorShuffle %v2float %45 %45 1 3
%47 = OpFMul %v2float %44 %46
%48 = OpFAdd %v2float %41 %47
%49 = OpCompositeExtract %float %48 0
%50 = OpCompositeExtract %float %48 1
%51 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%52 = OpLoad %v4float %51
%53 = OpCompositeExtract %float %52 3
%54 = OpCompositeConstruct %v4float %49 %50 %float_0 %53
%55 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %55 %54
OpEmitVertex
%57 = OpAccessChain %_ptr_Input_v4float %8 %int_0 %int_0
%58 = OpLoad %v4float %57
%60 = OpLoad %int %sk_InvocationID
%61 = OpConvertSToF %float %60
%62 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %61
%63 = OpFAdd %v4float %58 %62
%64 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %64 %63
%65 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%66 = OpLoad %v4float %65
%67 = OpVectorShuffle %v2float %66 %66 0 1
%68 = OpLoad %v4float %sk_RTAdjust
%69 = OpVectorShuffle %v2float %68 %68 0 2
%70 = OpFMul %v2float %67 %69
%71 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%72 = OpLoad %v4float %71
%73 = OpVectorShuffle %v2float %72 %72 3 3
%74 = OpLoad %v4float %sk_RTAdjust
%75 = OpVectorShuffle %v2float %74 %74 1 3
%76 = OpFMul %v2float %73 %75
%77 = OpFAdd %v2float %70 %76
%78 = OpCompositeExtract %float %77 0
%79 = OpCompositeExtract %float %77 1
%80 = OpAccessChain %_ptr_Output_v4float %3 %int_0
%81 = OpLoad %v4float %80
%82 = OpCompositeExtract %float %81 3
%83 = OpCompositeConstruct %v4float %78 %79 %float_0 %82
%84 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %84 %83
OpEmitVertex
OpEndPrimitive
OpReturn
OpFunctionEnd
