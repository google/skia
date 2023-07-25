               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_Clockwise "sk_Clockwise"
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
               OpDecorate %_0_v RelaxedPrecision
               OpDecorate %32 RelaxedPrecision
               OpDecorate %38 RelaxedPrecision
               OpDecorate %39 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %43 RelaxedPrecision
               OpDecorate %45 RelaxedPrecision
               OpDecorate %_2_x RelaxedPrecision
               OpDecorate %_3_y RelaxedPrecision
               OpDecorate %_4_z RelaxedPrecision
               OpDecorate %_5_w RelaxedPrecision
               OpDecorate %61 RelaxedPrecision
               OpDecorate %73 RelaxedPrecision
               OpDecorate %76 RelaxedPrecision
               OpDecorate %77 RelaxedPrecision
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
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
      %v4int = OpTypeVector %int 4
%_ptr_Function_v4int = OpTypePointer Function %v4int
      %int_1 = OpConstant %int 1
%_ptr_Function_float = OpTypePointer Function %float
%float_n1_25 = OpConstant %float -1.25
         %63 = OpConstantComposite %v4float %float_n1_25 %float_n1_25 %float_n1_25 %float_0
     %v4bool = OpTypeVector %bool 4
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
       %_0_v = OpVariable %_ptr_Function_v4float Function
       %_1_i = OpVariable %_ptr_Function_v4int Function
       %_2_x = OpVariable %_ptr_Function_float Function
       %_3_y = OpVariable %_ptr_Function_float Function
       %_4_z = OpVariable %_ptr_Function_float Function
       %_5_w = OpVariable %_ptr_Function_float Function
         %67 = OpVariable %_ptr_Function_v4float Function
         %28 = OpAccessChain %_ptr_Uniform_v4float %10 %int_0
         %32 = OpLoad %v4float %28
               OpStore %_0_v %32
         %36 = OpAccessChain %_ptr_Uniform_v4float %10 %int_1
         %38 = OpLoad %v4float %36
         %39 = OpCompositeExtract %float %38 0
         %40 = OpConvertFToS %int %39
         %41 = OpCompositeExtract %float %38 1
         %42 = OpConvertFToS %int %41
         %43 = OpCompositeExtract %float %38 2
         %44 = OpConvertFToS %int %43
         %45 = OpCompositeExtract %float %38 3
         %46 = OpConvertFToS %int %45
         %47 = OpCompositeConstruct %v4int %40 %42 %44 %46
               OpStore %_1_i %47
         %50 = OpCompositeExtract %int %47 0
         %51 = OpVectorExtractDynamic %float %32 %50
               OpStore %_2_x %51
         %53 = OpCompositeExtract %int %47 1
         %54 = OpVectorExtractDynamic %float %32 %53
               OpStore %_3_y %54
         %56 = OpCompositeExtract %int %47 2
         %57 = OpVectorExtractDynamic %float %32 %56
               OpStore %_4_z %57
         %59 = OpCompositeExtract %int %47 3
         %60 = OpVectorExtractDynamic %float %32 %59
               OpStore %_5_w %60
         %61 = OpCompositeConstruct %v4float %51 %54 %57 %60
         %64 = OpFOrdEqual %v4bool %61 %63
         %66 = OpAll %bool %64
               OpSelectionMerge %70 None
               OpBranchConditional %66 %68 %69
         %68 = OpLabel
         %71 = OpAccessChain %_ptr_Uniform_v4float %10 %int_2
         %73 = OpLoad %v4float %71
               OpStore %67 %73
               OpBranch %70
         %69 = OpLabel
         %74 = OpAccessChain %_ptr_Uniform_v4float %10 %int_3
         %76 = OpLoad %v4float %74
               OpStore %67 %76
               OpBranch %70
         %70 = OpLabel
         %77 = OpLoad %v4float %67
               OpReturnValue %77
               OpFunctionEnd
