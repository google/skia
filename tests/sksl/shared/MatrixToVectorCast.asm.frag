               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorGreen"
               OpMemberName %_UniformBuffer 1 "colorRed"
               OpMemberName %_UniformBuffer 2 "testMatrix2x2"
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
               OpMemberDecorate %_UniformBuffer 2 Offset 32
               OpMemberDecorate %_UniformBuffer 2 ColMajor
               OpMemberDecorate %_UniformBuffer 2 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %37 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %53 RelaxedPrecision
               OpDecorate %54 RelaxedPrecision
               OpDecorate %55 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %67 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
               OpDecorate %69 RelaxedPrecision
               OpDecorate %70 RelaxedPrecision
               OpDecorate %87 RelaxedPrecision
               OpDecorate %88 RelaxedPrecision
               OpDecorate %89 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %110 RelaxedPrecision
               OpDecorate %112 RelaxedPrecision
               OpDecorate %113 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v2float = OpTypeVector %float 2
%mat2v2float = OpTypeMatrix %v2float 2
%_UniformBuffer = OpTypeStruct %v4float %v4float %mat2v2float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %14 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %17 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %21 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
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
         %45 = OpConstantComposite %v4float %float_1 %float_2 %float_3 %float_4
     %v4bool = OpTypeVector %bool 4
      %v4int = OpTypeVector %int 4
      %int_1 = OpConstant %int 1
      %int_3 = OpConstant %int 3
      %int_4 = OpConstant %int 4
         %80 = OpConstantComposite %v4int %int_1 %int_2 %int_3 %int_4
         %98 = OpConstantComposite %v4bool %true %true %true %true
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_0 = OpConstant %int 0
%_entrypoint_v = OpFunction %void None %14
         %15 = OpLabel
         %18 = OpVariable %_ptr_Function_v2float Function
               OpStore %18 %17
         %20 = OpFunctionCall %v4float %main %18
               OpStore %sk_FragColor %20
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %21
         %22 = OpFunctionParameter %_ptr_Function_v2float
         %23 = OpLabel
         %ok = OpVariable %_ptr_Function_bool Function
        %102 = OpVariable %_ptr_Function_v4float Function
               OpStore %ok %true
               OpSelectionMerge %30 None
               OpBranchConditional %true %29 %30
         %29 = OpLabel
         %31 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_2
         %35 = OpLoad %mat2v2float %31
         %36 = OpCompositeExtract %float %35 0 0
         %37 = OpCompositeExtract %float %35 0 1
         %38 = OpCompositeExtract %float %35 1 0
         %39 = OpCompositeExtract %float %35 1 1
         %40 = OpCompositeConstruct %v4float %36 %37 %38 %39
         %46 = OpFOrdEqual %v4bool %40 %45
         %48 = OpAll %bool %46
               OpBranch %30
         %30 = OpLabel
         %49 = OpPhi %bool %false %23 %48 %29
               OpStore %ok %49
               OpSelectionMerge %51 None
               OpBranchConditional %49 %50 %51
         %50 = OpLabel
         %52 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_2
         %53 = OpLoad %mat2v2float %52
         %54 = OpCompositeExtract %float %53 0 0
         %55 = OpCompositeExtract %float %53 0 1
         %56 = OpCompositeExtract %float %53 1 0
         %57 = OpCompositeExtract %float %53 1 1
         %58 = OpCompositeConstruct %v4float %54 %55 %56 %57
         %59 = OpFOrdEqual %v4bool %58 %45
         %60 = OpAll %bool %59
               OpBranch %51
         %51 = OpLabel
         %61 = OpPhi %bool %false %30 %60 %50
               OpStore %ok %61
               OpSelectionMerge %63 None
               OpBranchConditional %61 %62 %63
         %62 = OpLabel
         %64 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_2
         %65 = OpLoad %mat2v2float %64
         %66 = OpCompositeExtract %float %65 0 0
         %67 = OpCompositeExtract %float %65 0 1
         %68 = OpCompositeExtract %float %65 1 0
         %69 = OpCompositeExtract %float %65 1 1
         %70 = OpCompositeConstruct %v4float %66 %67 %68 %69
         %71 = OpConvertFToS %int %66
         %72 = OpConvertFToS %int %67
         %73 = OpConvertFToS %int %68
         %74 = OpConvertFToS %int %69
         %76 = OpCompositeConstruct %v4int %71 %72 %73 %74
         %81 = OpIEqual %v4bool %76 %80
         %82 = OpAll %bool %81
               OpBranch %63
         %63 = OpLabel
         %83 = OpPhi %bool %false %51 %82 %62
               OpStore %ok %83
               OpSelectionMerge %85 None
               OpBranchConditional %83 %84 %85
         %84 = OpLabel
         %86 = OpAccessChain %_ptr_Uniform_mat2v2float %7 %int_2
         %87 = OpLoad %mat2v2float %86
         %88 = OpCompositeExtract %float %87 0 0
         %89 = OpCompositeExtract %float %87 0 1
         %90 = OpCompositeExtract %float %87 1 0
         %91 = OpCompositeExtract %float %87 1 1
         %92 = OpCompositeConstruct %v4float %88 %89 %90 %91
         %93 = OpFUnordNotEqual %bool %88 %float_0
         %94 = OpFUnordNotEqual %bool %89 %float_0
         %95 = OpFUnordNotEqual %bool %90 %float_0
         %96 = OpFUnordNotEqual %bool %91 %float_0
         %97 = OpCompositeConstruct %v4bool %93 %94 %95 %96
         %99 = OpLogicalEqual %v4bool %97 %98
        %100 = OpAll %bool %99
               OpBranch %85
         %85 = OpLabel
        %101 = OpPhi %bool %false %63 %100 %84
               OpStore %ok %101
               OpSelectionMerge %106 None
               OpBranchConditional %101 %104 %105
        %104 = OpLabel
        %107 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %110 = OpLoad %v4float %107
               OpStore %102 %110
               OpBranch %106
        %105 = OpLabel
        %111 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %112 = OpLoad %v4float %111
               OpStore %102 %112
               OpBranch %106
        %106 = OpLabel
        %113 = OpLoad %v4float %102
               OpReturnValue %113
               OpFunctionEnd
