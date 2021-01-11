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
OpDecorate %23 RelaxedPrecision
OpDecorate %30 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
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
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Function_float = OpTypePointer Function %float
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
%23 = OpLoad %int %b
%22 = OpConvertSToF %float %23
%24 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %24 %22
%30 = OpLoad %int %b
%29 = OpBitcast %uint %30
OpStore %us %29
%32 = OpLoad %uint %us
%31 = OpConvertUToF %float %32
%33 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %33 %31
%36 = OpLoad %uint %us
%35 = OpBitcast %int %36
OpStore %s %35
%38 = OpLoad %int %s
%37 = OpConvertSToF %float %38
%39 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %39 %37
%42 = OpLoad %int %s
%41 = OpBitcast %uint %42
OpStore %ui %41
%44 = OpLoad %uint %ui
%43 = OpConvertUToF %float %44
%45 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %45 %43
%48 = OpLoad %uint %ui
%47 = OpBitcast %int %48
OpStore %i %47
%50 = OpLoad %int %i
%49 = OpConvertSToF %float %50
%51 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %51 %49
%55 = OpLoad %int %i
%54 = OpConvertSToF %float %55
OpStore %h %54
%56 = OpLoad %float %h
%57 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %57 %56
%59 = OpLoad %float %h
OpStore %f %59
%60 = OpLoad %float %f
%61 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %61 %60
OpReturn
OpFunctionEnd
