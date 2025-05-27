               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testMatrix3x3"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %resizeMatrix_f22 "resizeMatrix_f22"
               OpName %main "main"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 ColMajor
               OpMemberDecorate %_UniformBuffer 0 MatrixStride 16
               OpMemberDecorate %_UniformBuffer 1 Offset 48
               OpMemberDecorate %_UniformBuffer 1 RelaxedPrecision
               OpMemberDecorate %_UniformBuffer 2 Offset 64
               OpMemberDecorate %_UniformBuffer 2 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %8 Binding 0
               OpDecorate %8 DescriptorSet 0
               OpDecorate %87 RelaxedPrecision
               OpDecorate %90 RelaxedPrecision
               OpDecorate %91 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %mat3v3float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %8 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %15 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %19 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%mat2v2float = OpTypeMatrix %v2float 2
         %24 = OpTypeFunction %mat2v2float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %37 = OpTypeFunction %v4float %_ptr_Function_v2float
       %bool = OpTypeBool
      %false = OpConstantFalse %bool
    %float_2 = OpConstant %float 2
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
         %46 = OpConstantComposite %v2float %float_1 %float_2
         %47 = OpConstantComposite %v2float %float_4 %float_5
         %48 = OpConstantComposite %mat2v2float %46 %47
     %v2bool = OpTypeVector %bool 2
         %64 = OpConstantComposite %v3float %float_0 %float_0 %float_1
         %66 = OpConstantComposite %v3float %float_1 %float_2 %float_0
         %67 = OpConstantComposite %v3float %float_4 %float_5 %float_0
         %68 = OpConstantComposite %mat3v3float %66 %67 %64
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %15
         %16 = OpLabel
         %20 = OpVariable %_ptr_Function_v2float Function
               OpStore %20 %19
         %22 = OpFunctionCall %v4float %main %20
               OpStore %sk_FragColor %22
               OpReturn
               OpFunctionEnd
%resizeMatrix_f22 = OpFunction %mat2v2float None %24
         %25 = OpLabel
         %26 = OpAccessChain %_ptr_Uniform_mat3v3float %8 %int_0
         %30 = OpLoad %mat3v3float %26
         %32 = OpCompositeExtract %v3float %30 0
         %33 = OpVectorShuffle %v2float %32 %32 0 1
         %34 = OpCompositeExtract %v3float %30 1
         %35 = OpVectorShuffle %v2float %34 %34 0 1
         %36 = OpCompositeConstruct %mat2v2float %33 %35
               OpReturnValue %36
               OpFunctionEnd
       %main = OpFunction %v4float None %37
         %38 = OpFunctionParameter %_ptr_Function_v2float
         %39 = OpLabel
         %79 = OpVariable %_ptr_Function_v4float Function
         %42 = OpFunctionCall %mat2v2float %resizeMatrix_f22
         %50 = OpCompositeExtract %v2float %42 0
         %51 = OpFOrdEqual %v2bool %50 %46
         %52 = OpAll %bool %51
         %53 = OpCompositeExtract %v2float %42 1
         %54 = OpFOrdEqual %v2bool %53 %47
         %55 = OpAll %bool %54
         %56 = OpLogicalAnd %bool %52 %55
               OpSelectionMerge %58 None
               OpBranchConditional %56 %57 %58
         %57 = OpLabel
         %59 = OpFunctionCall %mat2v2float %resizeMatrix_f22
         %60 = OpCompositeExtract %v2float %59 0
         %61 = OpCompositeConstruct %v3float %60 %float_0
         %62 = OpCompositeExtract %v2float %59 1
         %63 = OpCompositeConstruct %v3float %62 %float_0
         %65 = OpCompositeConstruct %mat3v3float %61 %63 %64
         %70 = OpFOrdEqual %v3bool %61 %66
         %71 = OpAll %bool %70
         %72 = OpFOrdEqual %v3bool %63 %67
         %73 = OpAll %bool %72
         %74 = OpLogicalAnd %bool %71 %73
         %75 = OpFOrdEqual %v3bool %64 %64
         %76 = OpAll %bool %75
         %77 = OpLogicalAnd %bool %74 %76
               OpBranch %58
         %58 = OpLabel
         %78 = OpPhi %bool %false %39 %77 %57
               OpSelectionMerge %83 None
               OpBranchConditional %78 %81 %82
         %81 = OpLabel
         %84 = OpAccessChain %_ptr_Uniform_v4float %8 %int_1
         %87 = OpLoad %v4float %84
               OpStore %79 %87
               OpBranch %83
         %82 = OpLabel
         %88 = OpAccessChain %_ptr_Uniform_v4float %8 %int_2
         %90 = OpLoad %v4float %88
               OpStore %79 %90
               OpBranch %83
         %83 = OpLabel
         %91 = OpLoad %v4float %79
               OpReturnValue %91
               OpFunctionEnd
