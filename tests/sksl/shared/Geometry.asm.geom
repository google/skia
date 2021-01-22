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
OpName %main "main"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %_arr_sk_PerVertex_int_1 ArrayStride 32
OpDecorate %sk_InvocationID BuiltIn InvocationId
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
%void = OpTypeVoid
%16 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%float_n0_5 = OpConstant %float -0.5
%float_0 = OpConstant %float 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%float_0_5 = OpConstant %float 0.5
%main = OpFunction %void None %16
%17 = OpLabel
%19 = OpAccessChain %_ptr_Input_v4float %8 %int_0 %int_0
%21 = OpLoad %v4float %19
%24 = OpLoad %int %sk_InvocationID
%25 = OpConvertSToF %float %24
%26 = OpCompositeConstruct %v4float %float_n0_5 %float_0 %float_0 %25
%27 = OpFAdd %v4float %21 %26
%28 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %28 %27
OpEmitVertex
%31 = OpAccessChain %_ptr_Input_v4float %8 %int_0 %int_0
%32 = OpLoad %v4float %31
%34 = OpLoad %int %sk_InvocationID
%35 = OpConvertSToF %float %34
%36 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %35
%37 = OpFAdd %v4float %32 %36
%38 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %38 %37
OpEmitVertex
OpEndPrimitive
OpReturn
OpFunctionEnd
