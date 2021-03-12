OpCapability Geometry
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main" %3 %8
OpExecutionMode %main InputPoints
OpExecutionMode %main OutputLineStrip
OpExecutionMode %main OutputVertices 2
OpExecutionMode %main Invocations 1
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %sk_InvocationID "sk_InvocationID"
OpName %main "main"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %_arr_sk_PerVertex_int_1 ArrayStride 32
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
%_ptr_Private_int = OpTypePointer Private %int
%sk_InvocationID = OpVariable %_ptr_Private_int Private
%void = OpTypeVoid
%16 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%int_2 = OpConstant %int 2
%bool = OpTypeBool
%_ptr_Input_v4float = OpTypePointer Input %v4float
%float_0_5 = OpConstant %float 0.5
%float_0 = OpConstant %float 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%float_n0_5 = OpConstant %float -0.5
%false = OpConstantFalse %bool
%main = OpFunction %void None %16
%17 = OpLabel
OpStore %sk_InvocationID %int_0
OpBranch %19
%19 = OpLabel
OpLoopMerge %23 %22 None
OpBranch %20
%20 = OpLabel
%24 = OpLoad %int %sk_InvocationID
%26 = OpSLessThan %bool %24 %int_2
OpBranchConditional %26 %21 %23
%21 = OpLabel
%28 = OpAccessChain %_ptr_Input_v4float %8 %int_0 %int_0
%30 = OpLoad %v4float %28
%33 = OpLoad %int %sk_InvocationID
%34 = OpConvertSToF %float %33
%35 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %34
%36 = OpFAdd %v4float %30 %35
%37 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %37 %36
OpEmitVertex
%40 = OpAccessChain %_ptr_Input_v4float %8 %int_0 %int_0
%41 = OpLoad %v4float %40
%43 = OpLoad %int %sk_InvocationID
%44 = OpConvertSToF %float %43
%45 = OpCompositeConstruct %v4float %float_n0_5 %float_0 %float_0 %44
%46 = OpFAdd %v4float %41 %45
%47 = OpAccessChain %_ptr_Output_v4float %3 %int_0
OpStore %47 %46
OpEmitVertex
OpEndPrimitive
OpBranch %22
%22 = OpLabel
%51 = OpLoad %int %sk_InvocationID
%52 = OpIAdd %int %51 %int_1
OpStore %sk_InvocationID %52
OpBranch %19
%23 = OpLabel
OpReturn
OpFunctionEnd
