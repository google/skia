[bits 32]

[global hashlookup:function]
[global hashtable2:data]
[global hashtable:data (hashtable.end-hashtable)]
[common dwordarray 128:4]

[section .text]
hashlookup

[section .data]
hashtable2
	db 5
hashtable
	db 1,2,3
.end
