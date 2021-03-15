OpCapability Geometry
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Geometry %main "main" %5 %10
OpExecutionMode %main InputPoints
OpExecutionMode %main OutputLineStrip
OpExecutionMode %main OutputVertices 2
OpExecutionMode %main Invocations 1
OpName %sk_PerVertex "sk_PerVertex"
OpMemberName %sk_PerVertex 0 "sk_Position"
OpMemberName %sk_PerVertex 1 "sk_PointSize"
OpName %sk_InvocationID "sk_InvocationID"
OpName %test "test"
OpName %_invoke "_invoke"
OpName %main "main"
OpMemberDecorate %sk_PerVertex 0 BuiltIn Position
OpMemberDecorate %sk_PerVertex 1 BuiltIn PointSize
OpDecorate %_arr_sk_PerVertex_int_1 ArrayStride 32
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%sk_PerVertex = OpTypeStruct %v4float %float
%_ptr_Output_sk_PerVertex = OpTypePointer Output %sk_PerVertex
%5 = OpVariable %_ptr_Output_sk_PerVertex Output
%int = OpTypeInt 32 1
%int_1 = OpConstant %int 1
%_arr_sk_PerVertex_int_1 = OpTypeArray %sk_PerVertex %int_1
%_ptr_Input__arr_sk_PerVertex_int_1 = OpTypePointer Input %_arr_sk_PerVertex_int_1
%10 = OpVariable %_ptr_Input__arr_sk_PerVertex_int_1 Input
%_ptr_Private_int = OpTypePointer Private %int
%sk_InvocationID = OpVariable %_ptr_Private_int Private
%void = OpTypeVoid
%18 = OpTypeFunction %void
%int_0 = OpConstant %int 0
%_ptr_Input_v4float = OpTypePointer Input %v4float
%float_0_5 = OpConstant %float 0.5
%float_0 = OpConstant %float 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%float_n0_5 = OpConstant %float -0.5
%int_2 = OpConstant %int 2
%bool = OpTypeBool
%test = OpFunction %void None %18
%19 = OpLabel
%21 = OpAccessChain %_ptr_Input_v4float %10 %int_0 %int_0
%23 = OpLoad %v4float %21
%26 = OpLoad %int %sk_InvocationID
%27 = OpConvertSToF %float %26
%28 = OpCompositeConstruct %v4float %float_0_5 %float_0 %float_0 %27
%29 = OpFAdd %v4float %23 %28
%30 = OpAccessChain %_ptr_Output_v4float %5 %int_0
OpStore %30 %29
OpEmitVertex
OpReturn
OpFunctionEnd
%_invoke = OpFunction %void None %18
%33 = OpLabel
%34 = OpFunctionCall %void %test
%35 = OpAccessChain %_ptr_Input_v4float %10 %int_0 %int_0
%36 = OpLoad %v4float %35
%38 = OpLoad %int %sk_InvocationID
%39 = OpConvertSToF %float %38
%40 = OpCompositeConstruct %v4float %float_n0_5 %float_0 %float_0 %39
%41 = OpFAdd %v4float %36 %40
%42 = OpAccessChain %_ptr_Output_v4float %5 %int_0
OpStore %42 %41
OpEmitVertex
OpReturn
OpFunctionEnd
%main = OpFunction %void None %18
%44 = OpLabel
OpStore %sk_InvocationID %int_0
OpBranch %45
%45 = OpLabel
OpLoopMerge %49 %48 None
OpBranch %46
%46 = OpLabel
%50 = OpLoad %int %sk_InvocationID
%52 = OpSLessThan %bool %50 %int_2
OpBranchConditional %52 %47 %49
%47 = OpLabel
%54 = OpFunctionCall %void %_invoke
OpEndPrimitive
OpBranch %48
%48 = OpLabel
%56 = OpLoad %int %sk_InvocationID
%57 = OpIAdd %int %56 %int_1
OpStore %sk_InvocationID %57
OpBranch %45
%49 = OpLabel
OpReturn
OpFunctionEnd
