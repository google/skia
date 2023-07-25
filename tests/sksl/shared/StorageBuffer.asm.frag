               OpCapability Shader
          %1 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_Clockwise %sk_FragColor %bufferIndex
               OpExecutionMode %_entrypoint_v OriginUpperLeft
               OpName %SomeData "SomeData"
               OpMemberName %SomeData 0 "a"
               OpMemberName %SomeData 1 "b"
               OpName %storageBuffer "storageBuffer"
               OpMemberName %storageBuffer 0 "offset"
               OpMemberName %storageBuffer 1 "inputData"
               OpName %outputBuffer "outputBuffer"
               OpMemberName %outputBuffer 0 "outputData"
               OpName %sk_Clockwise "sk_Clockwise"
               OpName %sk_FragColor "sk_FragColor"
               OpName %bufferIndex "bufferIndex"
               OpName %_entrypoint_v "_entrypoint_v"
               OpName %main "main"
               OpMemberDecorate %SomeData 0 Offset 0
               OpMemberDecorate %SomeData 1 Offset 16
               OpDecorate %_runtimearr_SomeData ArrayStride 32
               OpMemberDecorate %storageBuffer 0 Offset 0
               OpMemberDecorate %storageBuffer 1 Offset 16
               OpMemberDecorate %storageBuffer 1 RelaxedPrecision
               OpDecorate %storageBuffer BufferBlock
               OpDecorate %3 Binding 0
               OpDecorate %3 DescriptorSet 0
               OpMemberDecorate %outputBuffer 0 Offset 0
               OpMemberDecorate %outputBuffer 0 RelaxedPrecision
               OpDecorate %outputBuffer BufferBlock
               OpDecorate %12 Binding 1
               OpDecorate %12 DescriptorSet 0
               OpDecorate %sk_Clockwise BuiltIn FrontFacing
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %bufferIndex Location 2
               OpDecorate %bufferIndex Flat
               OpDecorate %42 RelaxedPrecision
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
   %SomeData = OpTypeStruct %v4float %v2float
%_runtimearr_SomeData = OpTypeRuntimeArray %SomeData
%storageBuffer = OpTypeStruct %uint %_runtimearr_SomeData
%_ptr_Uniform_storageBuffer = OpTypePointer Uniform %storageBuffer
          %3 = OpVariable %_ptr_Uniform_storageBuffer Uniform
%outputBuffer = OpTypeStruct %_runtimearr_SomeData
%_ptr_Uniform_outputBuffer = OpTypePointer Uniform %outputBuffer
         %12 = OpVariable %_ptr_Uniform_outputBuffer Uniform
       %bool = OpTypeBool
%_ptr_Input_bool = OpTypePointer Input %bool
%sk_Clockwise = OpVariable %_ptr_Input_bool Input
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
%bufferIndex = OpVariable %_ptr_Input_int Input
       %void = OpTypeVoid
         %25 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %28 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %32 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_Uniform_SomeData = OpTypePointer Uniform %SomeData
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float
%_entrypoint_v = OpFunction %void None %25
         %26 = OpLabel
         %29 = OpVariable %_ptr_Function_v2float Function
               OpStore %29 %28
         %31 = OpFunctionCall %v4float %main %29
               OpStore %sk_FragColor %31
               OpReturn
               OpFunctionEnd
       %main = OpFunction %v4float None %32
         %33 = OpFunctionParameter %_ptr_Function_v2float
         %34 = OpLabel
         %37 = OpAccessChain %_ptr_Uniform_uint %3 %int_0
         %39 = OpLoad %uint %37
         %40 = OpAccessChain %_ptr_Uniform_SomeData %3 %int_1 %39
         %42 = OpLoad %SomeData %40
         %43 = OpAccessChain %_ptr_Uniform_uint %3 %int_0
         %44 = OpLoad %uint %43
         %45 = OpAccessChain %_ptr_Uniform_SomeData %12 %int_0 %44
               OpStore %45 %42
         %46 = OpLoad %int %bufferIndex
         %47 = OpAccessChain %_ptr_Uniform_v4float %3 %int_1 %46 %int_0
         %49 = OpLoad %v4float %47
         %50 = OpLoad %int %bufferIndex
         %51 = OpAccessChain %_ptr_Uniform_v2float %3 %int_1 %50 %int_1
         %53 = OpLoad %v2float %51
         %54 = OpCompositeExtract %float %53 0
         %55 = OpVectorTimesScalar %v4float %49 %54
               OpReturnValue %55
               OpFunctionEnd
