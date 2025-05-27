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
               OpName %ok "ok"
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
               OpDecorate %34 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %49 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %62 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %95 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %104 RelaxedPrecision
               OpDecorate %105 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
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
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
      %int_1 = OpConstant %int 1
     %v2bool = OpTypeVector %bool 2
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
         %ok = OpVariable %_ptr_Function_bool Function
        %112 = OpVariable %_ptr_Function_v4float Function
               OpStore %ok %true
               OpSelectionMerge %29 None
               OpBranchConditional %true %28 %29
         %28 = OpLabel
         %30 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %34 = OpLoad %v4float %30
         %35 = OpCompositeExtract %float %34 1
         %37 = OpFOrdEqual %bool %35 %float_1
               OpBranch %29
         %29 = OpLabel
         %38 = OpPhi %bool %false %22 %37 %28
               OpStore %ok %38
               OpSelectionMerge %40 None
               OpBranchConditional %38 %39 %40
         %39 = OpLabel
         %41 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %42 = OpLoad %v4float %41
         %43 = OpCompositeExtract %float %42 0
         %44 = OpFUnordNotEqual %bool %43 %float_1
               OpBranch %40
         %40 = OpLabel
         %45 = OpPhi %bool %false %29 %44 %39
               OpStore %ok %45
               OpSelectionMerge %47 None
               OpBranchConditional %45 %46 %47
         %46 = OpLabel
         %48 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %49 = OpLoad %v4float %48
         %50 = OpVectorShuffle %v2float %49 %49 1 0
         %51 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %53 = OpLoad %v4float %51
         %54 = OpVectorShuffle %v2float %53 %53 0 1
         %55 = OpFOrdEqual %v2bool %50 %54
         %57 = OpAll %bool %55
               OpBranch %47
         %47 = OpLabel
         %58 = OpPhi %bool %false %40 %57 %46
               OpStore %ok %58
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
         %61 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %62 = OpLoad %v4float %61
         %63 = OpVectorShuffle %v2float %62 %62 1 0
         %64 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %65 = OpLoad %v4float %64
         %66 = OpVectorShuffle %v2float %65 %65 0 1
         %67 = OpFOrdEqual %v2bool %63 %66
         %68 = OpAll %bool %67
               OpBranch %60
         %60 = OpLabel
         %69 = OpPhi %bool %false %47 %68 %59
               OpStore %ok %69
               OpSelectionMerge %71 None
               OpBranchConditional %69 %70 %71
         %70 = OpLabel
         %72 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %73 = OpLoad %v4float %72
         %74 = OpVectorShuffle %v2float %73 %73 1 0
         %75 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %76 = OpLoad %v4float %75
         %77 = OpVectorShuffle %v2float %76 %76 0 1
         %78 = OpFOrdEqual %v2bool %74 %77
         %79 = OpAll %bool %78
               OpSelectionMerge %81 None
               OpBranchConditional %79 %81 %80
         %80 = OpLabel
         %82 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %83 = OpLoad %v4float %82
         %84 = OpCompositeExtract %float %83 3
         %85 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %86 = OpLoad %v4float %85
         %87 = OpCompositeExtract %float %86 3
         %88 = OpFUnordNotEqual %bool %84 %87
               OpBranch %81
         %81 = OpLabel
         %89 = OpPhi %bool %true %70 %88 %80
               OpBranch %71
         %71 = OpLabel
         %90 = OpPhi %bool %false %60 %89 %81
               OpStore %ok %90
               OpSelectionMerge %92 None
               OpBranchConditional %90 %91 %92
         %91 = OpLabel
         %93 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %94 = OpLoad %v4float %93
         %95 = OpVectorShuffle %v2float %94 %94 1 0
         %96 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %97 = OpLoad %v4float %96
         %98 = OpVectorShuffle %v2float %97 %97 0 1
         %99 = OpFUnordNotEqual %v2bool %95 %98
        %100 = OpAny %bool %99
               OpSelectionMerge %102 None
               OpBranchConditional %100 %101 %102
        %101 = OpLabel
        %103 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %104 = OpLoad %v4float %103
        %105 = OpCompositeExtract %float %104 3
        %106 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %107 = OpLoad %v4float %106
        %108 = OpCompositeExtract %float %107 3
        %109 = OpFOrdEqual %bool %105 %108
               OpBranch %102
        %102 = OpLabel
        %110 = OpPhi %bool %false %91 %109 %101
               OpBranch %92
         %92 = OpLabel
        %111 = OpPhi %bool %false %71 %110 %102
               OpStore %ok %111
               OpSelectionMerge %116 None
               OpBranchConditional %111 %114 %115
        %114 = OpLabel
        %117 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %118 = OpLoad %v4float %117
               OpStore %112 %118
               OpBranch %116
        %115 = OpLabel
        %119 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %120 = OpLoad %v4float %119
               OpStore %112 %120
               OpBranch %116
        %116 = OpLabel
        %121 = OpLoad %v4float %112
               OpReturnValue %121
               OpFunctionEnd
