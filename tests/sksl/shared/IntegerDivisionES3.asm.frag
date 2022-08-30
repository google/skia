OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
OpExecutionMode %_entrypoint_v OriginUpperLeft
OpName %sk_Clockwise "sk_Clockwise"
OpName %sk_FragColor "sk_FragColor"
OpName %_UniformBuffer "_UniformBuffer"
OpMemberName %_UniformBuffer 0 "colorGreen"
OpName %_entrypoint_v "_entrypoint_v"
OpName %exact_division_iii "exact_division_iii"
OpName %result "result"
OpName %main "main"
OpName %zero "zero"
OpName %one "one"
OpName %x "x"
OpName %y "y"
OpDecorate %sk_Clockwise BuiltIn FrontFacing
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpMemberDecorate %_UniformBuffer 0 Offset 0
OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
OpDecorate %_UniformBuffer Block
OpDecorate %11 Binding 0
OpDecorate %11 DescriptorSet 0
OpDecorate %53 RelaxedPrecision
OpDecorate %54 RelaxedPrecision
OpDecorate %58 RelaxedPrecision
OpDecorate %59 RelaxedPrecision
OpDecorate %98 RelaxedPrecision
OpDecorate %104 RelaxedPrecision
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
%11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
%void = OpTypeVoid
%16 = OpTypeFunction %void
%float_0 = OpConstant %float 0
%v2float = OpTypeVector %float 2
%20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
%26 = OpTypeFunction %int %_ptr_Function_int %_ptr_Function_int
%int_0 = OpConstant %int 0
%int_1 = OpConstant %int 1
%47 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%int_100 = OpConstant %int 100
%float_1 = OpConstant %float 1
%float_0_00392156886 = OpConstant %float 0.00392156886
%_entrypoint_v = OpFunction %void None %16
%17 = OpLabel
%21 = OpVariable %_ptr_Function_v2float Function
OpStore %21 %20
%23 = OpFunctionCall %v4float %main %21
OpStore %sk_FragColor %23
OpReturn
OpFunctionEnd
%exact_division_iii = OpFunction %int None %26
%27 = OpFunctionParameter %_ptr_Function_int
%28 = OpFunctionParameter %_ptr_Function_int
%29 = OpLabel
%result = OpVariable %_ptr_Function_int Function
OpStore %result %int_0
OpBranch %32
%32 = OpLabel
OpLoopMerge %36 %35 None
OpBranch %33
%33 = OpLabel
%37 = OpLoad %int %27
%38 = OpLoad %int %28
%39 = OpSGreaterThanEqual %bool %37 %38
OpBranchConditional %39 %34 %36
%34 = OpLabel
%41 = OpLoad %int %result
%42 = OpIAdd %int %41 %int_1
OpStore %result %42
%43 = OpLoad %int %27
%44 = OpLoad %int %28
%45 = OpISub %int %43 %44
OpStore %27 %45
OpBranch %35
%35 = OpLabel
OpBranch %32
%36 = OpLabel
%46 = OpLoad %int %result
OpReturnValue %46
OpFunctionEnd
%main = OpFunction %v4float None %47
%48 = OpFunctionParameter %_ptr_Function_v2float
%49 = OpLabel
%zero = OpVariable %_ptr_Function_int Function
%one = OpVariable %_ptr_Function_int Function
%x = OpVariable %_ptr_Function_int Function
%y = OpVariable %_ptr_Function_int Function
%83 = OpVariable %_ptr_Function_int Function
%85 = OpVariable %_ptr_Function_int Function
%51 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%53 = OpLoad %v4float %51
%54 = OpCompositeExtract %float %53 0
%55 = OpConvertFToS %int %54
OpStore %zero %55
%57 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%58 = OpLoad %v4float %57
%59 = OpCompositeExtract %float %58 1
%60 = OpConvertFToS %int %59
OpStore %one %60
OpStore %x %55
OpBranch %62
%62 = OpLabel
OpLoopMerge %66 %65 None
OpBranch %63
%63 = OpLabel
%67 = OpLoad %int %x
%69 = OpSLessThan %bool %67 %int_100
OpBranchConditional %69 %64 %66
%64 = OpLabel
%71 = OpLoad %int %one
OpStore %y %71
OpBranch %72
%72 = OpLabel
OpLoopMerge %76 %75 None
OpBranch %73
%73 = OpLabel
%77 = OpLoad %int %y
%78 = OpSLessThan %bool %77 %int_100
OpBranchConditional %78 %74 %76
%74 = OpLabel
%79 = OpLoad %int %x
%80 = OpLoad %int %y
%81 = OpSDiv %int %79 %80
%82 = OpLoad %int %x
OpStore %83 %82
%84 = OpLoad %int %y
OpStore %85 %84
%86 = OpFunctionCall %int %exact_division_iii %83 %85
%87 = OpINotEqual %bool %81 %86
OpSelectionMerge %89 None
OpBranchConditional %87 %88 %89
%88 = OpLabel
%91 = OpLoad %int %x
%92 = OpConvertSToF %float %91
%94 = OpFMul %float %92 %float_0_00392156886
%95 = OpLoad %int %y
%96 = OpConvertSToF %float %95
%97 = OpFMul %float %96 %float_0_00392156886
%98 = OpCompositeConstruct %v4float %float_1 %94 %97 %float_1
OpReturnValue %98
%89 = OpLabel
OpBranch %75
%75 = OpLabel
%99 = OpLoad %int %y
%100 = OpIAdd %int %99 %int_1
OpStore %y %100
OpBranch %72
%76 = OpLabel
OpBranch %65
%65 = OpLabel
%101 = OpLoad %int %x
%102 = OpIAdd %int %101 %int_1
OpStore %x %102
OpBranch %62
%66 = OpLabel
%103 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
%104 = OpLoad %v4float %103
OpReturnValue %104
OpFunctionEnd
