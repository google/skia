               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %expectedX RelaxedPrecision
               OpDecorate %25 RelaxedPrecision
               OpDecorate %expectedXY RelaxedPrecision
               OpDecorate %expectedXYZ RelaxedPrecision
               OpDecorate %expectedXYZW RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %117 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_float = OpTypePointer Function %float
%float_996878592 = OpConstant %float 996878592
%float_n1_99999996e_34 = OpConstant %float -1.99999996e+34
  %float_n49 = OpConstant %float -49
 %float_n169 = OpConstant %float -169
  %float_202 = OpConstant %float 202
         %32 = OpConstantComposite %v2float %float_n169 %float_202
    %v3float = OpTypeVector %float 3
%_ptr_Function_v3float = OpTypePointer Function %v3float
 %float_n379 = OpConstant %float -379
  %float_454 = OpConstant %float 454
 %float_n529 = OpConstant %float -529
         %39 = OpConstantComposite %v3float %float_n379 %float_454 %float_n529
%_ptr_Function_v4float = OpTypePointer Function %v4float
 %float_n699 = OpConstant %float -699
  %float_838 = OpConstant %float 838
 %float_n977 = OpConstant %float -977
 %float_1116 = OpConstant %float 1116
         %46 = OpConstantComposite %v4float %float_n699 %float_838 %float_n977 %float_1116
       %bool = OpTypeBool
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
  %expectedX = OpVariable %_ptr_Function_float Function
 %expectedXY = OpVariable %_ptr_Function_v2float Function
%expectedXYZ = OpVariable %_ptr_Function_v3float Function
%expectedXYZW = OpVariable %_ptr_Function_v4float Function
        %111 = OpVariable %_ptr_Function_v4float Function
         %25 = OpExtInst %float %1 Reflect %float_996878592 %float_n1_99999996e_34
               OpStore %expectedX %25
               OpStore %expectedX %float_n49
               OpStore %expectedXY %32
               OpStore %expectedXYZ %39
               OpStore %expectedXYZW %46
         %50 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %54 = OpLoad %v4float %50
         %55 = OpCompositeExtract %float %54 0
         %56 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %58 = OpLoad %v4float %56
         %59 = OpCompositeExtract %float %58 0
         %49 = OpExtInst %float %1 Reflect %55 %59
         %60 = OpFOrdEqual %bool %49 %float_n49
               OpSelectionMerge %62 None
               OpBranchConditional %60 %61 %62
         %61 = OpLabel
         %64 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %65 = OpLoad %v4float %64
         %66 = OpVectorShuffle %v2float %65 %65 0 1
         %67 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %68 = OpLoad %v4float %67
         %69 = OpVectorShuffle %v2float %68 %68 0 1
         %63 = OpExtInst %v2float %1 Reflect %66 %69
         %70 = OpFOrdEqual %v2bool %63 %32
         %72 = OpAll %bool %70
               OpBranch %62
         %62 = OpLabel
         %73 = OpPhi %bool %false %22 %72 %61
               OpSelectionMerge %75 None
               OpBranchConditional %73 %74 %75
         %74 = OpLabel
         %77 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %78 = OpLoad %v4float %77
         %79 = OpVectorShuffle %v3float %78 %78 0 1 2
         %80 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %81 = OpLoad %v4float %80
         %82 = OpVectorShuffle %v3float %81 %81 0 1 2
         %76 = OpExtInst %v3float %1 Reflect %79 %82
         %83 = OpFOrdEqual %v3bool %76 %39
         %85 = OpAll %bool %83
               OpBranch %75
         %75 = OpLabel
         %86 = OpPhi %bool %false %62 %85 %74
               OpSelectionMerge %88 None
               OpBranchConditional %86 %87 %88
         %87 = OpLabel
         %90 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %91 = OpLoad %v4float %90
         %92 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %93 = OpLoad %v4float %92
         %89 = OpExtInst %v4float %1 Reflect %91 %93
         %94 = OpFOrdEqual %v4bool %89 %46
         %96 = OpAll %bool %94
               OpBranch %88
         %88 = OpLabel
         %97 = OpPhi %bool %false %75 %96 %87
               OpSelectionMerge %99 None
               OpBranchConditional %97 %98 %99
         %98 = OpLabel
               OpBranch %99
         %99 = OpLabel
        %101 = OpPhi %bool %false %88 %true %98
               OpSelectionMerge %103 None
               OpBranchConditional %101 %102 %103
        %102 = OpLabel
               OpBranch %103
        %103 = OpLabel
        %104 = OpPhi %bool %false %99 %true %102
               OpSelectionMerge %106 None
               OpBranchConditional %104 %105 %106
        %105 = OpLabel
               OpBranch %106
        %106 = OpLabel
        %107 = OpPhi %bool %false %103 %true %105
               OpSelectionMerge %109 None
               OpBranchConditional %107 %108 %109
        %108 = OpLabel
               OpBranch %109
        %109 = OpLabel
        %110 = OpPhi %bool %false %106 %true %108
               OpSelectionMerge %114 None
               OpBranchConditional %110 %112 %113
        %112 = OpLabel
        %115 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
        %117 = OpLoad %v4float %115
               OpStore %111 %117
               OpBranch %114
        %113 = OpLabel
        %118 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
        %120 = OpLoad %v4float %118
               OpStore %111 %120
               OpBranch %114
        %114 = OpLabel
        %121 = OpLoad %v4float %111
               OpReturnValue %121
               OpFunctionEnd
