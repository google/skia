               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %setGreen_vh "setGreen_vh"
               OpName %setAlpha_vh "setAlpha_vh"
               OpName %main "main"
               OpName %green "green"
               OpName %alpha "alpha"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %green RelaxedPrecision
               OpDecorate %alpha RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_float = OpTypePointer Function %float
         %23 = OpTypeFunction %void %_ptr_Function_float
    %float_1 = OpConstant %float 1
         %29 = OpTypeFunction %v4float %_ptr_Function_v2float
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %19 = OpVariable %_ptr_Function_v2float Function
               OpStore %19 %18
         %21 = OpFunctionCall %v4float %main %19
               OpStore %sk_FragColor %21
               OpReturn
               OpFunctionEnd
%setGreen_vh = OpFunction %void None %23
         %24 = OpFunctionParameter %_ptr_Function_float
         %25 = OpLabel
               OpStore %24 %float_1
               OpReturn
               OpFunctionEnd
%setAlpha_vh = OpFunction %void None %23
         %27 = OpFunctionParameter %_ptr_Function_float
         %28 = OpLabel
               OpStore %27 %float_1
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %29
         %30 = OpFunctionParameter %_ptr_Function_v2float
         %31 = OpLabel
      %green = OpVariable %_ptr_Function_float Function
      %alpha = OpVariable %_ptr_Function_float Function
         %34 = OpVariable %_ptr_Function_float Function
         %37 = OpVariable %_ptr_Function_float Function
         %35 = OpFunctionCall %void %setGreen_vh %34
         %36 = OpLoad %float %34
               OpStore %green %36
         %38 = OpFunctionCall %void %setAlpha_vh %37
         %39 = OpLoad %float %37
               OpStore %alpha %39
         %40 = OpCompositeConstruct %v4float %float_0 %36 %float_0 %39
               OpReturnValue %40
               OpFunctionEnd
