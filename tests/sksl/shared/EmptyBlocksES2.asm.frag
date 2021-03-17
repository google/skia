OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint "_entrypoint"
OpName %main "main"
OpName %color "color"
OpName %counter "counter"
OpName %counter_0 "counter"
OpName %counter_1 "counter"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %71 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%12 = OpTypeFunction %void
%15 = OpTypeFunction %v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%float_0 = OpConstant %float 0
%20 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_0 = OpConstant %int 0
%int_10 = OpConstant %int 10
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%int_3 = OpConstant %int 3
%_entrypoint = OpFunction %void None %12
%13 = OpLabel
%14 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %14
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %15
%16 = OpLabel
%color = OpVariable %_ptr_Function_v4float Function
%counter = OpVariable %_ptr_Function_int Function
%counter_0 = OpVariable %_ptr_Function_int Function
%counter_1 = OpVariable %_ptr_Function_int Function
OpStore %color %20
OpStore %counter %int_0
OpBranch %25
%25 = OpLabel
OpLoopMerge %29 %28 None
OpBranch %26
%26 = OpLabel
%30 = OpLoad %int %counter
%32 = OpSLessThan %bool %30 %int_10
OpBranchConditional %32 %27 %29
%27 = OpLabel
OpBranch %28
%28 = OpLabel
%34 = OpLoad %int %counter
%35 = OpIAdd %int %34 %int_1
OpStore %counter %35
OpBranch %25
%29 = OpLabel
OpStore %counter_0 %int_0
OpBranch %37
%37 = OpLabel
OpLoopMerge %41 %40 None
OpBranch %38
%38 = OpLabel
%42 = OpLoad %int %counter_0
%43 = OpSLessThan %bool %42 %int_10
OpBranchConditional %43 %39 %41
%39 = OpLabel
OpBranch %40
%40 = OpLabel
%44 = OpLoad %int %counter_0
%45 = OpIAdd %int %44 %int_1
OpStore %counter_0 %45
OpBranch %37
%41 = OpLabel
OpStore %counter_1 %int_0
OpBranch %47
%47 = OpLabel
OpLoopMerge %51 %50 None
OpBranch %48
%48 = OpLabel
%52 = OpLoad %int %counter_1
%53 = OpSLessThan %bool %52 %int_10
OpBranchConditional %53 %49 %51
%49 = OpLabel
OpBranch %50
%50 = OpLabel
%54 = OpLoad %int %counter_1
%55 = OpIAdd %int %54 %int_1
OpStore %counter_1 %55
OpBranch %47
%51 = OpLabel
%56 = OpExtInst %float %1 Sqrt %float_1
%58 = OpFOrdEqual %bool %56 %float_1
OpSelectionMerge %60 None
OpBranchConditional %58 %59 %60
%59 = OpLabel
%61 = OpAccessChain %_ptr_Function_float %color %int_1
OpStore %61 %float_1
OpBranch %60
%60 = OpLabel
%63 = OpExtInst %float %1 Sqrt %float_1
%65 = OpFOrdEqual %bool %63 %float_2
OpSelectionMerge %68 None
OpBranchConditional %65 %66 %67
%66 = OpLabel
OpBranch %68
%67 = OpLabel
%69 = OpAccessChain %_ptr_Function_float %color %int_3
OpStore %69 %float_1
OpBranch %68
%68 = OpLabel
%71 = OpLoad %v4float %color
OpReturnValue %71
OpFunctionEnd
