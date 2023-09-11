               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %11 = OpTypeFunction %void
    %float_1 = OpConstant %float 1
         %19 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
      %false = OpConstantFalse %bool
       %main = OpFunction %void None %11
         %12 = OpLabel
               OpBranch %13
         %13 = OpLabel
               OpLoopMerge %17 %16 None
               OpBranch %14
         %14 = OpLabel
               OpStore %sk_FragColor %19
               OpBranch %15
         %15 = OpLabel
               OpBranch %16
         %16 = OpLabel
               OpBranchConditional %false %13 %17
         %17 = OpLabel
               OpReturn
               OpFunctionEnd
