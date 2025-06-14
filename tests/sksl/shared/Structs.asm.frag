               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %A "A"                        ; id %12
               OpMemberName %A 0 "x"
               OpMemberName %A 1 "y"
               OpName %a1 "a1"                      ; id %11
               OpName %B "B"                        ; id %17
               OpMemberName %B 0 "x"
               OpMemberName %B 1 "y"
               OpMemberName %B 2 "z"
               OpName %b1 "b1"                      ; id %14
               OpName %main "main"                  ; id %6

               ; Annotations
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
               OpDecorate %30 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
          %A = OpTypeStruct %int %int
%_ptr_Private_A = OpTypePointer Private %A
         %a1 = OpVariable %_ptr_Private_A Private
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2       ; ArrayStride 16
          %B = OpTypeStruct %float %_arr_float_int_2 %A
%_ptr_Private_B = OpTypePointer Private %B
         %b1 = OpVariable %_ptr_Private_B Private
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
      %int_0 = OpConstant %int 0
%_ptr_Private_int = OpTypePointer Private %int
    %float_0 = OpConstant %float 0
%_ptr_Private_float = OpTypePointer Private %float
%_ptr_Output_float = OpTypePointer Output %float


               ; Function main
       %main = OpFunction %void None %20

         %21 = OpLabel
         %23 =   OpAccessChain %_ptr_Private_int %a1 %int_0
                 OpStore %23 %int_0
         %26 =   OpAccessChain %_ptr_Private_float %b1 %int_0
                 OpStore %26 %float_0
         %28 =   OpAccessChain %_ptr_Private_int %a1 %int_0
         %29 =   OpLoad %int %28
         %30 =   OpConvertSToF %float %29           ; RelaxedPrecision
         %31 =   OpAccessChain %_ptr_Private_float %b1 %int_0
         %32 =   OpLoad %float %31
         %33 =   OpFAdd %float %30 %32              ; RelaxedPrecision
         %34 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %34 %33
                 OpReturn
               OpFunctionEnd
