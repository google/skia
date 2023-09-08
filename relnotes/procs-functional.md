The structs `SkSerialProcs` and `SkDeserialProcs` now take in `std::function`
instead of bare function pointers. This allows clients to pass in functors
(lambdas with captured objects) if desired.