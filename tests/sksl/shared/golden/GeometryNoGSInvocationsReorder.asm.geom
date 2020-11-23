OpCapability Geometry
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main" %4 %9
OpExecutionMode %main InputPoints
OpExecutionMode %main OutputLineStrip
OpExecutionMode %main OutputVertices 2
OpExecutionMode %main Invocations 1
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %sk_InvocationID "sk_InvocationID"
OpName %_invoke "_invoke"
OpName %main "main"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %_arr_sk_PerVertex_int_1 ArrayStride 32
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%4 = OpVariable %_ptr_Output_sk_PerVertex Output
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_arr_sk_PerVertex_int_1 = OpTypeArray %sk_PerVertex %int_1
%_ptr_Input__arr_sk_PerVertex_int_1 = OpTypePointer Input %_arr_sk_PerVertex_int_1
%9 = OpVariable %_ptr_Input__arr_sk_PerVertex_int_1 Input
%_ptr_Private_int = OpTypePointer Private %int
%sk_InvocationID = OpVariable %_ptr_Private_int Private
%void = OpTypeVoid
%17 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%float_0_5 = OpConstant %float 0.5
%float_0 = OpConstant %float 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%float_n0_5 = OpConstant %float -0.5
%int_2 = OpConstant %int 2
%bool = OpTypeBool
%_invoke = OpFunction %void None %17
%18 = OpLabel
%20 = OpAccessChain %_ptr_Input_v4float %9 %int_0 %int_0
%22 = OpLoad %v4float %20
%26 = OpLoad %int %sk_InvocationID
%25 = OpConvertSToF %float %26
%27 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %25
%28 = OpFAdd %v4float %22 %27
%29 = OpAccessChain %_ptr_Output_v4float %4 %int_0
OpStore %29 %28
OpEmitVertex
%32 = OpAccessChain %_ptr_Input_v4float %9 %int_0 %int_0
%33 = OpLoad %v4float %32
%36 = OpLoad %int %sk_InvocationID
%35 = OpConvertSToF %float %36
%37 = OpCompositeConstruct %v4float %float_n0_5 %float_0 %float_0 %35
%38 = OpFAdd %v4float %33 %37
%39 = OpAccessChain %_ptr_Output_v4float %4 %int_0
OpStore %39 %38
OpEmitVertex
OpReturn
OpFunctionEnd
%main = OpFunction %void None %17
%41 = OpLabel
OpStore %sk_InvocationID %int_0
OpBranch %42
%42 = OpLabel
OpLoopMerge %46 %45 None
OpBranch %43
%43 = OpLabel
%47 = OpLoad %int %sk_InvocationID
%49 = OpSLessThan %bool %47 %int_2
OpBranchConditional %49 %44 %46
%44 = OpLabel
%51 = OpFunctionCall %void %_invoke
OpEndPrimitive
OpBranch %45
%45 = OpLabel
%53 = OpLoad %int %sk_InvocationID
%54 = OpIAdd %int %53 %int_1
OpStore %sk_InvocationID %54
OpBranch %42
%46 = OpLabel
OpReturn
OpFunctionEnd
