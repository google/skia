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
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %46 RelaxedPrecision
               OpDecorate %47 RelaxedPrecision
               OpDecorate %48 RelaxedPrecision
               OpDecorate %50 RelaxedPrecision
               OpDecorate %63 RelaxedPrecision
               OpDecorate %64 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %83 RelaxedPrecision
               OpDecorate %84 RelaxedPrecision
               OpDecorate %86 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %124 RelaxedPrecision
               OpDecorate %127 RelaxedPrecision
               OpDecorate %128 RelaxedPrecision
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
        %int = OpTypeInt 32 1
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
     %int_n1 = OpConstant %int -1
      %int_0 = OpConstant %int 0
      %int_1 = OpConstant %int 1
         %33 = OpConstantComposite %v4int %int_n1 %int_0 %int_0 %int_1
      %false = OpConstantFalse %bool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %v2int = OpTypeVector %int 2
     %v2bool = OpTypeVector %bool 2
    %v3float = OpTypeVector %float 3
      %v3int = OpTypeVector %int 3
     %v3bool = OpTypeVector %bool 3
     %v4bool = OpTypeVector %bool 4
       %true = OpConstantTrue %bool
        %103 = OpConstantComposite %v2int %int_n1 %int_0
        %110 = OpConstantComposite %v3int %int_n1 %int_0 %int_0
%_ptr_Function_v4float = OpTypePointer Function %v4float
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
   %expected = OpVariable %_ptr_Function_v4int Function
        %118 = OpVariable %_ptr_Function_v4float Function
               OpStore %expected %33
         %36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %38 = OpLoad %v4float %36
         %39 = OpCompositeExtract %float %38 0
         %40 = OpConvertFToS %int %39
         %35 = OpExtInst %int %1 SSign %40
         %41 = OpIEqual %bool %35 %int_n1
               OpSelectionMerge %43 None
               OpBranchConditional %41 %42 %43
         %42 = OpLabel
         %45 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %46 = OpLoad %v4float %45
         %47 = OpVectorShuffle %v2float %46 %46 0 1
         %48 = OpCompositeExtract %float %47 0
         %49 = OpConvertFToS %int %48
         %50 = OpCompositeExtract %float %47 1
         %51 = OpConvertFToS %int %50
         %53 = OpCompositeConstruct %v2int %49 %51
         %44 = OpExtInst %v2int %1 SSign %53
         %54 = OpVectorShuffle %v2int %33 %33 0 1
         %55 = OpIEqual %v2bool %44 %54
         %57 = OpAll %bool %55
               OpBranch %43
         %43 = OpLabel
         %58 = OpPhi %bool %false %25 %57 %42
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
         %62 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %63 = OpLoad %v4float %62
         %64 = OpVectorShuffle %v3float %63 %63 0 1 2
         %66 = OpCompositeExtract %float %64 0
         %67 = OpConvertFToS %int %66
         %68 = OpCompositeExtract %float %64 1
         %69 = OpConvertFToS %int %68
         %70 = OpCompositeExtract %float %64 2
         %71 = OpConvertFToS %int %70
         %73 = OpCompositeConstruct %v3int %67 %69 %71
         %61 = OpExtInst %v3int %1 SSign %73
         %74 = OpVectorShuffle %v3int %33 %33 0 1 2
         %75 = OpIEqual %v3bool %61 %74
         %77 = OpAll %bool %75
               OpBranch %60
         %60 = OpLabel
         %78 = OpPhi %bool %false %43 %77 %59
               OpSelectionMerge %80 None
               OpBranchConditional %78 %79 %80
         %79 = OpLabel
         %82 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %83 = OpLoad %v4float %82
         %84 = OpCompositeExtract %float %83 0
         %85 = OpConvertFToS %int %84
         %86 = OpCompositeExtract %float %83 1
         %87 = OpConvertFToS %int %86
         %88 = OpCompositeExtract %float %83 2
         %89 = OpConvertFToS %int %88
         %90 = OpCompositeExtract %float %83 3
         %91 = OpConvertFToS %int %90
         %92 = OpCompositeConstruct %v4int %85 %87 %89 %91
         %81 = OpExtInst %v4int %1 SSign %92
         %93 = OpIEqual %v4bool %81 %33
         %95 = OpAll %bool %93
               OpBranch %80
         %80 = OpLabel
         %96 = OpPhi %bool %false %60 %95 %79
               OpSelectionMerge %98 None
               OpBranchConditional %96 %97 %98
         %97 = OpLabel
               OpBranch %98
         %98 = OpLabel
        %100 = OpPhi %bool %false %80 %true %97
               OpSelectionMerge %102 None
               OpBranchConditional %100 %101 %102
        %101 = OpLabel
        %104 = OpVectorShuffle %v2int %33 %33 0 1
        %105 = OpIEqual %v2bool %103 %104
        %106 = OpAll %bool %105
               OpBranch %102
        %102 = OpLabel
        %107 = OpPhi %bool %false %98 %106 %101
               OpSelectionMerge %109 None
               OpBranchConditional %107 %108 %109
        %108 = OpLabel
        %111 = OpVectorShuffle %v3int %33 %33 0 1 2
        %112 = OpIEqual %v3bool %110 %111
        %113 = OpAll %bool %112
               OpBranch %109
        %109 = OpLabel
        %114 = OpPhi %bool %false %102 %113 %108
               OpSelectionMerge %116 None
               OpBranchConditional %114 %115 %116
        %115 = OpLabel
               OpBranch %116
        %116 = OpLabel
        %117 = OpPhi %bool %false %109 %true %115
               OpSelectionMerge %122 None
               OpBranchConditional %117 %120 %121
        %120 = OpLabel
        %123 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %124 = OpLoad %v4float %123
               OpStore %118 %124
               OpBranch %122
        %121 = OpLabel
        %125 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
        %127 = OpLoad %v4float %125
               OpStore %118 %127
               OpBranch %122
        %122 = OpLabel
        %128 = OpLoad %v4float %118
               OpReturnValue %128
               OpFunctionEnd
