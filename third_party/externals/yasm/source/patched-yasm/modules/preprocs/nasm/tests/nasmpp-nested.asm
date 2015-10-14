%MACRO WORK_1 0
   %ASSIGN %%count1 4
   %error %%count1
   %REP %%count1
   %ENDREP
%ENDMACRO   

%MACRO WORK_2 0
   %ASSIGN %%count1 4
   %error %%count1
   %REP %%count1
   %REP 4
   %ENDREP
   %ENDREP
%ENDMACRO   

%MACRO DONT_WORK_1 0
   %ASSIGN %%count1 4
   %ASSIGN %%count2 4
   %error %%count1 %%count2
   %REP %%count1
   %REP %%count2
   %ENDREP
   %ENDREP
%ENDMACRO   

WORK_1 
WORK_2
DONT_WORK_1
