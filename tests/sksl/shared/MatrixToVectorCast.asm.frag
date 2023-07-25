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
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
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
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 ColMajor
               OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
               OpDecorate %60 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %72 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
               OpDecorate %94 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %114 RelaxedPrecision
               OpDecorate %115 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %10 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %17 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %20 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %24 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_bool = OpTypePointer Function %bool
       %true = OpConstantTrue %bool
      %false = OpConstantFalse %bool
%_ptr_Uniform_mat2v2float = OpTypePointer Uniform %mat2v2float
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
    %float_1 = OpConstant %float 1
    %float_2 = OpConstant %float 2
    %float_3 = OpConstant %float 3
    %float_4 = OpConstant %float 4
         %47 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
     %v4bool = OpTypeVector %bool 4
      %v4int = OpTypeVector %int 4
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
         %82 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
        %100 = OpConstantComposite %v4bool %true %true %true %true
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %17
         %18 = OpLabel
         %21 = OpVariable %_ptr_Function_v2float Function
               OpStore %21 %20
         %23 = OpFunctionCall %v4float %main %21
               OpStore %sk_FragColor %23
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %24
         %25 = OpFunctionParameter %_ptr_Function_v2float
         %26 = OpLabel
         %ok = OpVariable %_ptr_Function_bool Function
        %104 = OpVariable %_ptr_Function_v4float Function
               OpStore %ok %true
               OpSelectionMerge %32 None
               OpBranchConditional %true %31 %32
         %31 = OpLabel
         %33 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
         %37 = OpLoad %mat2v2float %33
         %38 = OpCompositeExtract %float %37 0 0
         %39 = OpCompositeExtract %float %37 0 1
         %40 = OpCompositeExtract %float %37 1 0
         %41 = OpCompositeExtract %float %37 1 1
         %42 = OpCompositeConstruct %v4float %38 %39 %40 %41
         %48 = OpFOrdEqual %v4bool %42 %47
         %50 = OpAll %bool %48
               OpBranch %32
         %32 = OpLabel
         %51 = OpPhi %bool %false %26 %50 %31
               OpStore %ok %51
               OpSelectionMerge %53 None
               OpBranchConditional %51 %52 %53
         %52 = OpLabel
         %54 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
         %55 = OpLoad %mat2v2float %54
         %56 = OpCompositeExtract %float %55 0 0
         %57 = OpCompositeExtract %float %55 0 1
         %58 = OpCompositeExtract %float %55 1 0
         %59 = OpCompositeExtract %float %55 1 1
         %60 = OpCompositeConstruct %v4float %56 %57 %58 %59
         %61 = OpFOrdEqual %v4bool %60 %47
         %62 = OpAll %bool %61
               OpBranch %53
         %53 = OpLabel
         %63 = OpPhi %bool %false %32 %62 %52
               OpStore %ok %63
               OpSelectionMerge %65 None
               OpBranchConditional %63 %64 %65
         %64 = OpLabel
         %66 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
         %67 = OpLoad %mat2v2float %66
         %68 = OpCompositeExtract %float %67 0 0
         %69 = OpCompositeExtract %float %67 0 1
         %70 = OpCompositeExtract %float %67 1 0
         %71 = OpCompositeExtract %float %67 1 1
         %72 = OpCompositeConstruct %v4float %68 %69 %70 %71
         %73 = OpConvertFToS %int %68
         %74 = OpConvertFToS %int %69
         %75 = OpConvertFToS %int %70
         %76 = OpConvertFToS %int %71
         %78 = OpCompositeConstruct %v4int %73 %74 %75 %76
         %83 = OpIEqual %v4bool %78 %82
         %84 = OpAll %bool %83
               OpBranch %65
         %65 = OpLabel
         %85 = OpPhi %bool %false %53 %84 %64
               OpStore %ok %85
               OpSelectionMerge %87 None
               OpBranchConditional %85 %86 %87
         %86 = OpLabel
         %88 = OpAccessChain %_ptr_Uniform_mat2v2float %10 %int_2
         %89 = OpLoad %mat2v2float %88
         %90 = OpCompositeExtract %float %89 0 0
         %91 = OpCompositeExtract %float %89 0 1
         %92 = OpCompositeExtract %float %89 1 0
         %93 = OpCompositeExtract %float %89 1 1
         %94 = OpCompositeConstruct %v4float %90 %91 %92 %93
         %95 = OpFUnordNotEqual %bool %90 %float_0
         %96 = OpFUnordNotEqual %bool %91 %float_0
         %97 = OpFUnordNotEqual %bool %92 %float_0
         %98 = OpFUnordNotEqual %bool %93 %float_0
         %99 = OpCompositeConstruct %v4bool %95 %96 %97 %98
        %101 = OpLogicalEqual %v4bool %99 %100
        %102 = OpAll %bool %101
               OpBranch %87
         %87 = OpLabel
        %103 = OpPhi %bool %false %65 %102 %86
               OpStore %ok %103
               OpSelectionMerge %108 None
               OpBranchConditional %103 %106 %107
        %106 = OpLabel
        %109 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
        %112 = OpLoad %v4float %109
               OpStore %104 %112
               OpBranch %108
        %107 = OpLabel
        %113 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
        %114 = OpLoad %v4float %113
               OpStore %104 %114
               OpBranch %108
        %108 = OpLabel
        %115 = OpLoad %v4float %104
               OpReturnValue %115
               OpFunctionEnd
