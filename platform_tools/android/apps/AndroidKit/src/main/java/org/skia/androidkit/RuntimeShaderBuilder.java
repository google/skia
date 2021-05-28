/*
 * Copyright 2021 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

package org.skia.androidkit;

import java.lang.String;
import org.skia.androidkit.Shader;

public class RuntimeShaderBuilder {
    private long mNativeInstance;

    public RuntimeShaderBuilder(String sksl) {
        mNativeInstance = nCreate(sksl);
    }

    public RuntimeShaderBuilder setUniform(String name, float val) {
        nSetUniformFloat(mNativeInstance, name, val);
        return this;
    }
    public RuntimeShaderBuilder setUniform(String name, float valX, float valY, float valZ) {
        nSetUniformFloat3(mNativeInstance, name, valX, valY, valZ);
        return this;
    }
    public RuntimeShaderBuilder setUniform(String name, Matrix mat) {
        nSetUniformMatrix(mNativeInstance, name, mat.getNativeInstance());
        return this;
    }

    public Shader makeShader() {
        return new Shader(nMakeShader(mNativeInstance));
    }

    /**
     * Releases any resources associated with this RuntimeShaderBuilder.
     */
    public void release() {
        nRelease(mNativeInstance);
        mNativeInstance = 0;
    }

    @Override
    protected void finalize() throws Throwable {
        release();
    }

    private static native long nCreate(String sksl);
    private static native void nRelease(long nativeInstance);

    private static native void nSetUniformFloat(long nativeInstance, String name, float val);
    private static native void nSetUniformFloat3(long nativeInstance, String name, float valX, float valY, float valZ);
    private static native void nSetUniformMatrix(long nativeInstance, String name, long nativeMatrix);
    private static native long nMakeShader(long nativeInstance);
}
