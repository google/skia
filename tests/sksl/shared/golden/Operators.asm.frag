OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %main "main"
OpName %x "x"
OpName %y "y"
OpName %z "z"
OpName %b "b"
OpName %c "c"
OpName %d "d"
OpName %e "e"
OpName %f "f"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %35 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
OpDecorate %86 RelaxedPrecision
OpDecorate %89 RelaxedPrecision
OpDecorate %92 RelaxedPrecision
OpDecorate %95 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%void = OpTypeVoid
%7 = OpTypeFunction %void
%float = OpTypeFloat 32
%_ptr_Function_float = OpTypePointer Function %float
%float_1 = OpConstant %float 1
%float_2 = OpConstant %float 2
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%int_3 = OpConstant %int 3
%float_0_5 = OpConstant %float 0.5
%int_8 = OpConstant %int 8
%_ptr_Function_bool = OpTypePointer Function %bool
%true = OpConstantTrue %bool
%false = OpConstantFalse %bool
%float_12 = OpConstant %float 12
%float_10 = OpConstant %float 10
%int_0 = OpConstant %int 0
%int_n1 = OpConstant %int -1
%int_2 = OpConstant %int 2
%int_4 = OpConstant %int 4
%int_5 = OpConstant %int 5
%v2float = OpTypeVector %float 2
%int_6 = OpConstant %int 6
%float_0 = OpConstant %float 0
%float_6 = OpConstant %float 6
%main = OpFunction %void None %7
%8 = OpLabel
%x = OpVariable %_ptr_Function_float Function
%y = OpVariable %_ptr_Function_float Function
%z = OpVariable %_ptr_Function_int Function
%b = OpVariable %_ptr_Function_bool Function
%c = OpVariable %_ptr_Function_bool Function
%d = OpVariable %_ptr_Function_bool Function
%e = OpVariable %_ptr_Function_bool Function
%f = OpVariable %_ptr_Function_bool Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
OpStore %x %float_2
OpStore %y %float_0_5
OpStore %z %int_8
%25 = OpLogicalEqual %bool %false %false
OpSelectionMerge %27 None
OpBranchConditional %25 %27 %26
%26 = OpLabel
%28 = OpExtInst %float %1 Sqrt %float_2
%29 = OpFOrdGreaterThanEqual %bool %float_2 %28
OpBranch %27
%27 = OpLabel
%30 = OpPhi %bool %true %8 %29 %26
OpStore %b %30
%32 = OpExtInst %float %1 Sqrt %float_2
%33 = OpFOrdGreaterThan %bool %32 %float_2
OpStore %c %33
%35 = OpLoad %bool %b
%36 = OpLoad %bool %c
%37 = OpLogicalNotEqual %bool %35 %36
OpStore %d %37
%39 = OpLoad %bool %b
OpSelectionMerge %41 None
OpBranchConditional %39 %40 %41
%40 = OpLabel
%42 = OpLoad %bool %c
OpBranch %41
%41 = OpLabel
%43 = OpPhi %bool %false %27 %42 %40
OpStore %e %43
%45 = OpLoad %bool %b
OpSelectionMerge %47 None
OpBranchConditional %45 %47 %46
%46 = OpLabel
%48 = OpLoad %bool %c
OpBranch %47
%47 = OpLabel
%49 = OpPhi %bool %true %41 %48 %46
OpStore %f %49
%50 = OpLoad %float %x
%52 = OpFAdd %float %50 %float_12
OpStore %x %52
%53 = OpLoad %float %x
%54 = OpFSub %float %53 %float_12
OpStore %x %54
%55 = OpLoad %float %x
%56 = OpLoad %float %y
%58 = OpFDiv %float %56 %float_10
OpStore %y %58
%59 = OpFMul %float %55 %58
OpStore %x %59
%60 = OpLoad %int %z
%62 = OpBitwiseOr %int %60 %int_0
OpStore %z %62
%63 = OpLoad %int %z
%65 = OpBitwiseAnd %int %63 %int_n1
OpStore %z %65
%66 = OpLoad %int %z
%67 = OpBitwiseXor %int %66 %int_0
OpStore %z %67
%68 = OpLoad %int %z
%70 = OpShiftRightArithmetic %int %68 %int_2
OpStore %z %70
%71 = OpLoad %int %z
%73 = OpShiftLeftLogical %int %71 %int_4
OpStore %z %73
%74 = OpLoad %int %z
%76 = OpSMod %int %74 %int_5
OpStore %z %76
%78 = OpExtInst %float %1 Sqrt %float_1
%79 = OpCompositeConstruct %v2float %78 %78
%77 = OpConvertSToF %float %int_6
OpStore %x %77
%83 = OpLoad %bool %b
%82 = OpSelect %float %83 %float_1 %float_0
%86 = OpLoad %bool %c
%85 = OpSelect %float %86 %float_1 %float_0
%87 = OpFMul %float %82 %85
%89 = OpLoad %bool %d
%88 = OpSelect %float %89 %float_1 %float_0
%90 = OpFMul %float %87 %88
%92 = OpLoad %bool %e
%91 = OpSelect %float %92 %float_1 %float_0
%93 = OpFMul %float %90 %91
%95 = OpLoad %bool %f
%94 = OpSelect %float %95 %float_1 %float_0
%96 = OpFMul %float %93 %94
OpStore %y %float_6
%98 = OpExtInst %float %1 Sqrt %float_1
%99 = OpCompositeConstruct %v2float %98 %98
OpStore %z %int_6
OpReturn
OpFunctionEnd
