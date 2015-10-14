label:
db 0
.local:
db 0
..@local1:
.local2:
dw label.local
dw label.local2

$label2:
db 0
$.local:
db 0
$..@local2:
$.local2:
dw label2.local
dw label2.local2
dw $label2.local
dw $label2.local2


