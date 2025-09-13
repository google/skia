               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_Clockwise "sk_Clockwise"  ; id %7
               OpName %sk_FragColor "sk_FragColor"  ; id %10
               OpName %main "main"                  ; id %6

               ; Annotations
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %17 RelaxedPrecision
               OpDecorate %21 RelaxedPrecision
               OpDecorate %22 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input   ; BuiltIn FrontFacing
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
      %int_1 = OpConstant %int 1
     %int_n1 = OpConstant %int -1


               ; Function main
       %main = OpFunction %void None %15

         %16 = OpLabel
         %17 =   OpLoad %bool %sk_Clockwise         ; RelaxedPrecision
         %18 =   OpSelect %int %17 %int_1 %int_n1
         %21 =   OpConvertSToF %float %18           ; RelaxedPrecision
         %22 =   OpCompositeConstruct %v4float %21 %21 %21 %21  ; RelaxedPrecision
                 OpStore %sk_FragColor %22
                 OpReturn
               OpFunctionEnd
