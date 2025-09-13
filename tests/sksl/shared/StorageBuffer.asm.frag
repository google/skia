               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %_entrypoint_v "_entrypoint" %sk_FragColor %bufferIndex
               OpExecutionMode %_entrypoint_v OriginUpperLeft

               ; Debug Information
               OpName %SomeData "SomeData"          ; id %12
               OpMemberName %SomeData 0 "a"
               OpMemberName %SomeData 1 "b"
               OpName %storageBuffer "storageBuffer"    ; id %14
               OpMemberName %storageBuffer 0 "offset"
               OpMemberName %storageBuffer 1 "inputData"
               OpName %outputBuffer "outputBuffer"  ; id %17
               OpMemberName %outputBuffer 0 "outputData"
               OpName %sk_FragColor "sk_FragColor"  ; id %19
               OpName %bufferIndex "bufferIndex"    ; id %21
               OpName %_entrypoint_v "_entrypoint_v"    ; id %22
               OpName %main "main"                      ; id %6

               ; Annotations
               OpDecorate %main RelaxedPrecision
               OpMemberDecorate %SomeData 0 Offset 0
               OpMemberDecorate %SomeData 1 Offset 16
               OpDecorate %_runtimearr_SomeData ArrayStride 32
               OpMemberDecorate %storageBuffer 0 Offset 0
               OpMemberDecorate %storageBuffer 1 Offset 16
               OpMemberDecorate %storageBuffer 1 RelaxedPrecision
               OpDecorate %storageBuffer BufferBlock
               OpDecorate %7 Binding 0
               OpDecorate %7 DescriptorSet 0
               OpMemberDecorate %outputBuffer 0 Offset 0
               OpMemberDecorate %outputBuffer 0 RelaxedPrecision
               OpDecorate %outputBuffer BufferBlock
               OpDecorate %16 Binding 1
               OpDecorate %16 DescriptorSet 0
               OpDecorate %sk_FragColor RelaxedPrecision
               OpDecorate %sk_FragColor Location 0
               OpDecorate %sk_FragColor Index 0
               OpDecorate %bufferIndex Location 2
               OpDecorate %bufferIndex Flat
               OpDecorate %41 RelaxedPrecision

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
       %uint = OpTypeInt 32 0
      %float = OpTypeFloat 32
    %v4float = OpTypeVector %float 4
    %v2float = OpTypeVector %float 2
   %SomeData = OpTypeStruct %v4float %v2float
%_runtimearr_SomeData = OpTypeRuntimeArray %SomeData    ; ArrayStride 32
%storageBuffer = OpTypeStruct %uint %_runtimearr_SomeData   ; BufferBlock
%_ptr_Uniform_storageBuffer = OpTypePointer Uniform %storageBuffer
          %7 = OpVariable %_ptr_Uniform_storageBuffer Uniform   ; Binding 0, DescriptorSet 0
%outputBuffer = OpTypeStruct %_runtimearr_SomeData              ; BufferBlock
%_ptr_Uniform_outputBuffer = OpTypePointer Uniform %outputBuffer
         %16 = OpVariable %_ptr_Uniform_outputBuffer Uniform    ; Binding 1, DescriptorSet 0
%_ptr_Output_v4float = OpTypePointer Output %v4float
%sk_FragColor = OpVariable %_ptr_Output_v4float Output  ; RelaxedPrecision, Location 0, Index 0
%bufferIndex = OpVariable %_ptr_Input_int Input         ; Location 2, Flat
       %void = OpTypeVoid
         %24 = OpTypeFunction %void
    %float_0 = OpConstant %float 0
         %27 = OpConstantComposite %v2float %float_0 %float_0
%_ptr_Function_v2float = OpTypePointer Function %v2float
         %31 = OpTypeFunction %v4float %_ptr_Function_v2float
      %int_1 = OpConstant %int 1
      %int_0 = OpConstant %int 0
%_ptr_Uniform_uint = OpTypePointer Uniform %uint
%_ptr_Uniform_SomeData = OpTypePointer Uniform %SomeData
%_ptr_Uniform_v4float = OpTypePointer Uniform %v4float
%_ptr_Uniform_v2float = OpTypePointer Uniform %v2float


               ; Function _entrypoint_v
%_entrypoint_v = OpFunction %void None %24

         %25 = OpLabel
         %28 =   OpVariable %_ptr_Function_v2float Function
                 OpStore %28 %27
         %30 =   OpFunctionCall %v4float %main %28
                 OpStore %sk_FragColor %30
                 OpReturn
               OpFunctionEnd


               ; Function main
       %main = OpFunction %v4float None %31         ; RelaxedPrecision
         %32 = OpFunctionParameter %_ptr_Function_v2float

         %33 = OpLabel
         %36 =   OpAccessChain %_ptr_Uniform_uint %7 %int_0
         %38 =   OpLoad %uint %36
         %39 =   OpAccessChain %_ptr_Uniform_SomeData %7 %int_1 %38
         %41 =   OpLoad %SomeData %39               ; RelaxedPrecision
         %42 =   OpAccessChain %_ptr_Uniform_uint %7 %int_0
         %43 =   OpLoad %uint %42
         %44 =   OpAccessChain %_ptr_Uniform_SomeData %16 %int_0 %43
                 OpStore %44 %41
         %45 =   OpLoad %int %bufferIndex
         %46 =   OpAccessChain %_ptr_Uniform_v4float %7 %int_1 %45 %int_0
         %48 =   OpLoad %v4float %46
         %49 =   OpLoad %int %bufferIndex
         %50 =   OpAccessChain %_ptr_Uniform_v2float %7 %int_1 %49 %int_1
         %52 =   OpLoad %v2float %50
         %53 =   OpCompositeExtract %float %52 0
         %54 =   OpVectorTimesScalar %v4float %48 %53
                 OpReturnValue %54
               OpFunctionEnd
