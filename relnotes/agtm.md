Add `skhdr::Agtm` interface.

Provide an interface to to create SMPTE ST 2094-50 (also known as Adaptive
Global Tone Mapping) metadata. This interface includes parsing, serialization,
and tone mapping (via an SkColorFilter).

Add interface to set and get serialized AGTM metadata to `skhdr::Metadata`.

