               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "I"
               OpMemberName %_UniformBuffer 1 "N"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %expectedX "expectedX"
               OpName %expectedXY "expectedXY"
               OpName %expectedXYZ "expectedXYZ"
               OpName %expectedXYZW "expectedXYZW"
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
               OpMemberDecorate %_UniformBuffer 3 Offset 48
               OpMemberDecorate %_UniformBuffer 3 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %expectedX RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %expectedXY RelaxedPrecision
               OpDecorate %expectedXYZ RelaxedPrecision
               OpDecorate %expectedXYZW RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %80 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %23 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_996878592 = OpConstant %float 996878592
%float_n1_99999996e_34 = OpConstant %float -1.99999996e+34
  %float_n49 = OpConstant %float -49
 %float_n169 = OpConstant %float -169
  %float_202 = OpConstant %float 202
         %35 = OpConstantComposite %v2float %float_n169 %float_202
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
 %float_n379 = OpConstant %float -379
  %float_454 = OpConstant %float 454
 %float_n529 = OpConstant %float -529
         %42 = OpConstantComposite %v3float %float_n379 %float_454 %float_n529
%_ptr_Function_v4float = OpTypePointer Function %v4float
 %float_n699 = OpConstant %float -699
  %float_838 = OpConstant %float 838
 %float_n977 = OpConstant %float -977
 %float_1116 = OpConstant %float 1116
         %49 = OpConstantComposite %v4float %float_n699 %float_838 %float_n977 %float_1116
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
     %v2bool = OpTypeVector %bool 2
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
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
  %expectedX = OpVariable %_ptr_Function_float Function
 %expectedXY = OpVariable %_ptr_Function_v2float Function
%expectedXYZ = OpVariable %_ptr_Function_v3float Function
%expectedXYZW = OpVariable %_ptr_Function_v4float Function
        %113 = OpVariable %_ptr_Function_v4float Function
         %28 = OpExtInst %float %1 Reflect %float_996878592 %float_n1_99999996e_34
               OpStore %expectedX %28
               OpStore %expectedX %float_n49
               OpStore %expectedXY %35
               OpStore %expectedXYZ %42
               OpStore %expectedXYZW %49
         %52 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %56 = OpLoad %v4float %52
         %57 = OpCompositeExtract %float %56 0
         %58 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %60 = OpLoad %v4float %58
         %61 = OpCompositeExtract %float %60 0
         %51 = OpExtInst %float %1 Reflect %57 %61
         %62 = OpFOrdEqual %bool %51 %float_n49
               OpSelectionMerge %64 None
               OpBranchConditional %62 %63 %64
         %63 = OpLabel
         %66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %67 = OpLoad %v4float %66
         %68 = OpVectorShuffle %v2float %67 %67 0 1
         %69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %70 = OpLoad %v4float %69
         %71 = OpVectorShuffle %v2float %70 %70 0 1
         %65 = OpExtInst %v2float %1 Reflect %68 %71
         %72 = OpFOrdEqual %v2bool %65 %35
         %74 = OpAll %bool %72
               OpBranch %64
         %64 = OpLabel
         %75 = OpPhi %bool %false %25 %74 %63
               OpSelectionMerge %77 None
               OpBranchConditional %75 %76 %77
         %76 = OpLabel
         %79 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %80 = OpLoad %v4float %79
         %81 = OpVectorShuffle %v3float %80 %80 0 1 2
         %82 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %83 = OpLoad %v4float %82
         %84 = OpVectorShuffle %v3float %83 %83 0 1 2
         %78 = OpExtInst %v3float %1 Reflect %81 %84
         %85 = OpFOrdEqual %v3bool %78 %42
         %87 = OpAll %bool %85
               OpBranch %77
         %77 = OpLabel
         %88 = OpPhi %bool %false %64 %87 %76
               OpSelectionMerge %90 None
               OpBranchConditional %88 %89 %90
         %89 = OpLabel
         %92 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %93 = OpLoad %v4float %92
         %94 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %95 = OpLoad %v4float %94
         %91 = OpExtInst %v4float %1 Reflect %93 %95
         %96 = OpFOrdEqual %v4bool %91 %49
         %98 = OpAll %bool %96
               OpBranch %90
         %90 = OpLabel
         %99 = OpPhi %bool %false %77 %98 %89
               OpSelectionMerge %101 None
               OpBranchConditional %99 %100 %101
        %100 = OpLabel
               OpBranch %101
        %101 = OpLabel
        %103 = OpPhi %bool %false %90 %true %100
               OpSelectionMerge %105 None
               OpBranchConditional %103 %104 %105
        %104 = OpLabel
               OpBranch %105
        %105 = OpLabel
        %106 = OpPhi %bool %false %101 %true %104
               OpSelectionMerge %108 None
               OpBranchConditional %106 %107 %108
        %107 = OpLabel
               OpBranch %108
        %108 = OpLabel
        %109 = OpPhi %bool %false %105 %true %107
               OpSelectionMerge %111 None
               OpBranchConditional %109 %110 %111
        %110 = OpLabel
               OpBranch %111
        %111 = OpLabel
        %112 = OpPhi %bool %false %108 %true %110
               OpSelectionMerge %116 None
               OpBranchConditional %112 %114 %115
        %114 = OpLabel
        %117 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %119 = OpLoad %v4float %117
               OpStore %113 %119
               OpBranch %116
        %115 = OpLabel
        %120 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %122 = OpLoad %v4float %120
               OpStore %113 %122
               OpBranch %116
        %116 = OpLabel
        %123 = OpLoad %v4float %113
               OpReturnValue %123
               OpFunctionEnd
