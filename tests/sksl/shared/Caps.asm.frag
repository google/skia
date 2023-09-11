               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %main "main"
               OpName %x "x"
               OpName %y "y"
               OpName %z "z"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %17 RelaxedPrecision
               OpDecorate %18 RelaxedPrecision
               OpDecorate %19 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision
               OpDecorate %23 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
    %v3float = OpTypeVector %float 3
       %main = OpFunction %void None %8
          %9 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
          %y = OpVariable %_ptr_Function_int Function
          %z = OpVariable %_ptr_Function_int Function
               OpStore %x %int_0
               OpStore %y %int_0
               OpStore %z %int_0
               OpStore %x %int_1
               OpStore %z %int_1
         %17 = OpConvertSToF %float %int_1
         %18 = OpConvertSToF %float %int_0
         %19 = OpConvertSToF %float %int_1
         %21 = OpCompositeConstruct %v3float %17 %18 %19
         %22 = OpLoad %v4float %sk_FragColor
         %23 = OpVectorShuffle %v4float %22 %21 4 5 6 3
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
