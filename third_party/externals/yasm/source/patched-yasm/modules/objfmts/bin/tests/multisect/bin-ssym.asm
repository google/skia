section foo start=0x10 vstart=0x6000
mov si, section.foo.start
mov di, section.foo.vstart
mov cx, section.foo.length
;mov dx, section.foo.vstart-section.foo.start
