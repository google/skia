               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %sk_FragColor "sk_FragColor"
               OpName %_UniformBuffer "_UniformBuffer"
               OpMemberName %_UniformBuffer 0 "colorWhite"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpName %color "color"
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %_UniformBuffer 0 Offset 0
               OpMemberDecorate %_UniformBuffer 0 RelaxedPrecision
               OpDecorate %_UniformBuffer Block
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpDecorate %color RelaxedPrecision
               OpDecorate %30 RelaxedPrecision
               OpDecorate %31 RelaxedPrecision
               OpDecorate %41 RelaxedPrecision
               OpDecorate %42 RelaxedPrecision
               OpDecorate %56 RelaxedPrecision
               OpDecorate %57 RelaxedPrecision
               OpDecorate %65 RelaxedPrecision
               OpDecorate %66 RelaxedPrecision
               OpDecorate %68 RelaxedPrecision
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
%_UniformBuffer = OpTypeStruct %v4float
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
         %25 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
        %int = OpTypeInt 32 1
      %int_0 = OpConstant %int 0
    %float_1 = OpConstant %float 1
       %bool = OpTypeBool
%_ptr_Function_float = OpTypePointer Function %float
      %int_1 = OpConstant %int 1
    %float_2 = OpConstant %float 2
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
      %color = OpVariable %_ptr_Function_v4float Function
               OpStore %color %25
         %26 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %30 = OpLoad %v4float %26
         %31 = OpCompositeExtract %float %30 0
         %33 = OpFOrdEqual %bool %31 %float_1
               OpSelectionMerge %36 None
               OpBranchConditional %33 %35 %36
         %35 = OpLabel
         %37 = OpAccessChain %_ptr_Function_float %color %int_1
               OpStore %37 %float_1
               OpBranch %36
         %36 = OpLabel
         %40 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %41 = OpLoad %v4float %40
         %42 = OpCompositeExtract %float %41 0
         %44 = OpFOrdEqual %bool %42 %float_2
               OpSelectionMerge %47 None
               OpBranchConditional %44 %45 %46
         %45 = OpLabel
               OpBranch %47
         %46 = OpLabel
         %48 = OpAccessChain %_ptr_Function_float %color %int_3
               OpStore %48 %float_1
               OpBranch %47
         %47 = OpLabel
               OpBranch %50
         %50 = OpLabel
               OpLoopMerge %54 %53 None
               OpBranch %51
         %51 = OpLabel
         %55 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %56 = OpLoad %v4float %55
         %57 = OpCompositeExtract %float %56 0
         %58 = OpFOrdEqual %bool %57 %float_2
               OpBranchConditional %58 %52 %54
         %52 = OpLabel
               OpBranch %53
         %53 = OpLabel
               OpBranch %50
         %54 = OpLabel
               OpBranch %59
         %59 = OpLabel
               OpLoopMerge %63 %62 None
               OpBranch %60
         %60 = OpLabel
               OpBranch %61
         %61 = OpLabel
               OpBranch %62
         %62 = OpLabel
         %64 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %65 = OpLoad %v4float %64
         %66 = OpCompositeExtract %float %65 0
         %67 = OpFOrdEqual %bool %66 %float_2
               OpBranchConditional %67 %59 %63
         %63 = OpLabel
         %68 = OpLoad %v4float %color
               OpReturnValue %68
               OpFunctionEnd
