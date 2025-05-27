               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %x "x"
               OpName %y "y"
               OpName %a "a"
               OpName %b "b"
               OpName %c "c"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %c RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %13 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %17 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
    %float_1 = OpConstant %float 1
%_entrypoint_v = OpFunction %void None %9
         %10 = OpLabel
         %14 = OpVariable %_ptr_Function_v2float Function
               OpStore %14 %13
         %16 = OpFunctionCall %v4float %main %14
               OpStore %sk_FragColor %16
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %17
         %18 = OpFunctionParameter %_ptr_Function_v2float
         %19 = OpLabel
          %x = OpVariable %_ptr_Function_float Function
          %y = OpVariable %_ptr_Function_float Function
          %a = OpVariable %_ptr_Function_float Function
          %b = OpVariable %_ptr_Function_float Function
          %c = OpVariable %_ptr_Function_float Function
               OpStore %y %float_1
               OpStore %x %float_1
               OpStore %c %float_0
               OpStore %b %float_0
               OpStore %a %float_0
         %27 = OpFMul %float %float_0 %float_0
         %28 = OpCompositeConstruct %v4float %27 %float_1 %float_0 %float_1
               OpReturnValue %28
               OpFunctionEnd
