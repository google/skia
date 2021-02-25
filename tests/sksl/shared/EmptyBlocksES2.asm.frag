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
OpName %x "x"
OpName %counter_0 "counter"
OpName %y "y"
OpName %z "z"
OpName %counter_1 "counter"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %82 RelaxedPrecision
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
%true = OpConstantTrue %bool
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%float_2 = OpConstant %float 2
%int_3 = OpConstant %int 3
%float_3 = OpConstant %float 3
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
%x = OpVariable %_ptr_Function_int Function
%counter_0 = OpVariable %_ptr_Function_int Function
%y = OpVariable %_ptr_Function_int Function
%z = OpVariable %_ptr_Function_int Function
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
%35 = OpLoad %int %counter
%36 = OpIAdd %int %35 %int_1
OpStore %counter %36
OpBranch %25
%29 = OpLabel
OpStore %counter_0 %int_0
OpBranch %38
%38 = OpLabel
OpLoopMerge %42 %41 None
OpBranch %39
%39 = OpLabel
%43 = OpLoad %int %counter_0
%44 = OpSLessThan %bool %43 %int_10
OpBranchConditional %44 %40 %42
%40 = OpLabel
OpBranch %41
%41 = OpLabel
%47 = OpLoad %int %counter_0
%48 = OpIAdd %int %47 %int_1
OpStore %counter_0 %48
OpBranch %38
%42 = OpLabel
OpStore %counter_1 %int_0
OpBranch %50
%50 = OpLabel
OpLoopMerge %54 %53 None
OpBranch %51
%51 = OpLabel
%55 = OpLoad %int %counter_1
%56 = OpSLessThan %bool %55 %int_10
OpBranchConditional %56 %52 %54
%52 = OpLabel
OpBranch %53
%53 = OpLabel
%58 = OpLoad %int %counter_1
%59 = OpIAdd %int %58 %int_1
OpStore %counter_1 %59
OpBranch %50
%54 = OpLabel
%60 = OpExtInst %float %1 Sqrt %float_1
%62 = OpFOrdEqual %bool %60 %float_1
OpSelectionMerge %65 None
OpBranchConditional %62 %63 %64
%63 = OpLabel
%66 = OpAccessChain %_ptr_Function_float %color %int_1
OpStore %66 %float_1
OpBranch %65
%64 = OpLabel
OpBranch %65
%65 = OpLabel
%68 = OpExtInst %float %1 Sqrt %float_1
%70 = OpFOrdEqual %bool %68 %float_2
OpSelectionMerge %73 None
OpBranchConditional %70 %71 %72
%71 = OpLabel
OpBranch %73
%72 = OpLabel
%74 = OpAccessChain %_ptr_Function_float %color %int_3
OpStore %74 %float_1
OpBranch %73
%73 = OpLabel
%76 = OpExtInst %float %1 Sqrt %float_1
%78 = OpFOrdEqual %bool %76 %float_3
OpSelectionMerge %81 None
OpBranchConditional %78 %79 %80
%79 = OpLabel
OpBranch %81
%80 = OpLabel
OpBranch %81
%81 = OpLabel
%82 = OpLoad %v4float %color
OpReturnValue %82
OpFunctionEnd
