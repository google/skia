               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %unpremul_h4h4 "unpremul_h4h4"
               OpName %live_fn_h4h4h4 "live_fn_h4h4h4"
               OpName %main "main"
               OpName %a "a"
               OpName %b "b"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %26 RelaxedPrecision
               OpDecorate %27 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %a RelaxedPrecision
               OpDecorate %b RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %9 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %18 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %23 = OpTypeFunction %v4float %_ptr_Function_v4float
    %v3float = OpTypeVector %float 3
%float_9_99999975en05 = OpConstant %float 9.99999975e-05
    %float_1 = OpConstant %float 1
         %42 = OpTypeFunction %v4float %_ptr_Function_v4float %_ptr_Function_v4float
         %49 = OpTypeFunction %v4float %_ptr_Function_v2float
    %float_3 = OpConstant %float 3
         %55 = OpConstantComposite %v4float %float_3 %float_3 %float_3 %float_3
   %float_n5 = OpConstant %float -5
         %58 = OpConstantComposite %v4float %float_n5 %float_n5 %float_n5 %float_n5
         %61 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
         %66 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
     %v4bool = OpTypeVector %bool 4
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %19 = OpVariable %_ptr_Function_v2float Function
               OpStore %19 %18
         %21 = OpFunctionCall %v4float %main %19
               OpStore %sk_FragColor %21
               OpReturn
               OpFunctionEnd
%unpremul_h4h4 = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v4float
         %25 = OpLabel
         %26 = OpLoad %v4float %24
         %27 = OpVectorShuffle %v3float %26 %26 0 1 2
         %30 = OpLoad %v4float %24
         %31 = OpCompositeExtract %float %30 3
         %29 = OpExtInst %float %1 FMax %31 %float_9_99999975en05
         %34 = OpFDiv %float %float_1 %29
         %35 = OpVectorTimesScalar %v3float %27 %34
         %36 = OpCompositeExtract %float %35 0
         %37 = OpCompositeExtract %float %35 1
         %38 = OpCompositeExtract %float %35 2
         %39 = OpLoad %v4float %24
         %40 = OpCompositeExtract %float %39 3
         %41 = OpCompositeConstruct %v4float %36 %37 %38 %40
               OpReturnValue %41
               OpFunctionEnd
%live_fn_h4h4h4 = OpFunction %v4float None %42
         %43 = OpFunctionParameter %_ptr_Function_v4float
         %44 = OpFunctionParameter %_ptr_Function_v4float
         %45 = OpLabel
         %46 = OpLoad %v4float %43
         %47 = OpLoad %v4float %44
         %48 = OpFAdd %v4float %46 %47
               OpReturnValue %48
               OpFunctionEnd
       %main = OpFunction %v4float None %49
         %50 = OpFunctionParameter %_ptr_Function_v2float
         %51 = OpLabel
          %a = OpVariable %_ptr_Function_v4float Function
          %b = OpVariable %_ptr_Function_v4float Function
         %56 = OpVariable %_ptr_Function_v4float Function
         %59 = OpVariable %_ptr_Function_v4float Function
         %62 = OpVariable %_ptr_Function_v4float Function
         %75 = OpVariable %_ptr_Function_v4float Function
               OpStore %56 %55
               OpStore %59 %58
         %60 = OpFunctionCall %v4float %live_fn_h4h4h4 %56 %59
               OpStore %a %60
               OpStore %62 %61
         %63 = OpFunctionCall %v4float %unpremul_h4h4 %62
               OpStore %b %63
         %67 = OpFUnordNotEqual %v4bool %60 %66
         %69 = OpAny %bool %67
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
         %72 = OpFUnordNotEqual %v4bool %63 %66
         %73 = OpAny %bool %72
               OpBranch %71
         %71 = OpLabel
         %74 = OpPhi %bool %false %51 %73 %70
               OpSelectionMerge %78 None
               OpBranchConditional %74 %76 %77
         %76 = OpLabel
         %79 = OpAccessChain %_ptr_Uniform_v4float %9 %int_0
         %83 = OpLoad %v4float %79
               OpStore %75 %83
               OpBranch %78
         %77 = OpLabel
         %84 = OpAccessChain %_ptr_Uniform_v4float %9 %int_1
         %86 = OpLoad %v4float %84
               OpStore %75 %86
               OpBranch %78
         %78 = OpLabel
         %87 = OpLoad %v4float %75
               OpReturnValue %87
               OpFunctionEnd
