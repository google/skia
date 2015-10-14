%macro testConcat 0

    %push ctx
    %assign %$x 0
    %rep 8

        movd eax, mm%$x
        %assign %$x %$x+1

    %endrep %pop

%endmacro

testConcat 
