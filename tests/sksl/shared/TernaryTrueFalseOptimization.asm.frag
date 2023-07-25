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
               OpName %ok "ok"
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
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %51 RelaxedPrecision
               OpDecorate %52 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %78 RelaxedPrecision
               OpDecorate %79 RelaxedPrecision
               OpDecorate %85 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %96 RelaxedPrecision
               OpDecorate %97 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %100 RelaxedPrecision
               OpDecorate %106 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %120 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
               OpDecorate %123 RelaxedPrecision
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
         %ok = OpVariable %_ptr_Function_bool Function
        %114 = OpVariable %_ptr_Function_v4float Function
               OpStore %ok %true
               OpSelectionMerge %31 None
               OpBranchConditional %true %30 %31
         %30 = OpLabel
         %32 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %36 = OpLoad %v4float %32
         %37 = OpCompositeExtract %float %36 1
         %39 = OpFOrdEqual %bool %37 %float_1
               OpBranch %31
         %31 = OpLabel
         %40 = OpPhi %bool %false %25 %39 %30
               OpStore %ok %40
               OpSelectionMerge %42 None
               OpBranchConditional %40 %41 %42
         %41 = OpLabel
         %43 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %44 = OpLoad %v4float %43
         %45 = OpCompositeExtract %float %44 0
         %46 = OpFUnordNotEqual %bool %45 %float_1
               OpBranch %42
         %42 = OpLabel
         %47 = OpPhi %bool %false %31 %46 %41
               OpStore %ok %47
               OpSelectionMerge %49 None
               OpBranchConditional %47 %48 %49
         %48 = OpLabel
         %50 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %51 = OpLoad %v4float %50
         %52 = OpVectorShuffle %v2float %51 %51 1 0
         %53 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %55 = OpLoad %v4float %53
         %56 = OpVectorShuffle %v2float %55 %55 0 1
         %57 = OpFOrdEqual %v2bool %52 %56
         %59 = OpAll %bool %57
               OpBranch %49
         %49 = OpLabel
         %60 = OpPhi %bool %false %42 %59 %48
               OpStore %ok %60
               OpSelectionMerge %62 None
               OpBranchConditional %60 %61 %62
         %61 = OpLabel
         %63 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %64 = OpLoad %v4float %63
         %65 = OpVectorShuffle %v2float %64 %64 1 0
         %66 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %67 = OpLoad %v4float %66
         %68 = OpVectorShuffle %v2float %67 %67 0 1
         %69 = OpFOrdEqual %v2bool %65 %68
         %70 = OpAll %bool %69
               OpBranch %62
         %62 = OpLabel
         %71 = OpPhi %bool %false %49 %70 %61
               OpStore %ok %71
               OpSelectionMerge %73 None
               OpBranchConditional %71 %72 %73
         %72 = OpLabel
         %74 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %75 = OpLoad %v4float %74
         %76 = OpVectorShuffle %v2float %75 %75 1 0
         %77 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %78 = OpLoad %v4float %77
         %79 = OpVectorShuffle %v2float %78 %78 0 1
         %80 = OpFOrdEqual %v2bool %76 %79
         %81 = OpAll %bool %80
               OpSelectionMerge %83 None
               OpBranchConditional %81 %83 %82
         %82 = OpLabel
         %84 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %85 = OpLoad %v4float %84
         %86 = OpCompositeExtract %float %85 3
         %87 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %88 = OpLoad %v4float %87
         %89 = OpCompositeExtract %float %88 3
         %90 = OpFUnordNotEqual %bool %86 %89
               OpBranch %83
         %83 = OpLabel
         %91 = OpPhi %bool %true %72 %90 %82
               OpBranch %73
         %73 = OpLabel
         %92 = OpPhi %bool %false %62 %91 %83
               OpStore %ok %92
               OpSelectionMerge %94 None
               OpBranchConditional %92 %93 %94
         %93 = OpLabel
         %95 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %96 = OpLoad %v4float %95
         %97 = OpVectorShuffle %v2float %96 %96 1 0
         %98 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %99 = OpLoad %v4float %98
        %100 = OpVectorShuffle %v2float %99 %99 0 1
        %101 = OpFUnordNotEqual %v2bool %97 %100
        %102 = OpAny %bool %101
               OpSelectionMerge %104 None
               OpBranchConditional %102 %103 %104
        %103 = OpLabel
        %105 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %106 = OpLoad %v4float %105
        %107 = OpCompositeExtract %float %106 3
        %108 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %109 = OpLoad %v4float %108
        %110 = OpCompositeExtract %float %109 3
        %111 = OpFOrdEqual %bool %107 %110
               OpBranch %104
        %104 = OpLabel
        %112 = OpPhi %bool %false %93 %111 %103
               OpBranch %94
         %94 = OpLabel
        %113 = OpPhi %bool %false %73 %112 %104
               OpStore %ok %113
               OpSelectionMerge %118 None
               OpBranchConditional %113 %116 %117
        %116 = OpLabel
        %119 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %120 = OpLoad %v4float %119
               OpStore %114 %120
               OpBranch %118
        %117 = OpLabel
        %121 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %122 = OpLoad %v4float %121
               OpStore %114 %122
               OpBranch %118
        %118 = OpLabel
        %123 = OpLoad %v4float %114
               OpReturnValue %123
               OpFunctionEnd
