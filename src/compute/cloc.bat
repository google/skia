SET DIRS=color common hs kx sk skc svg svg2skc

cloc-1.64 --exclude-ext=pre.cl -counted=cloc_counted.txt -ignored=cloc_ignored.txt %DIRS%
