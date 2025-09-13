               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %main "main"                  ; id %6
               OpName %x "x"                        ; id %14
               OpName %y "y"                        ; id %17
               OpName %z "z"                        ; id %18

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %30 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %int_1 = OpConstant %int 1
      %false = OpConstantFalse %bool
    %v3float = OpTypeVector %float 3


               ; Function main
       %main = OpFunction %void None %12

         %13 = OpLabel
          %x =   OpVariable %_ptr_Function_int Function
          %y =   OpVariable %_ptr_Function_int Function
          %z =   OpVariable %_ptr_Function_int Function
                 OpStore %x %int_0
                 OpStore %y %int_0
                 OpStore %z %int_0
                 OpSelectionMerge %22 None
                 OpBranchConditional %true %21 %22

         %21 =     OpLabel
                     OpStore %x %int_1
                     OpBranch %22

         %22 = OpLabel
                 OpSelectionMerge %26 None
                 OpBranchConditional %false %25 %26

         %25 =     OpLabel
                     OpStore %y %int_1
                     OpBranch %26

         %26 = OpLabel
                 OpSelectionMerge %28 None
                 OpBranchConditional %true %27 %28

         %27 =     OpLabel
                     OpStore %z %int_1
                     OpBranch %28

         %28 = OpLabel
         %29 =   OpLoad %int %x
         %30 =   OpConvertSToF %float %29           ; RelaxedPrecision
         %31 =   OpLoad %int %y
         %32 =   OpConvertSToF %float %31           ; RelaxedPrecision
         %33 =   OpLoad %int %z
         %34 =   OpConvertSToF %float %33           ; RelaxedPrecision
         %36 =   OpCompositeConstruct %v3float %30 %32 %34  ; RelaxedPrecision
         %37 =   OpLoad %v4float %sk_FragColor              ; RelaxedPrecision
         %38 =   OpVectorShuffle %v4float %37 %36 4 5 6 3   ; RelaxedPrecision
                 OpStore %sk_FragColor %38
                 OpReturn
               OpFunctionEnd
