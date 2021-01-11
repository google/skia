OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %f "f"
OpName %main "main"
OpName %h "h"
OpName %i "i"
OpName %ui "ui"
OpName %s "s"
OpName %us "us"
OpName %b "b"
OpName %ub "ub"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %19 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %43 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Private_float = OpTypePointer Private %float
%f = OpVariable %_ptr_Private_float Private
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%_ptr_Output_float = OpTypePointer Output %float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%main = OpFunction %void None %14
%15 = OpLabel
%h = OpVariable %_ptr_Function_float Function
%i = OpVariable %_ptr_Function_int Function
%ui = OpVariable %_ptr_Function_uint Function
%s = OpVariable %_ptr_Function_int Function
%us = OpVariable %_ptr_Function_uint Function
%b = OpVariable %_ptr_Function_int Function
%ub = OpVariable %_ptr_Function_uint Function
OpStore %f %float_1
%18 = OpLoad %float %f
OpStore %h %18
%19 = OpLoad %float %h
%20 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %20 %19
%27 = OpLoad %float %h
%26 = OpConvertFToS %int %27
OpStore %i %26
%29 = OpLoad %int %i
%28 = OpConvertSToF %float %29
%30 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %30 %28
%35 = OpLoad %int %i
%34 = OpBitcast %uint %35
OpStore %ui %34
%37 = OpLoad %uint %ui
%36 = OpConvertUToF %float %37
%38 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %38 %36
%41 = OpLoad %uint %ui
%40 = OpBitcast %int %41
OpStore %s %40
%43 = OpLoad %int %s
%42 = OpConvertSToF %float %43
%44 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %44 %42
%47 = OpLoad %int %s
%46 = OpBitcast %uint %47
OpStore %us %46
%49 = OpLoad %uint %us
%48 = OpConvertUToF %float %49
%50 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %50 %48
%53 = OpLoad %uint %us
%52 = OpBitcast %int %53
OpStore %b %52
%55 = OpLoad %int %b
%54 = OpConvertSToF %float %55
%56 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %56 %54
%59 = OpLoad %int %b
%58 = OpBitcast %uint %59
OpStore %ub %58
%61 = OpLoad %uint %ub
%60 = OpConvertUToF %float %61
%62 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %62 %60
OpReturn
OpFunctionEnd
