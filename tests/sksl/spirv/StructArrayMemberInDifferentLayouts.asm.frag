### Compilation failed:

error: SPIR-V validation error: Expected Constituent type to be equal to the corresponding member type of Result Type struct
  %27 = OpCompositeConstruct %S %26

               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main" %sk_FragColor
               OpExecutionMode %main OriginUpperLeft
               OpName %testPushConstants "testPushConstants"
               OpMemberName %testPushConstants 0 "pushConstantArray"
               OpName %testUniforms "testUniforms"
               OpMemberName %testUniforms 0 "uboArray"
               OpName %sk_FragColor "sk_FragColor"
               OpName %main "main"
               OpName %S "S"
               OpMemberName %S 0 "a"
               OpName %s1 "s1"
               OpName %s2 "s2"
               OpDecorate %_arr_float_int_2 ArrayStride 4
               OpMemberDecorate %testPushConstants 0 Offset 0
               OpDecorate %testPushConstants Block
               OpDecorate %_arr_float_int_2_0 ArrayStride 16
               OpMemberDecorate %testUniforms 0 Offset 0
               OpDecorate %testUniforms Block
               OpDecorate %10 Binding 0
               OpDecorate %10 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpMemberDecorate %S 0 Offset 0
               OpDecorate %50 RelaxedPrecision
      %float = OpTypeFloat 32
        %int = OpTypeInt 32 1
      %int_2 = OpConstant %int 2
%_arr_float_int_2 = OpTypeArray %float %int_2
%testPushConstants = OpTypeStruct %_arr_float_int_2
%_ptr_PushConstant_testPushConstants = OpTypePointer PushConstant %testPushConstants
          %3 = OpVariable %_ptr_PushConstant_testPushConstants PushConstant
%_arr_float_int_2_0 = OpTypeArray %float %int_2
%testUniforms = OpTypeStruct %_arr_float_int_2_0
%_ptr_Uniform_testUniforms = OpTypePointer Uniform %testUniforms
         %10 = OpVariable %_ptr_Uniform_testUniforms Uniform
    %v4float = OpTypeVector %float 4
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
       %void = OpTypeVoid
         %18 = OpTypeFunction %void
          %S = OpTypeStruct %_arr_float_int_2_0
%_ptr_Function_S = OpTypePointer Function %S
      %int_0 = OpConstant %int 0
%_ptr_PushConstant__arr_float_int_2 = OpTypePointer PushConstant %_arr_float_int_2
%_ptr_Uniform__arr_float_int_2_0 = OpTypePointer Uniform %_arr_float_int_2_0
       %bool = OpTypeBool
%_ptr_Function_v4float = OpTypePointer Function %v4float
    %float_1 = OpConstant %float 1
         %47 = OpConstantComposite %v4float %float_1 %float_1 %float_1 %float_1
    %float_0 = OpConstant %float 0
         %49 = OpConstantComposite %v4float %float_0 %float_0 %float_0 %float_0
       %main = OpFunction %void None %18
         %19 = OpLabel
         %s1 = OpVariable %_ptr_Function_S Function
         %s2 = OpVariable %_ptr_Function_S Function
         %41 = OpVariable %_ptr_Function_v4float Function
         %24 = OpAccessChain %_ptr_PushConstant__arr_float_int_2 %3 %int_0
         %26 = OpLoad %_arr_float_int_2 %24
         %27 = OpCompositeConstruct %S %26
               OpStore %s1 %27
         %29 = OpAccessChain %_ptr_Uniform__arr_float_int_2_0 %10 %int_0
         %31 = OpLoad %_arr_float_int_2_0 %29
         %32 = OpCompositeConstruct %S %31
               OpStore %s2 %32
         %33 = OpCompositeExtract %float %26 0
         %34 = OpCompositeExtract %float %31 0
         %35 = OpFOrdEqual %bool %33 %34
         %37 = OpCompositeExtract %float %26 1
         %38 = OpCompositeExtract %float %31 1
         %39 = OpFOrdEqual %bool %37 %38
         %40 = OpLogicalAnd %bool %39 %35
               OpSelectionMerge %45 None
               OpBranchConditional %40 %43 %44
         %43 = OpLabel
               OpStore %41 %47
               OpBranch %45
         %44 = OpLabel
               OpStore %41 %49
               OpBranch %45
         %45 = OpLabel
         %50 = OpLoad %v4float %41
               OpStore %sk_FragColor %50
               OpReturn
               OpFunctionEnd

1 error
