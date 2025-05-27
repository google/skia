               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %scratchVar "scratchVar"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %test_flat_b "test_flat_b"
               OpName %test_if_b "test_if_b"
               OpName %test_else_b "test_else_b"
               OpName %test_loop_if_b "test_loop_if_b"
               OpName %x "x"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %15 Binding 0
               OpDecorate %15 DescriptorSet 0
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %101 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
%_ptr_Private_int = OpTypePointer Private %int
 %scratchVar = OpVariable %_ptr_Private_int Private
      %int_0 = OpConstant %int 0
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %15 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %20 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %24 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
         %29 = OpTypeFunction %bool
       %true = OpConstantTrue %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
      %false = OpConstantFalse %bool
%_ptr_Function_int = OpTypePointer Function %int
         %76 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_entrypoint_v = OpFunction %void None %20
         %21 = OpLabel
         %25 = OpVariable %_ptr_Function_v2float Function
               OpStore %25 %24
         %27 = OpFunctionCall %v4float %main %25
               OpStore %sk_FragColor %27
               OpReturn
               OpFunctionEnd
%test_flat_b = OpFunction %bool None %29
         %30 = OpLabel
               OpReturnValue %true
               OpFunctionEnd
  %test_if_b = OpFunction %bool None %29
         %32 = OpLabel
         %33 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %35 = OpLoad %v4float %33
         %36 = OpCompositeExtract %float %35 1
         %37 = OpFOrdGreaterThan %bool %36 %float_0
               OpSelectionMerge %40 None
               OpBranchConditional %37 %38 %39
         %38 = OpLabel
               OpReturnValue %true
         %39 = OpLabel
         %42 = OpLoad %int %scratchVar
         %43 = OpIAdd %int %42 %int_1
               OpStore %scratchVar %43
               OpBranch %40
         %40 = OpLabel
         %44 = OpLoad %int %scratchVar
         %45 = OpIAdd %int %44 %int_1
               OpStore %scratchVar %45
               OpReturnValue %false
               OpFunctionEnd
%test_else_b = OpFunction %bool None %29
         %47 = OpLabel
         %48 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %49 = OpLoad %v4float %48
         %50 = OpCompositeExtract %float %49 1
         %51 = OpFOrdEqual %bool %50 %float_0
               OpSelectionMerge %54 None
               OpBranchConditional %51 %52 %53
         %52 = OpLabel
               OpReturnValue %false
         %53 = OpLabel
               OpReturnValue %true
         %54 = OpLabel
               OpUnreachable
               OpFunctionEnd
%test_loop_if_b = OpFunction %bool None %29
         %55 = OpLabel
          %x = OpVariable %_ptr_Function_int Function
               OpStore %x %int_0
               OpBranch %58
         %58 = OpLabel
               OpLoopMerge %62 %61 None
               OpBranch %59
         %59 = OpLabel
         %63 = OpLoad %int %x
         %64 = OpSLessThanEqual %bool %63 %int_1
               OpBranchConditional %64 %60 %62
         %60 = OpLabel
         %65 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %66 = OpLoad %v4float %65
         %67 = OpCompositeExtract %float %66 1
         %68 = OpFOrdEqual %bool %67 %float_0
               OpSelectionMerge %71 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
               OpReturnValue %false
         %70 = OpLabel
               OpReturnValue %true
         %71 = OpLabel
               OpBranch %61
         %61 = OpLabel
         %72 = OpLoad %int %x
         %73 = OpIAdd %int %72 %int_1
               OpStore %x %73
               OpBranch %58
         %62 = OpLabel
         %74 = OpLoad %int %scratchVar
         %75 = OpIAdd %int %74 %int_1
               OpStore %scratchVar %75
               OpReturnValue %true
               OpFunctionEnd
       %main = OpFunction %v4float None %76
         %77 = OpFunctionParameter %_ptr_Function_v2float
         %78 = OpLabel
         %92 = OpVariable %_ptr_Function_v4float Function
               OpStore %scratchVar %int_0
         %79 = OpFunctionCall %bool %test_flat_b
               OpSelectionMerge %81 None
               OpBranchConditional %79 %80 %81
         %80 = OpLabel
         %82 = OpFunctionCall %bool %test_if_b
               OpBranch %81
         %81 = OpLabel
         %83 = OpPhi %bool %false %78 %82 %80
               OpSelectionMerge %85 None
               OpBranchConditional %83 %84 %85
         %84 = OpLabel
         %86 = OpFunctionCall %bool %test_else_b
               OpBranch %85
         %85 = OpLabel
         %87 = OpPhi %bool %false %81 %86 %84
               OpSelectionMerge %89 None
               OpBranchConditional %87 %88 %89
         %88 = OpLabel
         %90 = OpFunctionCall %bool %test_loop_if_b
               OpBranch %89
         %89 = OpLabel
         %91 = OpPhi %bool %false %85 %90 %88
               OpSelectionMerge %96 None
               OpBranchConditional %91 %94 %95
         %94 = OpLabel
         %97 = OpAccessChain %_ptr_Uniform_v4float %15 %int_0
         %98 = OpLoad %v4float %97
               OpStore %92 %98
               OpBranch %96
         %95 = OpLabel
         %99 = OpAccessChain %_ptr_Uniform_v4float %15 %int_1
        %100 = OpLoad %v4float %99
               OpStore %92 %100
               OpBranch %96
         %96 = OpLabel
        %101 = OpLoad %v4float %92
               OpReturnValue %101
               OpFunctionEnd
