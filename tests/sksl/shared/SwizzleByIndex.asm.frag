               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "testInputs"
               OpMemberName %_UniformBuffer 1 "colorBlack"
               OpMemberName %_UniformBuffer 2 "colorGreen"
               OpMemberName %_UniformBuffer 3 "colorRed"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %_0_v "_0_v"
               OpName %_1_i "_1_i"
               OpName %_2_x "_2_x"
               OpName %_3_y "_3_y"
               OpName %_4_z "_4_z"
               OpName %_5_w "_5_w"
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
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %_0_v RelaxedPrecision
               OpDecorate %29 RelaxedPrecision
               OpDecorate %35 RelaxedPrecision
               OpDecorate %36 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %40 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %_2_x RelaxedPrecision
               OpDecorate %_3_y RelaxedPrecision
               OpDecorate %_4_z RelaxedPrecision
               OpDecorate %_5_w RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %71 RelaxedPrecision
               OpDecorate %74 RelaxedPrecision
               OpDecorate %75 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float %v4float %v4float %v4float
%_ptr_Uniform__UniformBuffer = OpTypePointer Uniform %_UniformBuffer
          %7 = OpVariable %_ptr_Uniform__UniformBuffer Uniform
       %void = OpTypeVoid
         %12 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
    %v2float = OpTypeVector %float 2
         %16 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %20 = OpTypeFunction %v4float %_ptr_Function_v2float
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%float_n1_25 = OpConstant %float -1.25
         %60 = OpConstantComposite %v4float %float_n1_25 %float_n1_25 %float_n1_25 %float_0
       %bool = OpTypeBool
     %v4bool = OpTypeVector %bool 4
      %int_2 = OpConstant %int 2
      %int_3 = OpConstant %int 3
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
       %_0_v = OpVariable %_ptr_Function_v4float Function
       %_1_i = OpVariable %_ptr_Function_v4int Function
       %_2_x = OpVariable %_ptr_Function_float Function
       %_3_y = OpVariable %_ptr_Function_float Function
       %_4_z = OpVariable %_ptr_Function_float Function
       %_5_w = OpVariable %_ptr_Function_float Function
         %65 = OpVariable %_ptr_Function_v4float Function
         %25 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %29 = OpLoad %v4float %25
               OpStore %_0_v %29
         %33 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %35 = OpLoad %v4float %33
         %36 = OpCompositeExtract %float %35 0
         %37 = OpConvertFToS %int %36
         %38 = OpCompositeExtract %float %35 1
         %39 = OpConvertFToS %int %38
         %40 = OpCompositeExtract %float %35 2
         %41 = OpConvertFToS %int %40
         %42 = OpCompositeExtract %float %35 3
         %43 = OpConvertFToS %int %42
         %44 = OpCompositeConstruct %v4int %37 %39 %41 %43
               OpStore %_1_i %44
         %47 = OpCompositeExtract %int %44 0
         %48 = OpVectorExtractDynamic %float %29 %47
               OpStore %_2_x %48
         %50 = OpCompositeExtract %int %44 1
         %51 = OpVectorExtractDynamic %float %29 %50
               OpStore %_3_y %51
         %53 = OpCompositeExtract %int %44 2
         %54 = OpVectorExtractDynamic %float %29 %53
               OpStore %_4_z %54
         %56 = OpCompositeExtract %int %44 3
         %57 = OpVectorExtractDynamic %float %29 %56
               OpStore %_5_w %57
         %58 = OpCompositeConstruct %v4float %48 %51 %54 %57
         %61 = OpFOrdEqual %v4bool %58 %60
         %64 = OpAll %bool %61
               OpSelectionMerge %68 None
               OpBranchConditional %64 %66 %67
         %66 = OpLabel
         %69 = OpAccessChain %_ptr_Uniform_v4float %7 %int_2
         %71 = OpLoad %v4float %69
               OpStore %65 %71
               OpBranch %68
         %67 = OpLabel
         %72 = OpAccessChain %_ptr_Uniform_v4float %7 %int_3
         %74 = OpLoad %v4float %72
               OpStore %65 %74
               OpBranch %68
         %68 = OpLabel
         %75 = OpLoad %v4float %65
               OpReturnValue %75
               OpFunctionEnd
