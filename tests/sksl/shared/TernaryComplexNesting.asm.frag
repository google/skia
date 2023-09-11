               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
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
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %29 RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %colorBlue RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %colorGreen RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %colorRed RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %result RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %102 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %16 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
         %25 = OpTypeFunction %bool %_ptr_Function_v4float %_ptr_Function_v4float
     %v4bool = OpTypeVector %bool 4
         %34 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %16
         %17 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
%IsEqual_bh4h4 = OpFunction %bool None %25
         %26 = OpFunctionParameter %_ptr_Function_v4float
         %27 = OpFunctionParameter %_ptr_Function_v4float
         %28 = OpLabel
         %29 = OpLoad %v4float %26
         %30 = OpLoad %v4float %27
         %31 = OpFOrdEqual %v4bool %29 %30
         %33 = OpAll %bool %31
               OpReturnValue %33
               OpFunctionEnd
       %main = OpFunction %v4float None %34
         %35 = OpFunctionParameter %_ptr_Function_v2float
         %36 = OpLabel
  %colorBlue = OpVariable %_ptr_Function_v4float Function
 %colorGreen = OpVariable %_ptr_Function_v4float Function
   %colorRed = OpVariable %_ptr_Function_v4float Function
     %result = OpVariable %_ptr_Function_v4float Function
         %67 = OpVariable %_ptr_Function_v4float Function
         %68 = OpVariable %_ptr_Function_v4float Function
         %70 = OpVariable %_ptr_Function_v4float Function
         %74 = OpVariable %_ptr_Function_v4float Function
         %75 = OpVariable %_ptr_Function_v4float Function
         %77 = OpVariable %_ptr_Function_v4float Function
         %83 = OpVariable %_ptr_Function_v4float Function
         %84 = OpVariable %_ptr_Function_v4float Function
         %86 = OpVariable %_ptr_Function_v4float Function
         %94 = OpVariable %_ptr_Function_v4float Function
         %95 = OpVariable %_ptr_Function_v4float Function
         %97 = OpVariable %_ptr_Function_v4float Function
        %104 = OpVariable %_ptr_Function_v4float Function
        %105 = OpVariable %_ptr_Function_v4float Function
        %107 = OpVariable %_ptr_Function_v4float Function
        %111 = OpVariable %_ptr_Function_v4float Function
        %114 = OpVariable %_ptr_Function_v4float Function
        %116 = OpVariable %_ptr_Function_v4float Function
         %38 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %42 = OpLoad %v4float %38
         %43 = OpVectorShuffle %v2float %42 %42 2 3
         %44 = OpCompositeExtract %float %43 0
         %45 = OpCompositeExtract %float %43 1
         %46 = OpCompositeConstruct %v4float %float_0 %float_0 %44 %45
               OpStore %colorBlue %46
         %48 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %49 = OpLoad %v4float %48
         %50 = OpCompositeExtract %float %49 1
         %51 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %52 = OpLoad %v4float %51
         %53 = OpCompositeExtract %float %52 3
         %54 = OpCompositeConstruct %v4float %float_0 %50 %float_0 %53
               OpStore %colorGreen %54
         %56 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %57 = OpLoad %v4float %56
         %58 = OpCompositeExtract %float %57 0
         %59 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %60 = OpLoad %v4float %59
         %61 = OpCompositeExtract %float %60 3
         %62 = OpCompositeConstruct %v4float %58 %float_0 %float_0 %61
               OpStore %colorRed %62
         %65 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %66 = OpLoad %v4float %65
               OpStore %67 %66
               OpStore %68 %46
         %69 = OpFunctionCall %bool %IsEqual_bh4h4 %67 %68
         %64 = OpLogicalNot %bool %69
               OpSelectionMerge %73 None
               OpBranchConditional %64 %71 %72
         %71 = OpLabel
               OpStore %74 %54
               OpStore %75 %62
         %76 = OpFunctionCall %bool %IsEqual_bh4h4 %74 %75
               OpSelectionMerge %80 None
               OpBranchConditional %76 %78 %79
         %78 = OpLabel
               OpStore %77 %62
               OpBranch %80
         %79 = OpLabel
               OpStore %77 %54
               OpBranch %80
         %80 = OpLabel
         %81 = OpLoad %v4float %77
               OpStore %70 %81
               OpBranch %73
         %72 = OpLabel
               OpStore %83 %62
               OpStore %84 %54
         %85 = OpFunctionCall %bool %IsEqual_bh4h4 %83 %84
         %82 = OpLogicalNot %bool %85
               OpSelectionMerge %89 None
               OpBranchConditional %82 %87 %88
         %87 = OpLabel
               OpStore %86 %46
               OpBranch %89
         %88 = OpLabel
         %90 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
         %91 = OpLoad %v4float %90
               OpStore %86 %91
               OpBranch %89
         %89 = OpLabel
         %92 = OpLoad %v4float %86
               OpStore %70 %92
               OpBranch %73
         %73 = OpLabel
         %93 = OpLoad %v4float %70
               OpStore %result %93
               OpStore %94 %62
               OpStore %95 %46
         %96 = OpFunctionCall %bool %IsEqual_bh4h4 %94 %95
               OpSelectionMerge %100 None
               OpBranchConditional %96 %98 %99
         %98 = OpLabel
        %101 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %102 = OpLoad %v4float %101
               OpStore %97 %102
               OpBranch %100
         %99 = OpLabel
               OpStore %104 %62
               OpStore %105 %54
        %106 = OpFunctionCall %bool %IsEqual_bh4h4 %104 %105
        %103 = OpLogicalNot %bool %106
               OpSelectionMerge %110 None
               OpBranchConditional %103 %108 %109
        %108 = OpLabel
               OpStore %107 %93
               OpBranch %110
        %109 = OpLabel
               OpStore %111 %62
        %112 = OpAccessChain %_ptr_Uniform_v4float %11 %int_0
        %113 = OpLoad %v4float %112
               OpStore %114 %113
        %115 = OpFunctionCall %bool %IsEqual_bh4h4 %111 %114
               OpSelectionMerge %119 None
               OpBranchConditional %115 %117 %118
        %117 = OpLabel
               OpStore %116 %46
               OpBranch %119
        %118 = OpLabel
               OpStore %116 %62
               OpBranch %119
        %119 = OpLabel
        %120 = OpLoad %v4float %116
               OpStore %107 %120
               OpBranch %110
        %110 = OpLabel
        %121 = OpLoad %v4float %107
               OpStore %97 %121
               OpBranch %100
        %100 = OpLabel
        %122 = OpLoad %v4float %97
               OpReturnValue %122
               OpFunctionEnd
