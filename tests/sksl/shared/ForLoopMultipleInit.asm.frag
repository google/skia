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
OpDecorate %25 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %71 RelaxedPrecision
OpDecorate %72 RelaxedPrecision
OpDecorate %_arr_float_int_2 ArrayStride 16
OpDecorate %_arr_float_int_4 ArrayStride 16
OpDecorate %118 RelaxedPrecision
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
%17 = OpVariable %_ptr_Function_v2float Function
OpStore %17 %16
%19 = OpFunctionCall %v4float %main %17
OpStore %sk_FragColor %19
OpReturn
OpFunctionEnd
%main = OpFunction %v4float None %20
%21 = OpFunctionParameter %_ptr_Function_v2float
%22 = OpLabel
%result = OpVariable %_ptr_Function_v4float Function
%a = OpVariable %_ptr_Function_float Function
%b = OpVariable %_ptr_Function_float Function
%c = OpVariable %_ptr_Function_int Function
%d = OpVariable %_ptr_Function__arr_float_int_2 Function
%e = OpVariable %_ptr_Function__arr_float_int_4 Function
%f = OpVariable %_ptr_Function_float Function
OpStore %result %25
OpStore %a %float_0
OpStore %b %float_0
OpBranch %29
%29 = OpLabel
OpLoopMerge %33 %32 None
OpBranch %30
%30 = OpLabel
%35 = OpLoad %float %a
%37 = OpFOrdLessThan %bool %35 %float_10
OpSelectionMerge %39 None
OpBranchConditional %37 %38 %39
%38 = OpLabel
%40 = OpLoad %float %b
%41 = OpFOrdLessThan %bool %40 %float_10
OpBranch %39
%39 = OpLabel
%42 = OpPhi %bool %false %30 %41 %38
OpBranchConditional %42 %31 %33
%31 = OpLabel
%43 = OpAccessChain %_ptr_Function_float %result %int_0
%46 = OpLoad %float %43
%47 = OpLoad %float %a
%48 = OpFAdd %float %46 %47
OpStore %43 %48
%49 = OpAccessChain %_ptr_Function_float %result %int_1
%51 = OpLoad %float %49
%52 = OpLoad %float %b
%53 = OpFAdd %float %51 %52
OpStore %49 %53
OpBranch %32
%32 = OpLabel
%55 = OpLoad %float %a
%56 = OpFAdd %float %55 %float_1
OpStore %a %56
%57 = OpLoad %float %b
%58 = OpFAdd %float %57 %float_1
OpStore %b %58
OpBranch %29
%33 = OpLabel
OpStore %c %int_0
OpBranch %61
%61 = OpLabel
OpLoopMerge %65 %64 None
OpBranch %62
%62 = OpLabel
%66 = OpLoad %int %c
%68 = OpSLessThan %bool %66 %int_10
OpBranchConditional %68 %63 %65
%63 = OpLabel
%69 = OpAccessChain %_ptr_Function_float %result %int_2
%71 = OpLoad %float %69
%72 = OpFAdd %float %71 %float_1
OpStore %69 %72
OpBranch %64
%64 = OpLabel
%73 = OpLoad %int %c
%74 = OpIAdd %int %73 %int_1
OpStore %c %74
OpBranch %61
%65 = OpLabel
%78 = OpCompositeConstruct %_arr_float_int_2 %float_0 %float_10
OpStore %d %78
%86 = OpCompositeConstruct %_arr_float_int_4 %float_1 %float_2 %float_3 %float_4
OpStore %e %86
OpStore %f %float_9
OpBranch %89
%89 = OpLabel
OpLoopMerge %93 %92 None
OpBranch %90
%90 = OpLabel
%94 = OpAccessChain %_ptr_Function_float %d %int_0
%95 = OpLoad %float %94
%96 = OpAccessChain %_ptr_Function_float %d %int_1
%97 = OpLoad %float %96
%98 = OpFOrdLessThan %bool %95 %97
OpBranchConditional %98 %91 %93
%91 = OpLabel
%99 = OpAccessChain %_ptr_Function_float %e %int_0
%100 = OpLoad %float %99
%101 = OpLoad %float %f
%102 = OpFMul %float %100 %101
%103 = OpAccessChain %_ptr_Function_float %result %int_3
OpStore %103 %102
OpBranch %92
%92 = OpLabel
%105 = OpAccessChain %_ptr_Function_float %d %int_0
%106 = OpLoad %float %105
%107 = OpFAdd %float %106 %float_1
OpStore %105 %107
OpBranch %89
%93 = OpLabel
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
OpBranch %113
%113 = OpLabel
OpLoopMerge %117 %116 None
OpBranch %114
%114 = OpLabel
OpBranch %115
%115 = OpLabel
OpBranch %117
%116 = OpLabel
OpBranch %113
%117 = OpLabel
%118 = OpLoad %v4float %result
OpReturnValue %118
OpFunctionEnd
