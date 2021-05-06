OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %color "color"
OpName %counter "counter"
OpName %counter_0 "counter"
OpName %counter_1 "counter"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %color RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%v2float = OpTypeVector %float 2
%float_0 = OpConstant %float 0
%16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%25 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %12
%13 = OpLabel
%17 = OpVariable %_ptr_Function_v2float Function
OpStore %17 %16
%19 = OpFunctionCall %v4float %main %17
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %20
%21 = OpFunctionParameter %_ptr_Function_v2float
%22 = OpLabel
%color = OpVariable %_ptr_Function_v4float Function
%counter = OpVariable %_ptr_Function_int Function
%counter_0 = OpVariable %_ptr_Function_int Function
%counter_1 = OpVariable %_ptr_Function_int Function
OpStore %color %25
OpStore %counter %int_0
OpBranch %30
%30 = OpLabel
OpLoopMerge %34 %33 None
OpBranch %31
%31 = OpLabel
%35 = OpLoad %int %counter
%37 = OpSLessThan %bool %35 %int_10
OpBranchConditional %37 %32 %34
%32 = OpLabel
OpBranch %33
%33 = OpLabel
%39 = OpLoad %int %counter
%40 = OpIAdd %int %39 %int_1
OpStore %counter %40
OpBranch %30
%34 = OpLabel
OpStore %counter_0 %int_0
OpBranch %42
%42 = OpLabel
OpLoopMerge %46 %45 None
OpBranch %43
%43 = OpLabel
%47 = OpLoad %int %counter_0
%48 = OpSLessThan %bool %47 %int_10
OpBranchConditional %48 %44 %46
%44 = OpLabel
OpBranch %45
%45 = OpLabel
%49 = OpLoad %int %counter_0
%50 = OpIAdd %int %49 %int_1
OpStore %counter_0 %50
OpBranch %42
%46 = OpLabel
OpStore %counter_1 %int_0
OpBranch %52
%52 = OpLabel
OpLoopMerge %56 %55 None
OpBranch %53
%53 = OpLabel
%57 = OpLoad %int %counter_1
%58 = OpSLessThan %bool %57 %int_10
OpBranchConditional %58 %54 %56
%54 = OpLabel
OpBranch %55
%55 = OpLabel
%59 = OpLoad %int %counter_1
%60 = OpIAdd %int %59 %int_1
OpStore %counter_1 %60
OpBranch %52
%56 = OpLabel
%62 = OpAccessChain %_ptr_Function_float %color %int_1
OpStore %62 %float_1
%64 = OpAccessChain %_ptr_Function_float %color %int_3
OpStore %64 %float_1
%66 = OpLoad %v4float %color
OpReturnValue %66
OpFunctionEnd
