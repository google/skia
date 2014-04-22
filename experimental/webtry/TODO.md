Public facing feature requests
------------------------------
 - ability to render against multiple targets (gpu/cpu)
 - versioning (which version of skia was this run against)
 - magnifying glass (both client side and server side)
 - specify scale, rotate, clip (possibly animating over a range)
 - change canvas size from 300x300 to other sizes


Implementation details
----------------------
 - Add flag for inout directory to webtry.go.
 - In webtry.go add mutexes per hash, to avoid conflicts of writing the same file at the same time.
 - Add font support in the c++ template.
 - Add inline links to doxygen.
 - Add monitoring and probing (nagios).
