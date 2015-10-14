.loc
.loc "foo"
.loc bar
.loc -5
#.loc foo=1
.loc 1
.loc 1 "foo"
.loc 1 bar
.loc 1 2
#.loc 1 bar=2
.loc 1 bar 2
.loc 1 2 "foo"
.loc 1 2 bar+1
.loc 1 2 is_stmt
.loc 1 2 is_stmt 1
#.loc 1 2 is_stmt="foo"
.loc 1 2 is_stmt "foo"
#.loc 1 2 is_stmt=bar
.loc 1 2 is_stmt bar
.loc 1 2 is_stmt (bar)
#.loc 1 2 is_stmt=2
.loc 1 2 is_stmt 2
.loc 1 2 isa
.loc 1 2 isa 1
#.loc 1 2 isa="foo"
.loc 1 2 isa "foo"
#.loc 1 2 isa=bar
.loc 1 2 isa bar
#.loc 1 2 isa=-2
.loc 1 2 isa (-2)
.loc 1 2 isa -2
#.loc 1 2 foo=1
.loc 1 2 foo 1
#.loc 1 2 bar
