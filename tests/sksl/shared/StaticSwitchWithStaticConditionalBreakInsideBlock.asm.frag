               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_Clockwise %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %main "main"
               OpName %x "x"
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
%_ptr_Function_float = OpTypePointer Function %float
    %float_0 = OpConstant %float 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %25 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %main = OpFunction %void None %11
         %12 = OpLabel
          %x = OpVariable %_ptr_Function_float Function
               OpStore %x %float_0
               OpSelectionMerge %18 None
               OpSwitch %int_0 %18 0 %19 1 %20
         %19 = OpLabel
               OpStore %x %float_0
         %22 = OpFOrdLessThan %bool %float_0 %float_1
               OpSelectionMerge %24 None
               OpBranchConditional %22 %23 %24
         %23 = OpLabel
               OpStore %sk_FragColor %25
               OpBranch %18
         %24 = OpLabel
               OpBranch %20
         %20 = OpLabel
               OpStore %x %float_1
               OpBranch %18
         %18 = OpLabel
               OpReturn
               OpFunctionEnd
