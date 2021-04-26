OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %sk_Clockwise
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %_entrypoint_v "_entrypoint_v"
OpName %main "main"
OpName %result "result"
OpName %a "a"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %e "e"
OpName %f "f"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %result RelaxedPrecision
OpDecorate %20 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %67 RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %113 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%false = OpConstantFalse %bool
%float_10 = OpConstant %float 10
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%float_1 = OpConstant %float 1
%_ptr_Function_int = OpTypePointer Function %int
%int_10 = OpConstant %int 10
%int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%_ptr_Function__arr_float_int_2 = OpTypePointer Function %_arr_float_int_2
%int_4 = OpConstant %int 4
%_arr_float_int_4 = OpTypeArray %float %int_4
%_ptr_Function__arr_float_int_4 = OpTypePointer Function %_arr_float_int_4
%float_2 = OpConstant %float 2
%float_3 = OpConstant %float 3
%float_4 = OpConstant %float 4
%float_9 = OpConstant %float 9
%int_3 = OpConstant %int 3
%_entrypoint_v = OpFunction %void None %12
%13 = OpLabel
%14 = OpFunctionCall %v4float %main
OpStore %sk_FragColor %14
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %15
%16 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%a = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_int Function
%d = OpVariable %_ptr_Function__arr_float_int_2 Function
%e = OpVariable %_ptr_Function__arr_float_int_4 Function
%f = OpVariable %_ptr_Function_float Function
OpStore %result %20
OpStore %a %float_0
OpStore %b %float_0
OpBranch %24
%24 = OpLabel
OpLoopMerge %28 %27 None
OpBranch %25
%25 = OpLabel
%30 = OpLoad %float %a
%32 = OpFOrdLessThan %bool %30 %float_10
OpSelectionMerge %34 None
OpBranchConditional %32 %33 %34
%33 = OpLabel
%35 = OpLoad %float %b
%36 = OpFOrdLessThan %bool %35 %float_10
OpBranch %34
%34 = OpLabel
%37 = OpPhi %bool %false %25 %36 %33
OpBranchConditional %37 %26 %28
%26 = OpLabel
%38 = OpAccessChain %_ptr_Function_float %result %int_0
%41 = OpLoad %float %38
%42 = OpLoad %float %a
%43 = OpFAdd %float %41 %42
OpStore %38 %43
%44 = OpAccessChain %_ptr_Function_float %result %int_1
%46 = OpLoad %float %44
%47 = OpLoad %float %b
%48 = OpFAdd %float %46 %47
OpStore %44 %48
OpBranch %27
%27 = OpLabel
%50 = OpLoad %float %a
%51 = OpFAdd %float %50 %float_1
OpStore %a %51
%52 = OpLoad %float %b
%53 = OpFAdd %float %52 %float_1
OpStore %b %53
OpBranch %24
%28 = OpLabel
OpStore %c %int_0
OpBranch %56
%56 = OpLabel
OpLoopMerge %60 %59 None
OpBranch %57
%57 = OpLabel
%61 = OpLoad %int %c
%63 = OpSLessThan %bool %61 %int_10
OpBranchConditional %63 %58 %60
%58 = OpLabel
%64 = OpAccessChain %_ptr_Function_float %result %int_2
%66 = OpLoad %float %64
%67 = OpFAdd %float %66 %float_1
OpStore %64 %67
OpBranch %59
%59 = OpLabel
%68 = OpLoad %int %c
%69 = OpIAdd %int %68 %int_1
OpStore %c %69
OpBranch %56
%60 = OpLabel
%73 = OpCompositeConstruct %_arr_float_int_2 %float_0 %float_10
OpStore %d %73
%81 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %e %81
OpStore %f %float_9
OpBranch %84
%84 = OpLabel
OpLoopMerge %88 %87 None
OpBranch %85
%85 = OpLabel
%89 = OpAccessChain %_ptr_Function_float %d %int_0
%90 = OpLoad %float %89
%91 = OpAccessChain %_ptr_Function_float %d %int_1
%92 = OpLoad %float %91
%93 = OpFOrdLessThan %bool %90 %92
OpBranchConditional %93 %86 %88
%86 = OpLabel
%94 = OpAccessChain %_ptr_Function_float %e %int_0
%95 = OpLoad %float %94
%96 = OpLoad %float %f
%97 = OpFMul %float %95 %96
%98 = OpAccessChain %_ptr_Function_float %result %int_3
OpStore %98 %97
OpBranch %87
%87 = OpLabel
%100 = OpAccessChain %_ptr_Function_float %d %int_0
%101 = OpLoad %float %100
%102 = OpFAdd %float %101 %float_1
OpStore %100 %102
OpBranch %84
%88 = OpLabel
OpBranch %103
%103 = OpLabel
OpLoopMerge %107 %106 None
OpBranch %104
%104 = OpLabel
OpBranch %105
%105 = OpLabel
OpBranch %107
%106 = OpLabel
OpBranch %103
%107 = OpLabel
OpBranch %108
%108 = OpLabel
OpLoopMerge %112 %111 None
OpBranch %109
%109 = OpLabel
OpBranch %110
%110 = OpLabel
OpBranch %112
%111 = OpLabel
OpBranch %108
%112 = OpLabel
%113 = OpLoad %v4float %result
OpReturnValue %113
OpFunctionEnd
