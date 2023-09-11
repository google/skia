               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testMatrix3x3"
               OpMemberName %_UniformBuffer 1 "colorGreen"
               OpMemberName %_UniformBuffer 2 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %resizeMatrix_f22 "resizeMatrix_f22"
               OpName %main "main"
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
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
               OpDecorate %11 Binding 0
               OpDecorate %11 DescriptorSet 0
               OpDecorate %89 RelaxedPrecision
               OpDecorate %92 RelaxedPrecision
               OpDecorate %93 RelaxedPrecision
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
    %v3float = OpTypeVector %float 3
%mat3v3float = OpTypeMatrix %v3float 3
%_UniformBuffer = OpTypeStruct %mat3v3float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
         %11 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %22 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
%mat2v2float = OpTypeMatrix %v2float 2
         %27 = OpTypeFunction %mat2v2float
%_ptr_Uniform_mat3v3float = OpTypePointer Uniform %mat3v3float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
         %40 = OpTypeFunction %v4float %_ptr_Function_v2float
      %false = OpConstantFalse %bool
    %float_2 = OpConstant %float 2
    %float_4 = OpConstant %float 4
    %float_5 = OpConstant %float 5
         %48 = OpConstantComposite %v2float %float_1 %float_2
         %49 = OpConstantComposite %v2float %float_4 %float_5
         %50 = OpConstantComposite %mat2v2float %48 %49
     %v2bool = OpTypeVector %bool 2
         %66 = OpConstantComposite %v3float %float_0 %float_0 %float_1
         %68 = OpConstantComposite %v3float %float_1 %float_2 %float_0
         %69 = OpConstantComposite %v3float %float_4 %float_5 %float_0
         %70 = OpConstantComposite %mat3v3float %68 %69 %66
     %v3bool = OpTypeVector %bool 3
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
      %int_2 = OpConstant %int 2
%_entrypoint_v = OpFunction %void None %18
         %19 = OpLabel
         %23 = OpVariable %_ptr_Function_v2float Function
               OpStore %23 %22
         %25 = OpFunctionCall %v4float %main %23
               OpStore %sk_FragColor %25
               OpReturn
               OpFunctionEnd
%resizeMatrix_f22 = OpFunction %mat2v2float None %27
         %28 = OpLabel
         %29 = OpAccessChain %_ptr_Uniform_mat3v3float %11 %int_0
         %33 = OpLoad %mat3v3float %29
         %35 = OpCompositeExtract %v3float %33 0
         %36 = OpVectorShuffle %v2float %35 %35 0 1
         %37 = OpCompositeExtract %v3float %33 1
         %38 = OpVectorShuffle %v2float %37 %37 0 1
         %39 = OpCompositeConstruct %mat2v2float %36 %38
               OpReturnValue %39
               OpFunctionEnd
       %main = OpFunction %v4float None %40
         %41 = OpFunctionParameter %_ptr_Function_v2float
         %42 = OpLabel
         %81 = OpVariable %_ptr_Function_v4float Function
         %44 = OpFunctionCall %mat2v2float %resizeMatrix_f22
         %52 = OpCompositeExtract %v2float %44 0
         %53 = OpFOrdEqual %v2bool %52 %48
         %54 = OpAll %bool %53
         %55 = OpCompositeExtract %v2float %44 1
         %56 = OpFOrdEqual %v2bool %55 %49
         %57 = OpAll %bool %56
         %58 = OpLogicalAnd %bool %54 %57
               OpSelectionMerge %60 None
               OpBranchConditional %58 %59 %60
         %59 = OpLabel
         %61 = OpFunctionCall %mat2v2float %resizeMatrix_f22
         %62 = OpCompositeExtract %v2float %61 0
         %63 = OpCompositeConstruct %v3float %62 %float_0
         %64 = OpCompositeExtract %v2float %61 1
         %65 = OpCompositeConstruct %v3float %64 %float_0
         %67 = OpCompositeConstruct %mat3v3float %63 %65 %66
         %72 = OpFOrdEqual %v3bool %63 %68
         %73 = OpAll %bool %72
         %74 = OpFOrdEqual %v3bool %65 %69
         %75 = OpAll %bool %74
         %76 = OpLogicalAnd %bool %73 %75
         %77 = OpFOrdEqual %v3bool %66 %66
         %78 = OpAll %bool %77
         %79 = OpLogicalAnd %bool %76 %78
               OpBranch %60
         %60 = OpLabel
         %80 = OpPhi %bool %false %42 %79 %59
               OpSelectionMerge %85 None
               OpBranchConditional %80 %83 %84
         %83 = OpLabel
         %86 = OpAccessChain %_ptr_Uniform_v4float %11 %int_1
         %89 = OpLoad %v4float %86
               OpStore %81 %89
               OpBranch %85
         %84 = OpLabel
         %90 = OpAccessChain %_ptr_Uniform_v4float %11 %int_2
         %92 = OpLoad %v4float %90
               OpStore %81 %92
               OpBranch %85
         %85 = OpLabel
         %93 = OpLoad %v4float %81
               OpReturnValue %93
               OpFunctionEnd
