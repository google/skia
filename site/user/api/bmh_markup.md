

# <a name="Bookmaker_Markup"></a> Bookmaker Markup
###$  
<a href="bmh_undocumented?cl=9919#Text">Text</a>, except for the single markup character, requires no annotation.
# comments are preceded by a hash symbol and whitespace
# comments may terminated by linefeed or double hash ## <- end of comment
Keywords are preceded by a single hash symbol without whitespace.
#Keyword
Keywords are terminated by double hash and may be labeled
##            <- end of #Keyword
# ##   <- alternate labeled end of #Keyword
Tables use single hash symbols to delimit columns, and double to end row.
#Table
#Legend
# first column in table # next column in table ##
##            <- end of #Legend
# a row                 # another row ##
# another row           # another row ##
# ##     <- or, just ##

| first column in table | next column in table |
| --- | ---  |
| a row | another row |
| another row | another row |

The markup character is initially # at the start of any .bmh file
###x          <- redefine the markup character as 'x'
xxx#          <- restore the default markup character
  anchor, ala HTML
  anchors may start anywhere in the line
# text #_reference ##
  class description
#$$$#   
