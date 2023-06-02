fn main(xy: vec2<f32>) -> vec4<f32> {
    var i: i32;
    {
        var a: i32 = 0;
        loop {
            if a < 10 {
                {
                    {
                        var b: i32 = 0;
                        loop {
                            if b < 10 {
                                {
                                    {
                                        var c: i32 = 0;
                                        loop {
                                            if c < 10 {
                                                {
                                                    {
                                                        var d: i32 = 0;
                                                        loop {
                                                            if d < 10 {
                                                                {
                                                                    i = i + i32(1);
                                                                }
                                                            } else {
                                                                break;
                                                            }
                                                            continuing {
                                                                d = d + i32(1);
                                                            }
                                                        }
                                                    }
                                                }
                                            } else {
                                                break;
                                            }
                                            continuing {
                                                c = c + i32(1);
                                            }
                                        }
                                    }
                                }
                            } else {
                                break;
                            }
                            continuing {
                                b = b + i32(1);
                            }
                        }
                    }
                }
            } else {
                break;
            }
            continuing {
                a = a + i32(1);
            }
        }
    }
    return vec4<f32>(0.0);
}
@fragment fn runtimeShaderMain(@location(0) _coords: vec2<f32>) -> @location(0) vec4<f32> {
    return main(_coords);
}
