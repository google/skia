               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %A "A"
               OpMemberName %A 0 "x"
               OpMemberName %A 1 "y"
               OpName %a1 "a1"
               OpName %B "B"
               OpMemberName %B 0 "x"
               OpMemberName %B 1 "y"
               OpMemberName %B 2 "z"
               OpName %b1 "b1"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %A 0 Offset 0
               OpMemberDecorate %A 1 Offset 4
               OpDecorate %_arr_float_int_2 ArrayStride 16
               OpMemberDecorate %B 0 Offset 0
               OpMemberDecorate %B 1 Offset 16
               OpMemberDecorate %B 2 Offset 48
               OpMemberDecorate %B 2 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
          %A = OpTypeStruct %int %int
%_ptr_Private_A = OpTypePointer Private %A
         %a1 = OpVariable %_ptr_Private_A Private
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
          %B = OpTypeStruct %float %_arr_float_int_2 %A
%_ptr_Private_B = OpTypePointer Private %B
         %b1 = OpVariable %_ptr_Private_B Private
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Private_int = OpTypePointer Private %int
    %float_0 = OpConstant %float 0
%_ptr_Private_float = OpTypePointer Private %float
%_ptr_Output_float = OpTypePointer Output %float
       %main = OpFunction %void None %17
         %18 = OpLabel
         %20 = OpAccessChain %_ptr_Private_int %a1 %int_0
               OpStore %20 %int_0
         %23 = OpAccessChain %_ptr_Private_float %b1 %int_0
               OpStore %23 %float_0
         %25 = OpAccessChain %_ptr_Private_int %a1 %int_0
         %26 = OpLoad %int %25
         %27 = OpConvertSToF %float %26
         %28 = OpAccessChain %_ptr_Private_float %b1 %int_0
         %29 = OpLoad %float %28
         %30 = OpFAdd %float %27 %29
         %31 = OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
               OpStore %31 %30
               OpReturn
               OpFunctionEnd
