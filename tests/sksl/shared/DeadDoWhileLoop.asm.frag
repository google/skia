               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %sk_FragColor "sk_FragColor"  ; id %7
               OpName %main "main"                  ; id %6

               ; Annotations
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_1 = OpConstant %float 1
         %20 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool


               ; Function main
       %main = OpFunction %void None %12

         %13 = OpLabel
                 OpBranch %14

         %14 = OpLabel
                 OpLoopMerge %18 %17 None
                 OpBranch %15

         %15 =     OpLabel
                     OpStore %sk_FragColor %20
                     OpBranch %16

         %16 =     OpLabel
                     OpBranch %17

         %17 =   OpLabel
                   OpBranchConditional %false %14 %18

         %18 = OpLabel
                 OpReturn
               OpFunctionEnd
