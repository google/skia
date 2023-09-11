               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %main "main"
               OpName %i "i"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %16 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
%_ptr_Output_float = OpTypePointer Output %float
      %int_0 = OpConstant %int 0
       %main = OpFunction %void None %8
          %9 = OpLabel
          %i = OpVariable %_ptr_Function_int Function
               OpStore %i %int_1
               OpStore %i %int_3
         %15 = OpIMul %int %int_1 %int_3
         %16 = OpConvertSToF %float %int_3
         %17 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %17 %16
               OpReturn
               OpFunctionEnd
