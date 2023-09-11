use cxx;
#[cxx::bridge(namespace = "hype_train")]
mod ffi {

    pub struct HypeOutput {
        output: String,
        new_len: usize,
    }

    extern "Rust" {
        fn hypeify(input: String, num_exclamations: i32) -> HypeOutput;
    }

}

use crate::ffi::HypeOutput;

pub fn hypeify(input: String, num_exclamations: i32) -> HypeOutput {
    let mut res = input.to_uppercase();
    for _ in 0..num_exclamations  {
        res += "!"
    }
    return HypeOutput{
        new_len: res.len(), output: res,
    }

}