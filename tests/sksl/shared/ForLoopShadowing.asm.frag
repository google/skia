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
               OpName %counter "counter"
               OpName %i "i"
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
               OpDecorate %56 RelaxedPrecision
               OpDecorate %58 RelaxedPrecision
               OpDecorate %59 RelaxedPrecision
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
        %int = OpTypeInt 32 1
%_ptr_Function_int = OpTypePointer Function %int
      %int_0 = OpConstant %int 0
     %int_10 = OpConstant %int 10
       %bool = OpTypeBool
      %int_1 = OpConstant %int 1
     %int_90 = OpConstant %int 90
%_ptr_Function_v4float = OpTypePointer Function %v4float
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
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
    %counter = OpVariable %_ptr_Function_int Function
          %i = OpVariable %_ptr_Function_int Function
         %49 = OpVariable %_ptr_Function_v4float Function
               OpStore %counter %int_0
               OpStore %i %int_0
               OpBranch %28
         %28 = OpLabel
               OpLoopMerge %32 %31 None
               OpBranch %29
         %29 = OpLabel
         %33 = OpLoad %int %i
         %35 = OpSLessThan %bool %33 %int_10
               OpBranchConditional %35 %30 %32
         %30 = OpLabel
         %37 = OpLoad %int %i
         %38 = OpIEqual %bool %37 %int_0
               OpSelectionMerge %40 None
               OpBranchConditional %38 %39 %40
         %39 = OpLabel
               OpBranch %31
         %40 = OpLabel
         %41 = OpLoad %int %counter
         %42 = OpIAdd %int %41 %int_10
               OpStore %counter %42
               OpBranch %31
         %31 = OpLabel
         %43 = OpLoad %int %i
         %45 = OpIAdd %int %43 %int_1
               OpStore %i %45
               OpBranch %28
         %32 = OpLabel
         %46 = OpLoad %int %counter
         %48 = OpIEqual %bool %46 %int_90
               OpSelectionMerge %53 None
               OpBranchConditional %48 %51 %52
         %51 = OpLabel
         %54 = OpAccessChain %_ptr_Uniform_v4float %7 %int_0
         %56 = OpLoad %v4float %54
               OpStore %49 %56
               OpBranch %53
         %52 = OpLabel
         %57 = OpAccessChain %_ptr_Uniform_v4float %7 %int_1
         %58 = OpLoad %v4float %57
               OpStore %49 %58
               OpBranch %53
         %53 = OpLabel
         %59 = OpLoad %v4float %49
               OpReturnValue %59
               OpFunctionEnd
