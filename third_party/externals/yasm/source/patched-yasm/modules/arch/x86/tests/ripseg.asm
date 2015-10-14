bits 64
foo:

default abs
mov rbx, [foo]
mov rbx, [es:foo]
mov rbx, [fs:foo]
mov rbx, [gs:foo]
mov rbx, [rel es:foo]
mov rbx, [rel fs:foo]
mov rbx, [rel gs:foo]
mov rbx, [abs es:foo]
mov rbx, [abs fs:foo]
mov rbx, [abs gs:foo]
;mov rbx, [es:rel foo]
;mov rbx, [fs:rel foo]
;mov rbx, [es:abs foo]
;mov rbx, [fs:abs foo]

default rel
mov rbx, [foo]
mov rbx, [es:foo]
mov rbx, [fs:foo]
mov rbx, [gs:foo]
mov rbx, [rel es:foo]
mov rbx, [rel fs:foo]
mov rbx, [rel gs:foo]
mov rbx, [abs es:foo]
mov rbx, [abs fs:foo]
mov rbx, [abs gs:foo]
;mov rbx, [es:rel foo]
;mov rbx, [fs:rel foo]
;mov rbx, [es:abs foo]
;mov rbx, [fs:abs foo]

