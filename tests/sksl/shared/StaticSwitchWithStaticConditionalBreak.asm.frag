               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %main "main"
               OpName %x "x"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %24 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
%_ptr_Function_float = OpTypePointer Function %float
    %float_0 = OpConstant %float 0
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
       %main = OpFunction %void None %8
          %9 = OpLabel
          %x = OpVariable %_ptr_Function_float Function
               OpStore %x %float_0
               OpSelectionMerge %15 None
               OpSwitch %int_0 %15 0 %16 1 %17
         %16 = OpLabel
               OpStore %x %float_0
         %19 = OpFOrdLessThan %bool %float_0 %float_1
               OpSelectionMerge %22 None
               OpBranchConditional %19 %21 %22
         %21 = OpLabel
               OpBranch %15
         %22 = OpLabel
               OpBranch %17
         %17 = OpLabel
               OpStore %x %float_1
               OpBranch %15
         %15 = OpLabel
         %23 = OpLoad %float %x
         %24 = OpCompositeConstruct %v4float %23 %23 %23 %23
               OpStore %sk_FragColor %24
               OpReturn
               OpFunctionEnd
