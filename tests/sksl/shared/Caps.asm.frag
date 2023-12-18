               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %main "main"
               OpName %x "x"
               OpName %y "y"
               OpName %z "z"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %27 RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
          %8 = OpTypeFunction %void
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
       %bool = OpTypeBool
       %true = OpConstantTrue %bool
      %int_1 = OpConstant %int 1
      %false = OpConstantFalse %bool
    %v3float = OpTypeVector %float 3
       %main = OpFunction %void None %8
          %9 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
          %y = OpVariable %_ptr_Function_int Function
          %z = OpVariable %_ptr_Function_int Function
               OpStore %x %int_0
               OpStore %y %int_0
               OpStore %z %int_0
               OpSelectionMerge %19 None
               OpBranchConditional %true %18 %19
         %18 = OpLabel
               OpStore %x %int_1
               OpBranch %19
         %19 = OpLabel
               OpSelectionMerge %23 None
               OpBranchConditional %false %22 %23
         %22 = OpLabel
               OpStore %y %int_1
               OpBranch %23
         %23 = OpLabel
               OpSelectionMerge %25 None
               OpBranchConditional %true %24 %25
         %24 = OpLabel
               OpStore %z %int_1
               OpBranch %25
         %25 = OpLabel
         %26 = OpLoad %int %x
         %27 = OpConvertSToF %float %26
         %28 = OpLoad %int %y
         %29 = OpConvertSToF %float %28
         %30 = OpLoad %int %z
         %31 = OpConvertSToF %float %30
         %33 = OpCompositeConstruct %v3float %27 %29 %31
         %34 = OpLoad %v4float %sk_FragColor
         %35 = OpVectorShuffle %v4float %34 %33 4 5 6 3
               OpStore %sk_FragColor %35
               OpReturn
               OpFunctionEnd
