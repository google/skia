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
OpDecorate %23 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %37 RelaxedPrecision
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
%_ptr_Private_float = OpTypePointer Private %float
%f = OpVariable %_ptr_Private_float Private
%float_1 = OpConstant %float 1
%void = OpTypeVoid
%14 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%uint = OpTypeInt 32 0
%_ptr_Function_uint = OpTypePointer Function %uint
%_ptr_Output_float = OpTypePointer Output %float
%int_0 = OpConstant %int 0
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
%23 = OpLoad %float %h
%22 = OpConvertFToS %int %23
OpStore %i %22
%28 = OpLoad %int %i
%27 = OpBitcast %uint %28
OpStore %ui %27
%31 = OpLoad %uint %ui
%30 = OpBitcast %int %31
OpStore %s %30
%34 = OpLoad %int %s
%33 = OpBitcast %uint %34
OpStore %us %33
%37 = OpLoad %uint %us
%36 = OpBitcast %int %37
OpStore %b %36
%40 = OpLoad %int %b
%39 = OpBitcast %uint %40
OpStore %ub %39
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
