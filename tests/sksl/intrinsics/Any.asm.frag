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
               OpName %main "main"
               OpName %inputVal "inputVal"
               OpName %expected "expected"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %31 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
%_ptr_Function_v4bool = OpTypePointer Function %v4bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %false = OpConstantFalse %bool
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
      %int_1 = OpConstant %int 1
%_entrypoint_v = OpFunction %void None %12
         %13 = OpLabel
         %17 = OpVariable %_ptr_Function_v2float Function
               OpStore %17 %16
         %19 = OpFunctionCall %v4float %main %17
               OpStore %sk_FragColor %19
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %20
         %21 = OpFunctionParameter %_ptr_Function_v2float
         %22 = OpLabel
   %inputVal = OpVariable %_ptr_Function_v4bool Function
   %expected = OpVariable %_ptr_Function_v4bool Function
         %87 = OpVariable %_ptr_Function_v4float Function
         %27 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %31 = OpLoad %v4float %27
         %32 = OpVectorShuffle %v4float %31 %31 0 0 1 2
         %33 = OpCompositeExtract %float %32 0
         %34 = OpFUnordNotEqual %bool %33 %float_0
         %35 = OpCompositeExtract %float %32 1
         %36 = OpFUnordNotEqual %bool %35 %float_0
         %37 = OpCompositeExtract %float %32 2
         %38 = OpFUnordNotEqual %bool %37 %float_0
         %39 = OpCompositeExtract %float %32 3
         %40 = OpFUnordNotEqual %bool %39 %float_0
         %41 = OpCompositeConstruct %v4bool %34 %36 %38 %40
               OpStore %inputVal %41
         %43 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %44 = OpLoad %v4float %43
         %45 = OpVectorShuffle %v4float %44 %44 0 1 1 3
         %46 = OpCompositeExtract %float %45 0
         %47 = OpFUnordNotEqual %bool %46 %float_0
         %48 = OpCompositeExtract %float %45 1
         %49 = OpFUnordNotEqual %bool %48 %float_0
         %50 = OpCompositeExtract %float %45 2
         %51 = OpFUnordNotEqual %bool %50 %float_0
         %52 = OpCompositeExtract %float %45 3
         %53 = OpFUnordNotEqual %bool %52 %float_0
         %54 = OpCompositeConstruct %v4bool %47 %49 %51 %53
               OpStore %expected %54
         %57 = OpVectorShuffle %v2bool %41 %41 0 1
         %56 = OpAny %bool %57
         %59 = OpCompositeExtract %bool %54 0
         %60 = OpLogicalEqual %bool %56 %59
               OpSelectionMerge %62 None
               OpBranchConditional %60 %61 %62
         %61 = OpLabel
         %64 = OpVectorShuffle %v3bool %41 %41 0 1 2
         %63 = OpAny %bool %64
         %66 = OpCompositeExtract %bool %54 1
         %67 = OpLogicalEqual %bool %63 %66
               OpBranch %62
         %62 = OpLabel
         %68 = OpPhi %bool %false %22 %67 %61
               OpSelectionMerge %70 None
               OpBranchConditional %68 %69 %70
         %69 = OpLabel
         %71 = OpAny %bool %41
         %72 = OpCompositeExtract %bool %54 2
         %73 = OpLogicalEqual %bool %71 %72
               OpBranch %70
         %70 = OpLabel
         %74 = OpPhi %bool %false %62 %73 %69
               OpSelectionMerge %76 None
               OpBranchConditional %74 %75 %76
         %75 = OpLabel
         %77 = OpLogicalEqual %bool %false %59
               OpBranch %76
         %76 = OpLabel
         %78 = OpPhi %bool %false %70 %77 %75
               OpSelectionMerge %80 None
               OpBranchConditional %78 %79 %80
         %79 = OpLabel
         %81 = OpCompositeExtract %bool %54 1
               OpBranch %80
         %80 = OpLabel
         %82 = OpPhi %bool %false %76 %81 %79
               OpSelectionMerge %84 None
               OpBranchConditional %82 %83 %84
         %83 = OpLabel
         %85 = OpCompositeExtract %bool %54 2
               OpBranch %84
         %84 = OpLabel
         %86 = OpPhi %bool %false %80 %85 %83
               OpSelectionMerge %91 None
               OpBranchConditional %86 %89 %90
         %89 = OpLabel
         %92 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %93 = OpLoad %v4float %92
               OpStore %87 %93
               OpBranch %91
         %90 = OpLabel
         %94 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %96 = OpLoad %v4float %94
               OpStore %87 %96
               OpBranch %91
         %91 = OpLabel
         %97 = OpLoad %v4float %87
               OpReturnValue %97
               OpFunctionEnd
