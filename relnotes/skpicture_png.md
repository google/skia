`SkPicture`s no longer serialize `SkImage`s to PNG encoded data by default. Clients who wish to
preserve this should make use of `SkSerialProcs`, specifically the `fImageProc` field.