OpCapability Shader
%1 = OpExtInstImport "GLSL.std.450"
OpMemoryModel Logical GLSL450
OpEntryPoint Fragment %main "main" %sk_FragColor %sk_Clockwise
OpExecutionMode %main OriginUpperLeft
OpName %sk_FragColor "sk_FragColor"
OpName %sk_Clockwise "sk_Clockwise"
OpName %this_function_is_defined_before_use "this_function_is_defined_before_use"
OpName %main "main"
OpName %this_function_is_defined_after_use "this_function_is_defined_after_use"
OpDecorate %sk_FragColor RelaxedPrecision
OpDecorate %sk_FragColor Location 0
OpDecorate %sk_FragColor Index 0
OpDecorate %sk_Clockwise RelaxedPrecision
OpDecorate %sk_Clockwise BuiltIn FrontFacing
%float = OpTypeFloat 32
%v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%12 = OpTypeFunction %v4float
%float_1 = OpConstant %float 1
%14 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%void = OpTypeVoid
%17 = OpTypeFunction %void
%float_2 = OpConstant %float 2
%22 = OpConstantComposite %v4float %float_2 %float_2 %float_2 %float_2
%this_function_is_defined_before_use = OpFunction %v4float None %12
%13 = OpLabel
OpReturnValue %14
OpFunctionEnd
%main = OpFunction %void None %17
%18 = OpLabel
%19 = OpFunctionCall %v4float %this_function_is_defined_before_use
OpStore %sk_FragColor %19
%20 = OpFunctionCall %v4float %this_function_is_defined_after_use
OpStore %sk_FragColor %20
OpReturn
OpFunctionEnd
%this_function_is_defined_after_use = OpFunction %v4float None %12
%21 = OpLabel
OpReturnValue %22
OpFunctionEnd
