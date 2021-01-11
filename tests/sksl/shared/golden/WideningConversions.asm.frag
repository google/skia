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
OpDecorate %21 RelaxedPrecision
OpDecorate %29 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %69 RelaxedPrecision
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
%float_0_5 = OpConstant %float 0.5
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
%21 = OpFMul %float %19 %float_0_5
%22 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %22 %21
%29 = OpLoad %float %h
%28 = OpConvertFToS %int %29
OpStore %i %28
%31 = OpLoad %int %i
%30 = OpConvertSToF %float %31
%32 = OpFMul %float %30 %float_0_5
%33 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %33 %32
%38 = OpLoad %int %i
%37 = OpBitcast %uint %38
OpStore %ui %37
%40 = OpLoad %uint %ui
%39 = OpConvertUToF %float %40
%41 = OpFMul %float %39 %float_0_5
%42 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %42 %41
%45 = OpLoad %uint %ui
%44 = OpBitcast %int %45
OpStore %s %44
%47 = OpLoad %int %s
%46 = OpConvertSToF %float %47
%48 = OpFMul %float %46 %float_0_5
%49 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %49 %48
%52 = OpLoad %int %s
%51 = OpBitcast %uint %52
OpStore %us %51
%54 = OpLoad %uint %us
%53 = OpConvertUToF %float %54
%55 = OpFMul %float %53 %float_0_5
%56 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %56 %55
%59 = OpLoad %uint %us
%58 = OpBitcast %int %59
OpStore %b %58
%61 = OpLoad %int %b
%60 = OpConvertSToF %float %61
%62 = OpFMul %float %60 %float_0_5
%63 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %63 %62
%66 = OpLoad %int %b
%65 = OpBitcast %uint %66
OpStore %ub %65
%68 = OpLoad %uint %ub
%67 = OpConvertUToF %float %68
%69 = OpFMul %float %67 %float_0_5
%70 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %70 %69
OpReturn
OpFunctionEnd
