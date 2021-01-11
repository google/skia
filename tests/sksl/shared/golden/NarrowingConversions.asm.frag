OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %ub "ub"
OpName %main "main"
OpName %b "b"
OpName %us "us"
OpName %s "s"
OpName %ui "ui"
OpName %i "i"
OpName %h "h"
OpName %f "f"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %ub RelaxedPrecision
OpDecorate %21 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %31 RelaxedPrecision
OpDecorate %40 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%uint = OpTypeInt 32 0
%_ptr_Private_uint = OpTypePointer Private %uint
%ub = OpVariable %_ptr_Private_uint Private
%uint_1 = OpConstant %uint 1
%void = OpTypeVoid
%15 = OpTypeFunction %void
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%main = OpFunction %void None %15
%16 = OpLabel
%b = OpVariable %_ptr_Function_int Function
%us = OpVariable %_ptr_Function_uint Function
%s = OpVariable %_ptr_Function_int Function
%ui = OpVariable %_ptr_Function_uint Function
%i = OpVariable %_ptr_Function_int Function
%h = OpVariable %_ptr_Function_float Function
%f = OpVariable %_ptr_Function_float Function
OpStore %ub %uint_1
%21 = OpLoad %uint %ub
%20 = OpBitcast %int %21
OpStore %b %20
%25 = OpLoad %int %b
%24 = OpBitcast %uint %25
OpStore %us %24
%28 = OpLoad %uint %us
%27 = OpBitcast %int %28
OpStore %s %27
%31 = OpLoad %int %s
%30 = OpBitcast %uint %31
OpStore %ui %30
%34 = OpLoad %uint %ui
%33 = OpBitcast %int %34
OpStore %i %33
%38 = OpLoad %int %i
%37 = OpConvertSToF %float %38
OpStore %h %37
%40 = OpLoad %float %h
OpStore %f %40
%41 = OpLoad %float %f
%42 = OpLoad %float %h
%43 = OpFMul %float %41 %42
%45 = OpLoad %int %i
%44 = OpConvertSToF %float %45
%46 = OpFMul %float %43 %44
%48 = OpLoad %uint %ui
%47 = OpConvertUToF %float %48
%49 = OpFMul %float %46 %47
%51 = OpLoad %int %s
%50 = OpConvertSToF %float %51
%52 = OpFMul %float %49 %50
%54 = OpLoad %uint %us
%53 = OpConvertUToF %float %54
%55 = OpFMul %float %52 %53
%57 = OpLoad %int %b
%56 = OpConvertSToF %float %57
%58 = OpFMul %float %55 %56
%60 = OpLoad %uint %ub
%59 = OpConvertUToF %float %60
%61 = OpFMul %float %58 %59
%62 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %62 %61
OpReturn
OpFunctionEnd
