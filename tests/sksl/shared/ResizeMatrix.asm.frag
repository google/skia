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
               OpName %result "result"
               OpName %a "a"
               OpName %b "b"
               OpName %c "c"
               OpName %d "d"
               OpName %e "e"
               OpName %f "f"
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
               OpDecorate %105 RelaxedPrecision
               OpDecorate %108 RelaxedPrecision
               OpDecorate %109 RelaxedPrecision
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
%_ptr_Function_float = OpTypePointer Function %float
%mat2v2float = OpTypeMatrix %v2float 2
%_ptr_Function_mat2v2float = OpTypePointer Function %mat2v2float
    %float_1 = OpConstant %float 1
    %v3float = OpTypeVector %float 3
         %30 = OpConstantComposite %v3float %float_1 %float_0 %float_0
         %31 = OpConstantComposite %v3float %float_0 %float_1 %float_0
         %32 = OpConstantComposite %v3float %float_0 %float_0 %float_1
%mat3v3float = OpTypeMatrix %v3float 3
         %34 = OpConstantComposite %mat3v3float %30 %31 %32
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
         %45 = OpConstantComposite %v4float %float_1 %float_0 %float_0 %float_0
         %46 = OpConstantComposite %v4float %float_0 %float_1 %float_0 %float_0
         %47 = OpConstantComposite %v4float %float_0 %float_0 %float_1 %float_0
         %48 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_1
%mat4v4float = OpTypeMatrix %v4float 4
         %50 = OpConstantComposite %mat4v4float %45 %46 %47 %48
%_ptr_Function_mat3v3float = OpTypePointer Function %mat3v3float
%_ptr_Function_v3float = OpTypePointer Function %v3float
         %70 = OpConstantComposite %v2float %float_1 %float_0
         %71 = OpConstantComposite %v2float %float_0 %float_1
         %72 = OpConstantComposite %mat2v2float %70 %71
%_ptr_Function_mat4v4float = OpTypePointer Function %mat4v4float
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_6 = OpConstant %float 6
       %bool = OpTypeBool
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
      %int_1 = OpConstant %int 1
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
     %result = OpVariable %_ptr_Function_float Function
          %a = OpVariable %_ptr_Function_mat2v2float Function
          %b = OpVariable %_ptr_Function_mat2v2float Function
          %c = OpVariable %_ptr_Function_mat3v3float Function
          %d = OpVariable %_ptr_Function_mat3v3float Function
          %e = OpVariable %_ptr_Function_mat4v4float Function
          %f = OpVariable %_ptr_Function_mat2v2float Function
         %99 = OpVariable %_ptr_Function_v4float Function
               OpStore %result %float_0
         %35 = OpVectorShuffle %v2float %30 %30 0 1
         %36 = OpVectorShuffle %v2float %31 %31 0 1
         %37 = OpCompositeConstruct %mat2v2float %35 %36
               OpStore %a %37
         %40 = OpAccessChain %_ptr_Function_v2float %a %int_0
         %41 = OpLoad %v2float %40
         %42 = OpCompositeExtract %float %41 0
         %43 = OpFAdd %float %float_0 %42
               OpStore %result %43
         %51 = OpVectorShuffle %v2float %45 %45 0 1
         %52 = OpVectorShuffle %v2float %46 %46 0 1
         %53 = OpCompositeConstruct %mat2v2float %51 %52
               OpStore %b %53
         %54 = OpAccessChain %_ptr_Function_v2float %b %int_0
         %55 = OpLoad %v2float %54
         %56 = OpCompositeExtract %float %55 0
         %57 = OpFAdd %float %43 %56
               OpStore %result %57
         %60 = OpVectorShuffle %v3float %45 %45 0 1 2
         %61 = OpVectorShuffle %v3float %46 %46 0 1 2
         %62 = OpVectorShuffle %v3float %47 %47 0 1 2
         %63 = OpCompositeConstruct %mat3v3float %60 %61 %62
               OpStore %c %63
         %64 = OpAccessChain %_ptr_Function_v3float %c %int_0
         %66 = OpLoad %v3float %64
         %67 = OpCompositeExtract %float %66 0
         %68 = OpFAdd %float %57 %67
               OpStore %result %68
               OpStore %d %34
         %73 = OpAccessChain %_ptr_Function_v3float %d %int_0
         %74 = OpLoad %v3float %73
         %75 = OpCompositeExtract %float %74 0
         %76 = OpFAdd %float %68 %75
               OpStore %result %76
               OpStore %e %50
         %79 = OpAccessChain %_ptr_Function_v4float %e %int_0
         %81 = OpLoad %v4float %79
         %82 = OpCompositeExtract %float %81 0
         %83 = OpFAdd %float %76 %82
               OpStore %result %83
         %85 = OpVectorShuffle %v3float %45 %45 0 1 2
         %86 = OpVectorShuffle %v3float %46 %46 0 1 2
         %87 = OpVectorShuffle %v3float %47 %47 0 1 2
         %88 = OpCompositeConstruct %mat3v3float %85 %86 %87
         %89 = OpVectorShuffle %v2float %85 %85 0 1
         %90 = OpVectorShuffle %v2float %86 %86 0 1
         %91 = OpCompositeConstruct %mat2v2float %89 %90
               OpStore %f %91
         %92 = OpAccessChain %_ptr_Function_v2float %f %int_0
         %93 = OpLoad %v2float %92
         %94 = OpCompositeExtract %float %93 0
         %95 = OpFAdd %float %83 %94
               OpStore %result %95
         %97 = OpFOrdEqual %bool %95 %float_6
               OpSelectionMerge %102 None
               OpBranchConditional %97 %100 %101
        %100 = OpLabel
        %103 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
        %105 = OpLoad %v4float %103
               OpStore %99 %105
               OpBranch %102
        %101 = OpLabel
        %106 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
        %108 = OpLoad %v4float %106
               OpStore %99 %108
               OpBranch %102
        %102 = OpLabel
        %109 = OpLoad %v4float %99
               OpReturnValue %109
               OpFunctionEnd
