Public facing feature requests
------------------------------
 - return printf output
 - permalinks for fiddles
 - ability to render against multiple targets (gpu/cpu)
 - versioning (which version of skia was this run against)
 - serve up a iframe-able page
 - magnifying glass (both client side and server side)
 - specify scale, rotate, clip (possibly animating over a range)
 - change canvas size from 300x300 to other sizes


Implementation details
----------------------
 - Add flag for inout directory to webtry.go.
 - In webtry.go add mutexes per hash, to avoid conflicts of writing the same file at the same time.
 - Don't allow #macros in user code.
 - Limit the size of the user code submitted.
 - Add font support in the c++ template.
 - Add inline links to doxygen.
 - Add monitoring and probing (nagios).
 - sanitize the file name in the output.
