               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %expected "expected"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %expected RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
 %float_0_75 = OpConstant %float 0.75
    %float_1 = OpConstant %float 1
         %30 = OpConstantComposite %v4float %float_0 %float_0 %float_0_75 %float_1
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %46 = OpConstantComposite %v2float %float_1 %float_1
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
         %59 = OpConstantComposite %v3float %float_0 %float_0 %float_0
         %60 = OpConstantComposite %v3float %float_1 %float_1 %float_1
     %v3bool = OpTypeVector %bool 3
         %71 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
         %72 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %89 = OpConstantComposite %v3float %float_0 %float_0 %float_0_75
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
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
   %expected = OpVariable %_ptr_Function_v4float Function
         %97 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %30
         %33 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %37 = OpLoad %v4float %33
         %38 = OpCompositeExtract %float %37 0
         %32 = OpExtInst %float %1 FClamp %38 %float_0 %float_1
         %39 = OpFOrdEqual %bool %32 %float_0
               OpSelectionMerge %41 None
               OpBranchConditional %39 %40 %41
         %40 = OpLabel
         %43 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %44 = OpLoad %v4float %43
         %45 = OpVectorShuffle %v2float %44 %44 0 1
         %42 = OpExtInst %v2float %1 FClamp %45 %19 %46
         %47 = OpVectorShuffle %v2float %30 %30 0 1
         %48 = OpFOrdEqual %v2bool %42 %47
         %50 = OpAll %bool %48
               OpBranch %41
         %41 = OpLabel
         %51 = OpPhi %bool %false %25 %50 %40
               OpSelectionMerge %53 None
               OpBranchConditional %51 %52 %53
         %52 = OpLabel
         %55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %56 = OpLoad %v4float %55
         %57 = OpVectorShuffle %v3float %56 %56 0 1 2
         %54 = OpExtInst %v3float %1 FClamp %57 %59 %60
         %61 = OpVectorShuffle %v3float %30 %30 0 1 2
         %62 = OpFOrdEqual %v3bool %54 %61
         %64 = OpAll %bool %62
               OpBranch %53
         %53 = OpLabel
         %65 = OpPhi %bool %false %41 %64 %52
               OpSelectionMerge %67 None
               OpBranchConditional %65 %66 %67
         %66 = OpLabel
         %69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %70 = OpLoad %v4float %69
         %68 = OpExtInst %v4float %1 FClamp %70 %71 %72
         %73 = OpFOrdEqual %v4bool %68 %30
         %75 = OpAll %bool %73
               OpBranch %67
         %67 = OpLabel
         %76 = OpPhi %bool %false %53 %75 %66
               OpSelectionMerge %78 None
               OpBranchConditional %76 %77 %78
         %77 = OpLabel
               OpBranch %78
         %78 = OpLabel
         %80 = OpPhi %bool %false %67 %true %77
               OpSelectionMerge %82 None
               OpBranchConditional %80 %81 %82
         %81 = OpLabel
         %83 = OpVectorShuffle %v2float %30 %30 0 1
         %84 = OpFOrdEqual %v2bool %19 %83
         %85 = OpAll %bool %84
               OpBranch %82
         %82 = OpLabel
         %86 = OpPhi %bool %false %78 %85 %81
               OpSelectionMerge %88 None
               OpBranchConditional %86 %87 %88
         %87 = OpLabel
         %90 = OpVectorShuffle %v3float %30 %30 0 1 2
         %91 = OpFOrdEqual %v3bool %89 %90
         %92 = OpAll %bool %91
               OpBranch %88
         %88 = OpLabel
         %93 = OpPhi %bool %false %82 %92 %87
               OpSelectionMerge %95 None
               OpBranchConditional %93 %94 %95
         %94 = OpLabel
               OpBranch %95
         %95 = OpLabel
         %96 = OpPhi %bool %false %88 %true %94
               OpSelectionMerge %100 None
               OpBranchConditional %96 %98 %99
         %98 = OpLabel
        %101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %103 = OpLoad %v4float %101
               OpStore %97 %103
               OpBranch %100
         %99 = OpLabel
        %104 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %106 = OpLoad %v4float %104
               OpStore %97 %106
               OpBranch %100
        %100 = OpLabel
        %107 = OpLoad %v4float %97
               OpReturnValue %107
               OpFunctionEnd
