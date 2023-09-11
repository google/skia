               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %x "x"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %x RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
          %9 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %13 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %17 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
         %23 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
 %float_0_25 = OpConstant %float 0.25
      %int_2 = OpConstant %int 2
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %9
         %10 = OpLabel
         %14 = OpVariable %_ptr_Function_v2float Function
               OpStore %14 %13
         %16 = OpFunctionCall %v4float %main %14
               OpStore %sk_FragColor %16
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %17
         %18 = OpFunctionParameter %_ptr_Function_v2float
         %19 = OpLabel
          %x = OpVariable %_ptr_Function_v4float Function
               OpStore %x %23
               OpBranch %24
         %24 = OpLabel
               OpLoopMerge %28 %27 None
               OpBranch %25
         %25 = OpLabel
         %29 = OpLoad %v4float %x
         %30 = OpCompositeExtract %float %29 3
         %31 = OpFOrdEqual %bool %30 %float_1
               OpBranchConditional %31 %26 %28
         %26 = OpLabel
         %33 = OpAccessChain %_ptr_Function_float %x %int_0
         %37 = OpLoad %float %33
         %39 = OpFSub %float %37 %float_0_25
               OpStore %33 %39
         %40 = OpLoad %v4float %x
         %41 = OpCompositeExtract %float %40 0
         %42 = OpFOrdLessThanEqual %bool %41 %float_0
               OpSelectionMerge %44 None
               OpBranchConditional %42 %43 %44
         %43 = OpLabel
               OpBranch %28
         %44 = OpLabel
               OpBranch %27
         %27 = OpLabel
               OpBranch %24
         %28 = OpLabel
               OpBranch %45
         %45 = OpLabel
               OpLoopMerge %49 %48 None
               OpBranch %46
         %46 = OpLabel
         %50 = OpLoad %v4float %x
         %51 = OpCompositeExtract %float %50 2
         %52 = OpFOrdGreaterThan %bool %51 %float_0
               OpBranchConditional %52 %47 %49
         %47 = OpLabel
         %53 = OpAccessChain %_ptr_Function_float %x %int_2
         %55 = OpLoad %float %53
         %56 = OpFSub %float %55 %float_0_25
               OpStore %53 %56
         %57 = OpLoad %v4float %x
         %58 = OpCompositeExtract %float %57 3
         %59 = OpFOrdEqual %bool %58 %float_1
               OpSelectionMerge %61 None
               OpBranchConditional %59 %60 %61
         %60 = OpLabel
               OpBranch %48
         %61 = OpLabel
         %62 = OpAccessChain %_ptr_Function_float %x %int_1
               OpStore %62 %float_0
               OpBranch %48
         %48 = OpLabel
               OpBranch %45
         %49 = OpLabel
         %64 = OpLoad %v4float %x
               OpReturnValue %64
               OpFunctionEnd
