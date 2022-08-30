OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
OpExecutionMode %main OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "src"
OpMemberName %_UniformBuffer 1 "dst"
OpName %blend_src_over_h4h4h4 "blend_src_over_h4h4h4"
OpName %blend_darken_h4h4h4h "blend_darken_h4h4h4h"
OpName %a "a"
OpName %b "b"
OpName %blend_darken_h4h4h4 "blend_darken_h4h4h4"
OpName %main "main"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpMemberDecorate %_UniformBuffer 1 Offset 16
OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %13 Binding 0
OpDecorate %13 DescriptorSet 0
OpDecorate %21 RelaxedPrecision
OpDecorate %23 RelaxedPrecision
OpDecorate %24 RelaxedPrecision
OpDecorate %25 RelaxedPrecision
OpDecorate %26 RelaxedPrecision
OpDecorate %27 RelaxedPrecision
OpDecorate %28 RelaxedPrecision
OpDecorate %a RelaxedPrecision
OpDecorate %36 RelaxedPrecision
OpDecorate %38 RelaxedPrecision
OpDecorate %b RelaxedPrecision
OpDecorate %44 RelaxedPrecision
OpDecorate %45 RelaxedPrecision
OpDecorate %46 RelaxedPrecision
OpDecorate %47 RelaxedPrecision
OpDecorate %48 RelaxedPrecision
OpDecorate %49 RelaxedPrecision
OpDecorate %50 RelaxedPrecision
OpDecorate %51 RelaxedPrecision
OpDecorate %52 RelaxedPrecision
OpDecorate %53 RelaxedPrecision
OpDecorate %55 RelaxedPrecision
OpDecorate %56 RelaxedPrecision
OpDecorate %57 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %60 RelaxedPrecision
OpDecorate %61 RelaxedPrecision
OpDecorate %62 RelaxedPrecision
OpDecorate %66 RelaxedPrecision
OpDecorate %68 RelaxedPrecision
OpDecorate %79 RelaxedPrecision
OpDecorate %83 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%13 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%_ptr_Function_v4float = OpTypePointer Function %v4float
%17 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
%float_1 = OpConstant %float 1
%_ptr_Function_float = OpTypePointer Function %float
%30 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float %_ptr_Function_float
%v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
%void = OpTypeVoid
%73 = OpTypeFunction %void
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int = OpTypeInt 32 1
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%blend_src_over_h4h4h4 = OpFunction %v4float None %17
%18 = OpFunctionParameter %_ptr_Function_v4float
%19 = OpFunctionParameter %_ptr_Function_v4float
%20 = OpLabel
%21 = OpLoad %v4float %18
%23 = OpLoad %v4float %18
%24 = OpCompositeExtract %float %23 3
%25 = OpFSub %float %float_1 %24
%26 = OpLoad %v4float %19
%27 = OpVectorTimesScalar %v4float %26 %25
%28 = OpFAdd %v4float %21 %27
OpReturnValue %28
OpFunctionEnd
%blend_darken_h4h4h4h = OpFunction %v4float None %30
%31 = OpFunctionParameter %_ptr_Function_v4float
%32 = OpFunctionParameter %_ptr_Function_v4float
%33 = OpFunctionParameter %_ptr_Function_float
%34 = OpLabel
%a = OpVariable %_ptr_Function_v4float Function
%37 = OpVariable %_ptr_Function_v4float Function
%39 = OpVariable %_ptr_Function_v4float Function
%b = OpVariable %_ptr_Function_v3float Function
%36 = OpLoad %v4float %31
OpStore %37 %36
%38 = OpLoad %v4float %32
OpStore %39 %38
%40 = OpFunctionCall %v4float %blend_src_over_h4h4h4 %37 %39
OpStore %a %40
%44 = OpLoad %v4float %32
%45 = OpCompositeExtract %float %44 3
%46 = OpFSub %float %float_1 %45
%47 = OpLoad %v4float %31
%48 = OpVectorShuffle %v3float %47 %47 0 1 2
%49 = OpVectorTimesScalar %v3float %48 %46
%50 = OpLoad %v4float %32
%51 = OpVectorShuffle %v3float %50 %50 0 1 2
%52 = OpFAdd %v3float %49 %51
OpStore %b %52
%53 = OpLoad %float %33
%55 = OpVectorShuffle %v3float %40 %40 0 1 2
%56 = OpLoad %float %33
%57 = OpVectorTimesScalar %v3float %55 %56
%58 = OpLoad %float %33
%59 = OpVectorTimesScalar %v3float %52 %58
%54 = OpExtInst %v3float %1 FMin %57 %59
%60 = OpVectorTimesScalar %v3float %54 %53
%61 = OpLoad %v4float %a
%62 = OpVectorShuffle %v4float %61 %60 4 5 6 3
OpStore %a %62
OpReturnValue %62
OpFunctionEnd
%blend_darken_h4h4h4 = OpFunction %v4float None %17
%63 = OpFunctionParameter %_ptr_Function_v4float
%64 = OpFunctionParameter %_ptr_Function_v4float
%65 = OpLabel
%67 = OpVariable %_ptr_Function_v4float Function
%69 = OpVariable %_ptr_Function_v4float Function
%70 = OpVariable %_ptr_Function_float Function
%66 = OpLoad %v4float %63
OpStore %67 %66
%68 = OpLoad %v4float %64
OpStore %69 %68
OpStore %70 %float_1
%71 = OpFunctionCall %v4float %blend_darken_h4h4h4h %67 %69 %70
OpReturnValue %71
OpFunctionEnd
%main = OpFunction %void None %73
%74 = OpLabel
%80 = OpVariable %_ptr_Function_v4float Function
%84 = OpVariable %_ptr_Function_v4float Function
%75 = OpAccessChain %_ptr_Uniform_v4float %13 %int_0
%79 = OpLoad %v4float %75
OpStore %80 %79
%81 = OpAccessChain %_ptr_Uniform_v4float %13 %int_1
%83 = OpLoad %v4float %81
OpStore %84 %83
%85 = OpFunctionCall %v4float %blend_darken_h4h4h4 %80 %84
OpStore %sk_FragColor %85
OpReturn
OpFunctionEnd
