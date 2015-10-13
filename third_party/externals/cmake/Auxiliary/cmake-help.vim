nmap ,hc :call OpenCmakeHelp()<CR>

function! OpenCmakeHelp()
    let s = getline( '.' )
    let i = col( '.' ) - 1
    while i > 0 && strpart( s, i, 1 ) =~ '[A-Za-z0-9_]'
        let i = i - 1
    endwhile
    while i < col('$') && strpart( s, i, 1 ) !~ '[A-Za-z0-9_]'
        let i = i + 1
    endwhile
    let start = match( s, '[A-Za-z0-9_]\+', i )
    let end = matchend( s, '[A-Za-z0-9_]\+', i )
    let ident = strpart( s, start, end - start )
    execute "vertical new"
    execute "%!cmake --help-command ".ident
    set nomodified
    set readonly
endfunction

autocmd BufRead,BufNewFile *.cmake,CMakeLists.txt,*.cmake.in nmap <F1> :call OpenCmakeHelp()<CR>
