               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %IsEqual_bh4h4 "IsEqual_bh4h4"
               OpName %main "main"
               OpName %colorBlue "colorBlue"
               OpName %colorGreen "colorGreen"
               OpName %colorRed "colorRed"
               OpName %result "result"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %27 RelaxedPrecision
               OpDecorate %28 RelaxedPrecision
               OpDecorate %colorBlue RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %colorGreen RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %colorRed RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %111 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %119 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %13 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
       %bool = OpTypeBool
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %23 = OpTypeFunction %bool %_ptr_Function_v4float %_ptr_Function_v4float
     %v4bool = OpTypeVector %bool 4
         %32 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %13
         %14 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
%IsEqual_bh4h4 = OpFunction %bool None %23
         %24 = OpFunctionParameter %_ptr_Function_v4float
         %25 = OpFunctionParameter %_ptr_Function_v4float
         %26 = OpLabel
         %27 = OpLoad %v4float %24
         %28 = OpLoad %v4float %25
         %29 = OpFOrdEqual %v4bool %27 %28
         %31 = OpAll %bool %29
               OpReturnValue %31
               OpFunctionEnd
       %main = OpFunction %v4float None %32
         %33 = OpFunctionParameter %_ptr_Function_v2float
         %34 = OpLabel
  %colorBlue = OpVariable %_ptr_Function_v4float Function
 %colorGreen = OpVariable %_ptr_Function_v4float Function
   %colorRed = OpVariable %_ptr_Function_v4float Function
     %result = OpVariable %_ptr_Function_v4float Function
         %65 = OpVariable %_ptr_Function_v4float Function
         %66 = OpVariable %_ptr_Function_v4float Function
         %68 = OpVariable %_ptr_Function_v4float Function
         %72 = OpVariable %_ptr_Function_v4float Function
         %73 = OpVariable %_ptr_Function_v4float Function
         %75 = OpVariable %_ptr_Function_v4float Function
         %81 = OpVariable %_ptr_Function_v4float Function
         %82 = OpVariable %_ptr_Function_v4float Function
         %84 = OpVariable %_ptr_Function_v4float Function
         %92 = OpVariable %_ptr_Function_v4float Function
         %93 = OpVariable %_ptr_Function_v4float Function
         %95 = OpVariable %_ptr_Function_v4float Function
        %102 = OpVariable %_ptr_Function_v4float Function
        %103 = OpVariable %_ptr_Function_v4float Function
        %105 = OpVariable %_ptr_Function_v4float Function
        %109 = OpVariable %_ptr_Function_v4float Function
        %112 = OpVariable %_ptr_Function_v4float Function
        %114 = OpVariable %_ptr_Function_v4float Function
         %36 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %40 = OpLoad %v4float %36
         %41 = OpVectorShuffle %v2float %40 %40 2 3
         %42 = OpCompositeExtract %float %41 0
         %43 = OpCompositeExtract %float %41 1
         %44 = OpCompositeConstruct %v4float %float_0 %float_0 %42 %43
               OpStore %colorBlue %44
         %46 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %47 = OpLoad %v4float %46
         %48 = OpCompositeExtract %float %47 1
         %49 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %50 = OpLoad %v4float %49
         %51 = OpCompositeExtract %float %50 3
         %52 = OpCompositeConstruct %v4float %float_0 %48 %float_0 %51
               OpStore %colorGreen %52
         %54 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %55 = OpLoad %v4float %54
         %56 = OpCompositeExtract %float %55 0
         %57 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %58 = OpLoad %v4float %57
         %59 = OpCompositeExtract %float %58 3
         %60 = OpCompositeConstruct %v4float %56 %float_0 %float_0 %59
               OpStore %colorRed %60
         %63 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %64 = OpLoad %v4float %63
               OpStore %65 %64
               OpStore %66 %44
         %67 = OpFunctionCall %bool %IsEqual_bh4h4 %65 %66
         %62 = OpLogicalNot %bool %67
               OpSelectionMerge %71 None
               OpBranchConditional %62 %69 %70
         %69 = OpLabel
               OpStore %72 %52
               OpStore %73 %60
         %74 = OpFunctionCall %bool %IsEqual_bh4h4 %72 %73
               OpSelectionMerge %78 None
               OpBranchConditional %74 %76 %77
         %76 = OpLabel
               OpStore %75 %60
               OpBranch %78
         %77 = OpLabel
               OpStore %75 %52
               OpBranch %78
         %78 = OpLabel
         %79 = OpLoad %v4float %75
               OpStore %68 %79
               OpBranch %71
         %70 = OpLabel
               OpStore %81 %60
               OpStore %82 %52
         %83 = OpFunctionCall %bool %IsEqual_bh4h4 %81 %82
         %80 = OpLogicalNot %bool %83
               OpSelectionMerge %87 None
               OpBranchConditional %80 %85 %86
         %85 = OpLabel
               OpStore %84 %44
               OpBranch %87
         %86 = OpLabel
         %88 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
         %89 = OpLoad %v4float %88
               OpStore %84 %89
               OpBranch %87
         %87 = OpLabel
         %90 = OpLoad %v4float %84
               OpStore %68 %90
               OpBranch %71
         %71 = OpLabel
         %91 = OpLoad %v4float %68
               OpStore %result %91
               OpStore %92 %60
               OpStore %93 %44
         %94 = OpFunctionCall %bool %IsEqual_bh4h4 %92 %93
               OpSelectionMerge %98 None
               OpBranchConditional %94 %96 %97
         %96 = OpLabel
         %99 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %100 = OpLoad %v4float %99
               OpStore %95 %100
               OpBranch %98
         %97 = OpLabel
               OpStore %102 %60
               OpStore %103 %52
        %104 = OpFunctionCall %bool %IsEqual_bh4h4 %102 %103
        %101 = OpLogicalNot %bool %104
               OpSelectionMerge %108 None
               OpBranchConditional %101 %106 %107
        %106 = OpLabel
               OpStore %105 %91
               OpBranch %108
        %107 = OpLabel
               OpStore %109 %60
        %110 = OpAccessChain %_ptr_Uniform_v4float %8 %int_0
        %111 = OpLoad %v4float %110
               OpStore %112 %111
        %113 = OpFunctionCall %bool %IsEqual_bh4h4 %109 %112
               OpSelectionMerge %117 None
               OpBranchConditional %113 %115 %116
        %115 = OpLabel
               OpStore %114 %44
               OpBranch %117
        %116 = OpLabel
               OpStore %114 %60
               OpBranch %117
        %117 = OpLabel
        %118 = OpLoad %v4float %114
               OpStore %105 %118
               OpBranch %108
        %108 = OpLabel
        %119 = OpLoad %v4float %105
               OpStore %95 %119
               OpBranch %98
         %98 = OpLabel
        %120 = OpLoad %v4float %95
               OpReturnValue %120
               OpFunctionEnd
