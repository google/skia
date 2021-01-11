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
OpDecorate %25 RelaxedPrecision
OpDecorate %32 RelaxedPrecision
OpDecorate %34 RelaxedPrecision
OpDecorate %35 RelaxedPrecision
OpDecorate %39 RelaxedPrecision
OpDecorate %41 RelaxedPrecision
OpDecorate %42 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %63 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
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
%float_0_5 = OpConstant %float 0.5
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
%25 = OpFMul %float %22 %float_0_5
%26 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %26 %25
%32 = OpLoad %int %b
%31 = OpBitcast %uint %32
OpStore %us %31
%34 = OpLoad %uint %us
%33 = OpConvertUToF %float %34
%35 = OpFMul %float %33 %float_0_5
%36 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %36 %35
%39 = OpLoad %uint %us
%38 = OpBitcast %int %39
OpStore %s %38
%41 = OpLoad %int %s
%40 = OpConvertSToF %float %41
%42 = OpFMul %float %40 %float_0_5
%43 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %43 %42
%46 = OpLoad %int %s
%45 = OpBitcast %uint %46
OpStore %ui %45
%48 = OpLoad %uint %ui
%47 = OpConvertUToF %float %48
%49 = OpFMul %float %47 %float_0_5
%50 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %50 %49
%53 = OpLoad %uint %ui
%52 = OpBitcast %int %53
OpStore %i %52
%55 = OpLoad %int %i
%54 = OpConvertSToF %float %55
%56 = OpFMul %float %54 %float_0_5
%57 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %57 %56
%61 = OpLoad %int %i
%60 = OpConvertSToF %float %61
OpStore %h %60
%62 = OpLoad %float %h
%63 = OpFMul %float %62 %float_0_5
%64 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %64 %63
%66 = OpLoad %float %h
OpStore %f %66
%67 = OpLoad %float %f
%68 = OpFMul %float %67 %float_0_5
%69 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
OpStore %69 %68
OpReturn
OpFunctionEnd
