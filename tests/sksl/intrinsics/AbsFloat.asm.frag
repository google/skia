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
               OpMemberDecorate %_UniformBuffer 1 Offset 16
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %100 RelaxedPrecision
               OpDecorate %103 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
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
 %float_1_25 = OpConstant %float 1.25
 %float_0_75 = OpConstant %float 0.75
 %float_2_25 = OpConstant %float 2.25
         %31 = OpConstantComposite %v4float %float_1_25 %float_0 %float_0_75 %float_2_25
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
         %79 = OpConstantComposite %v2float %float_1_25 %float_0
         %86 = OpConstantComposite %v3float %float_1_25 %float_0 %float_0_75
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
         %94 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %31
         %34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %38 = OpLoad %v4float %34
         %39 = OpCompositeExtract %float %38 0
         %33 = OpExtInst %float %1 FAbs %39
         %40 = OpFOrdEqual %bool %33 %float_1_25
               OpSelectionMerge %42 None
               OpBranchConditional %40 %41 %42
         %41 = OpLabel
         %44 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %45 = OpLoad %v4float %44
         %46 = OpVectorShuffle %v2float %45 %45 0 1
         %43 = OpExtInst %v2float %1 FAbs %46
         %47 = OpVectorShuffle %v2float %31 %31 0 1
         %48 = OpFOrdEqual %v2bool %43 %47
         %50 = OpAll %bool %48
               OpBranch %42
         %42 = OpLabel
         %51 = OpPhi %bool %false %25 %50 %41
               OpSelectionMerge %53 None
               OpBranchConditional %51 %52 %53
         %52 = OpLabel
         %55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %56 = OpLoad %v4float %55
         %57 = OpVectorShuffle %v3float %56 %56 0 1 2
         %54 = OpExtInst %v3float %1 FAbs %57
         %59 = OpVectorShuffle %v3float %31 %31 0 1 2
         %60 = OpFOrdEqual %v3bool %54 %59
         %62 = OpAll %bool %60
               OpBranch %53
         %53 = OpLabel
         %63 = OpPhi %bool %false %42 %62 %52
               OpSelectionMerge %65 None
               OpBranchConditional %63 %64 %65
         %64 = OpLabel
         %67 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %68 = OpLoad %v4float %67
         %66 = OpExtInst %v4float %1 FAbs %68
         %69 = OpFOrdEqual %v4bool %66 %31
         %71 = OpAll %bool %69
               OpBranch %65
         %65 = OpLabel
         %72 = OpPhi %bool %false %53 %71 %64
               OpSelectionMerge %74 None
               OpBranchConditional %72 %73 %74
         %73 = OpLabel
               OpBranch %74
         %74 = OpLabel
         %76 = OpPhi %bool %false %65 %true %73
               OpSelectionMerge %78 None
               OpBranchConditional %76 %77 %78
         %77 = OpLabel
         %80 = OpVectorShuffle %v2float %31 %31 0 1
         %81 = OpFOrdEqual %v2bool %79 %80
         %82 = OpAll %bool %81
               OpBranch %78
         %78 = OpLabel
         %83 = OpPhi %bool %false %74 %82 %77
               OpSelectionMerge %85 None
               OpBranchConditional %83 %84 %85
         %84 = OpLabel
         %87 = OpVectorShuffle %v3float %31 %31 0 1 2
         %88 = OpFOrdEqual %v3bool %86 %87
         %89 = OpAll %bool %88
               OpBranch %85
         %85 = OpLabel
         %90 = OpPhi %bool %false %78 %89 %84
               OpSelectionMerge %92 None
               OpBranchConditional %90 %91 %92
         %91 = OpLabel
               OpBranch %92
         %92 = OpLabel
         %93 = OpPhi %bool %false %85 %true %91
               OpSelectionMerge %97 None
               OpBranchConditional %93 %95 %96
         %95 = OpLabel
         %98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %100 = OpLoad %v4float %98
               OpStore %94 %100
               OpBranch %97
         %96 = OpLabel
        %101 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %103 = OpLoad %v4float %101
               OpStore %94 %103
               OpBranch %97
         %97 = OpLabel
        %104 = OpLoad %v4float %94
               OpReturnValue %104
               OpFunctionEnd
