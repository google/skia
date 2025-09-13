               OpCapability Shader
          %5 = OpExtInstImport "GLSL.std.450"
               OpMemoryModel Logical GLSL450
               OpEntryPoint Fragment %main "main"
               OpExecutionMode %main OriginUpperLeft

               ; Debug Information
               OpName %StructB "StructB"            ; id %11
               OpMemberName %StructB 0 "val"
               OpName %StructA "StructA"            ; id %12
               OpMemberName %StructA 0 "structB"
               OpMemberName %StructA 1 "val"
               OpName %testStorageBuffer "testStorageBuffer"    ; id %14
               OpMemberName %testStorageBuffer 0 "testStructA"
               OpMemberName %testStorageBuffer 1 "testArr"
               OpName %testSecondStorageBuffer "testSecondStorageBuffer"    ; id %18
               OpMemberName %testSecondStorageBuffer 0 "testStructArr"
               OpName %foo_fff_testArr "foo_fff_testArr"    ; id %6
               OpName %bar_fSf_testStructArr "bar_fSf_testStructArr"    ; id %7
               OpName %main "main"                                      ; id %8

               ; Annotations
               OpMemberDecorate %StructB 0 Offset 0
               OpMemberDecorate %StructA 0 Offset 0
               OpMemberDecorate %StructA 0 RelaxedPrecision
               OpMemberDecorate %StructA 1 Offset 4
               OpDecorate %_runtimearr_float ArrayStride 4
               OpMemberDecorate %testStorageBuffer 0 Offset 0
               OpMemberDecorate %testStorageBuffer 0 RelaxedPrecision
               OpMemberDecorate %testStorageBuffer 1 Offset 8
               OpDecorate %testStorageBuffer BufferBlock
               OpDecorate %9 Binding 0
               OpDecorate %9 DescriptorSet 0
               OpDecorate %_runtimearr_StructA ArrayStride 8
               OpMemberDecorate %testSecondStorageBuffer 0 Offset 0
               OpMemberDecorate %testSecondStorageBuffer 0 RelaxedPrecision
               OpDecorate %testSecondStorageBuffer BufferBlock
               OpDecorate %16 Binding 1
               OpDecorate %16 DescriptorSet 0

               ; Types, variables and constants
        %int = OpTypeInt 32 1
%_ptr_Input_int = OpTypePointer Input %int
      %float = OpTypeFloat 32
    %StructB = OpTypeStruct %float
    %StructA = OpTypeStruct %StructB %float
%_runtimearr_float = OpTypeRuntimeArray %float      ; ArrayStride 4
%testStorageBuffer = OpTypeStruct %StructA %_runtimearr_float   ; BufferBlock
%_ptr_Uniform_testStorageBuffer = OpTypePointer Uniform %testStorageBuffer
          %9 = OpVariable %_ptr_Uniform_testStorageBuffer Uniform   ; Binding 0, DescriptorSet 0
%_runtimearr_StructA = OpTypeRuntimeArray %StructA                  ; ArrayStride 8
%testSecondStorageBuffer = OpTypeStruct %_runtimearr_StructA        ; BufferBlock
%_ptr_Uniform_testSecondStorageBuffer = OpTypePointer Uniform %testSecondStorageBuffer
         %16 = OpVariable %_ptr_Uniform_testSecondStorageBuffer Uniform     ; Binding 1, DescriptorSet 0
%_ptr_Function_float = OpTypePointer Function %float
         %21 = OpTypeFunction %float %_ptr_Function_float
      %int_1 = OpConstant %int 1
%_ptr_Uniform_float = OpTypePointer Uniform %float
      %int_0 = OpConstant %int 0
       %void = OpTypeVoid
         %38 = OpTypeFunction %void


               ; Function foo_fff_testArr
%foo_fff_testArr = OpFunction %float None %21
         %22 = OpFunctionParameter %_ptr_Function_float

         %23 = OpLabel
         %25 =   OpLoad %float %22
         %26 =   OpConvertFToS %int %25
         %27 =   OpAccessChain %_ptr_Uniform_float %9 %int_1 %26
         %29 =   OpLoad %float %27
                 OpReturnValue %29
               OpFunctionEnd


               ; Function bar_fSf_testStructArr
%bar_fSf_testStructArr = OpFunction %float None %21
         %30 = OpFunctionParameter %_ptr_Function_float

         %31 = OpLabel
         %33 =   OpLoad %float %30
         %34 =   OpConvertFToS %int %33
         %35 =   OpAccessChain %_ptr_Uniform_float %16 %int_0 %34 %int_0 %int_0
         %36 =   OpLoad %float %35
                 OpReturnValue %36
               OpFunctionEnd


               ; Function main
       %main = OpFunction %void None %38

         %39 = OpLabel
         %42 =   OpVariable %_ptr_Function_float Function
         %46 =   OpVariable %_ptr_Function_float Function
         %50 =   OpVariable %_ptr_Function_float Function
         %40 =   OpAccessChain %_ptr_Uniform_float %9 %int_0 %int_1
         %41 =   OpLoad %float %40
                 OpStore %42 %41
         %43 =   OpFunctionCall %float %foo_fff_testArr %42
         %44 =   OpAccessChain %_ptr_Uniform_float %9 %int_0 %int_0 %int_0
         %45 =   OpLoad %float %44
                 OpStore %46 %45
         %47 =   OpFunctionCall %float %foo_fff_testArr %46
         %48 =   OpAccessChain %_ptr_Uniform_float %9 %int_0 %int_0 %int_0
         %49 =   OpLoad %float %48
                 OpStore %50 %49
         %51 =   OpFunctionCall %float %bar_fSf_testStructArr %50
                 OpReturn
               OpFunctionEnd
