               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %main "main"                  ; id %6
               OpName %Test "Test"                  ; id %15
               OpMemberName %Test 0 "x"
               OpMemberName %Test 1 "y"
               OpMemberName %Test 2 "z"
               OpName %t "t"                        ; id %14

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %Test 0 Offset 0
               OpMemberDecorate %Test 1 Offset 4
               OpMemberDecorate %Test 2 Offset 8
               OpDecorate %22 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
       %Test = OpTypeStruct %int %int %int
%_ptr_Function_Test = OpTypePointer Function %Test
      %int_0 = OpConstant %int 0
%_ptr_Function_int = OpTypePointer Function %int
%_ptr_Output_float = OpTypePointer Output %float


               ; Function main
       %main = OpFunction %void None %12

         %13 = OpLabel
          %t =   OpVariable %_ptr_Function_Test Function
         %18 =   OpAccessChain %_ptr_Function_int %t %int_0
                 OpStore %18 %int_0
         %20 =   OpAccessChain %_ptr_Function_int %t %int_0
         %21 =   OpLoad %int %20
         %22 =   OpConvertSToF %float %21           ; RelaxedPrecision
         %23 =   OpAccessChain %_ptr_Output_float %sk_FragColor %int_0
                 OpStore %23 %22
                 OpReturn
               OpFunctionEnd
