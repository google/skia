
A new PrecompileContext object has been added to assist Precompilation. The old API of the form:\
    bool Precompile(Context*, ...);\
has been deprecated and replaced with the API:\
    bool Precompile(PrecompileContext*, ...)\
The new PrecompileContext object can be obtained via the Context::makePrecompileContext call.

As an example of a possible Compilation/Precompilation threading model, one could employ 4 threads:

2 for creating Recordings (\<r1\> and \<r2\>) \
1 for precompiling (\<p1\>) \
and the main thread - which owns the Context and submits Recordings. 

Start up for this scenario would look like:

  the main thread moves a PrecompileContext to <p1> and begins precompiling there\
  the main thread creates two Recorders and moves them to <r1> and <r2> to create Recordings\
  the main thread continues on - calling Context::insertRecording on the posted Recordings.

The PrecompileContext can safely outlive the Context that created it, but it will 
effectively be shut down at that point.
