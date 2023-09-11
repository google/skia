               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "inputVal"
               OpMemberName %_UniformBuffer 1 "expected"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
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
               OpDecorate %27 RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %33 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %44 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %81 RelaxedPrecision
               OpDecorate %82 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %98 RelaxedPrecision
               OpDecorate %99 RelaxedPrecision
               OpDecorate %107 RelaxedPrecision
               OpDecorate %118 RelaxedPrecision
               OpDecorate %121 RelaxedPrecision
               OpDecorate %122 RelaxedPrecision
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
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
    %float_1 = OpConstant %float 1
         %87 = OpConstantComposite %v2float %float_1 %float_1
         %96 = OpConstantComposite %v3float %float_1 %float_1 %float_1
        %105 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
        %111 = OpVariable %_ptr_Function_v4float Function
         %28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %32 = OpLoad %v4float %28
         %33 = OpCompositeExtract %float %32 0
         %27 = OpExtInst %float %1 Cosh %33
         %34 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %36 = OpLoad %v4float %34
         %37 = OpCompositeExtract %float %36 0
         %38 = OpFOrdEqual %bool %27 %37
               OpSelectionMerge %40 None
               OpBranchConditional %38 %39 %40
         %39 = OpLabel
         %42 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %43 = OpLoad %v4float %42
         %44 = OpVectorShuffle %v2float %43 %43 0 1
         %41 = OpExtInst %v2float %1 Cosh %44
         %45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %46 = OpLoad %v4float %45
         %47 = OpVectorShuffle %v2float %46 %46 0 1
         %48 = OpFOrdEqual %v2bool %41 %47
         %50 = OpAll %bool %48
               OpBranch %40
         %40 = OpLabel
         %51 = OpPhi %bool %false %25 %50 %39
               OpSelectionMerge %53 None
               OpBranchConditional %51 %52 %53
         %52 = OpLabel
         %55 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %56 = OpLoad %v4float %55
         %57 = OpVectorShuffle %v3float %56 %56 0 1 2
         %54 = OpExtInst %v3float %1 Cosh %57
         %59 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %60 = OpLoad %v4float %59
         %61 = OpVectorShuffle %v3float %60 %60 0 1 2
         %62 = OpFOrdEqual %v3bool %54 %61
         %64 = OpAll %bool %62
               OpBranch %53
         %53 = OpLabel
         %65 = OpPhi %bool %false %40 %64 %52
               OpSelectionMerge %67 None
               OpBranchConditional %65 %66 %67
         %66 = OpLabel
         %69 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %70 = OpLoad %v4float %69
         %68 = OpExtInst %v4float %1 Cosh %70
         %71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %72 = OpLoad %v4float %71
         %73 = OpFOrdEqual %v4bool %68 %72
         %75 = OpAll %bool %73
               OpBranch %67
         %67 = OpLabel
         %76 = OpPhi %bool %false %53 %75 %66
               OpSelectionMerge %78 None
               OpBranchConditional %76 %77 %78
         %77 = OpLabel
         %80 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %81 = OpLoad %v4float %80
         %82 = OpCompositeExtract %float %81 0
         %83 = OpFOrdEqual %bool %float_1 %82
               OpBranch %78
         %78 = OpLabel
         %84 = OpPhi %bool %false %67 %83 %77
               OpSelectionMerge %86 None
               OpBranchConditional %84 %85 %86
         %85 = OpLabel
         %88 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %89 = OpLoad %v4float %88
         %90 = OpVectorShuffle %v2float %89 %89 0 1
         %91 = OpFOrdEqual %v2bool %87 %90
         %92 = OpAll %bool %91
               OpBranch %86
         %86 = OpLabel
         %93 = OpPhi %bool %false %78 %92 %85
               OpSelectionMerge %95 None
               OpBranchConditional %93 %94 %95
         %94 = OpLabel
         %97 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %98 = OpLoad %v4float %97
         %99 = OpVectorShuffle %v3float %98 %98 0 1 2
        %100 = OpFOrdEqual %v3bool %96 %99
        %101 = OpAll %bool %100
               OpBranch %95
         %95 = OpLabel
        %102 = OpPhi %bool %false %86 %101 %94
               OpSelectionMerge %104 None
               OpBranchConditional %102 %103 %104
        %103 = OpLabel
        %106 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %107 = OpLoad %v4float %106
        %108 = OpFOrdEqual %v4bool %105 %107
        %109 = OpAll %bool %108
               OpBranch %104
        %104 = OpLabel
        %110 = OpPhi %bool %false %95 %109 %103
               OpSelectionMerge %115 None
               OpBranchConditional %110 %113 %114
        %113 = OpLabel
        %116 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %118 = OpLoad %v4float %116
               OpStore %111 %118
               OpBranch %115
        %114 = OpLabel
        %119 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
        %121 = OpLoad %v4float %119
               OpStore %111 %121
               OpBranch %115
        %115 = OpLabel
        %122 = OpLoad %v4float %111
               OpReturnValue %122
               OpFunctionEnd
