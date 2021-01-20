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
OpName %c "c"
OpName %d "d"
OpName %e "e"
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %27 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
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
%c = OpVariable %_ptr_Function_bool Function
%d = OpVariable %_ptr_Function_bool Function
%e = OpVariable %_ptr_Function_bool Function
OpStore %x %float_1
OpStore %y %float_2
OpStore %z %int_3
OpStore %x %float_2
OpStore %y %float_0_5
OpStore %z %int_8
%23 = OpExtInst %float %1 Sqrt %float_2
%24 = OpFOrdGreaterThan %bool %23 %float_2
OpStore %c %24
%27 = OpLoad %bool %c
%28 = OpLogicalNotEqual %bool %true %27
OpStore %d %28
%30 = OpLoad %bool %c
OpStore %e %30
%31 = OpLoad %float %x
%33 = OpFAdd %float %31 %float_12
OpStore %x %33
%34 = OpLoad %float %x
%35 = OpFSub %float %34 %float_12
OpStore %x %35
%36 = OpLoad %float %x
%37 = OpLoad %float %y
%39 = OpFDiv %float %37 %float_10
OpStore %y %39
%40 = OpFMul %float %36 %39
OpStore %x %40
%41 = OpLoad %int %z
%43 = OpBitwiseOr %int %41 %int_0
OpStore %z %43
%44 = OpLoad %int %z
%46 = OpBitwiseAnd %int %44 %int_n1
OpStore %z %46
%47 = OpLoad %int %z
%48 = OpBitwiseXor %int %47 %int_0
OpStore %z %48
%49 = OpLoad %int %z
%51 = OpShiftRightArithmetic %int %49 %int_2
OpStore %z %51
%52 = OpLoad %int %z
%54 = OpShiftLeftLogical %int %52 %int_4
OpStore %z %54
%55 = OpLoad %int %z
%57 = OpSMod %int %55 %int_5
OpStore %z %57
%58 = OpExtInst %float %1 Sqrt %float_1
%59 = OpCompositeConstruct %v2float %58 %58
%62 = OpConvertSToF %float %int_6
OpStore %x %62
%63 = OpLoad %bool %c
%64 = OpSelect %float %63 %float_1 %float_0
%66 = OpLoad %bool %d
%67 = OpSelect %float %66 %float_1 %float_0
%68 = OpFMul %float %64 %67
%69 = OpLoad %bool %e
%70 = OpSelect %float %69 %float_1 %float_0
%71 = OpFMul %float %68 %70
OpStore %y %float_6
%73 = OpExtInst %float %1 Sqrt %float_1
%74 = OpCompositeConstruct %v2float %73 %73
OpStore %z %int_6
OpReturn
OpFunctionEnd
