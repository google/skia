               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %inputVal "inputVal"
               OpName %expected "expected"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %33 RelaxedPrecision
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
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
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %23
         %24 = OpFunctionParameter %_ptr_Function_v2float
         %25 = OpLabel
   %inputVal = OpVariable %_ptr_Function_v4bool Function
   %expected = OpVariable %_ptr_Function_v4bool Function
         %89 = OpVariable %_ptr_Function_v4float Function
         %29 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %33 = OpLoad %v4float %29
         %34 = OpVectorShuffle %v4float %33 %33 0 0 1 2
         %35 = OpCompositeExtract %float %34 0
         %36 = OpFUnordNotEqual %bool %35 %float_0
         %37 = OpCompositeExtract %float %34 1
         %38 = OpFUnordNotEqual %bool %37 %float_0
         %39 = OpCompositeExtract %float %34 2
         %40 = OpFUnordNotEqual %bool %39 %float_0
         %41 = OpCompositeExtract %float %34 3
         %42 = OpFUnordNotEqual %bool %41 %float_0
         %43 = OpCompositeConstruct %v4bool %36 %38 %40 %42
               OpStore %inputVal %43
         %45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %46 = OpLoad %v4float %45
         %47 = OpVectorShuffle %v4float %46 %46 0 1 1 3
         %48 = OpCompositeExtract %float %47 0
         %49 = OpFUnordNotEqual %bool %48 %float_0
         %50 = OpCompositeExtract %float %47 1
         %51 = OpFUnordNotEqual %bool %50 %float_0
         %52 = OpCompositeExtract %float %47 2
         %53 = OpFUnordNotEqual %bool %52 %float_0
         %54 = OpCompositeExtract %float %47 3
         %55 = OpFUnordNotEqual %bool %54 %float_0
         %56 = OpCompositeConstruct %v4bool %49 %51 %53 %55
               OpStore %expected %56
         %59 = OpVectorShuffle %v2bool %43 %43 0 1
         %58 = OpAny %bool %59
         %61 = OpCompositeExtract %bool %56 0
         %62 = OpLogicalEqual %bool %58 %61
               OpSelectionMerge %64 None
               OpBranchConditional %62 %63 %64
         %63 = OpLabel
         %66 = OpVectorShuffle %v3bool %43 %43 0 1 2
         %65 = OpAny %bool %66
         %68 = OpCompositeExtract %bool %56 1
         %69 = OpLogicalEqual %bool %65 %68
               OpBranch %64
         %64 = OpLabel
         %70 = OpPhi %bool %false %25 %69 %63
               OpSelectionMerge %72 None
               OpBranchConditional %70 %71 %72
         %71 = OpLabel
         %73 = OpAny %bool %43
         %74 = OpCompositeExtract %bool %56 2
         %75 = OpLogicalEqual %bool %73 %74
               OpBranch %72
         %72 = OpLabel
         %76 = OpPhi %bool %false %64 %75 %71
               OpSelectionMerge %78 None
               OpBranchConditional %76 %77 %78
         %77 = OpLabel
         %79 = OpLogicalEqual %bool %false %61
               OpBranch %78
         %78 = OpLabel
         %80 = OpPhi %bool %false %72 %79 %77
               OpSelectionMerge %82 None
               OpBranchConditional %80 %81 %82
         %81 = OpLabel
         %83 = OpCompositeExtract %bool %56 1
               OpBranch %82
         %82 = OpLabel
         %84 = OpPhi %bool %false %78 %83 %81
               OpSelectionMerge %86 None
               OpBranchConditional %84 %85 %86
         %85 = OpLabel
         %87 = OpCompositeExtract %bool %56 2
               OpBranch %86
         %86 = OpLabel
         %88 = OpPhi %bool %false %82 %87 %85
               OpSelectionMerge %93 None
               OpBranchConditional %88 %91 %92
         %91 = OpLabel
         %94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %95 = OpLoad %v4float %94
               OpStore %89 %95
               OpBranch %93
         %92 = OpLabel
         %96 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %98 = OpLoad %v4float %96
               OpStore %89 %98
               OpBranch %93
         %93 = OpLabel
         %99 = OpLoad %v4float %89
               OpReturnValue %99
               OpFunctionEnd
