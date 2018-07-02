package com.google.ar.core.examples.java.helloskar;

import android.graphics.Canvas;
import android.graphics.ColorFilter;
import android.graphics.Paint;
import android.graphics.RectF;

import com.google.skar.SkARMatrix;
import com.google.skar.SkARUtil;

import java.util.ArrayList;

public class DrawManager {
    private float[] projectionMatrix = new float[16];
    private float[] viewMatrix = new float[16];
    private float[] viewportMatrix = new float[16];
    private ColorFilter lightFilter;
    public ArrayList<float[]> modelMatrices = new ArrayList<>();

    public void updateViewportMatrix(float width, float height) {
        viewportMatrix = SkARMatrix.createViewportMatrix(width, height);
    }

    public void updateProjectionMatrix(float[] projectionMatrix) {
        this.projectionMatrix = projectionMatrix;
    }

    public void updateViewMatrix(float[] viewMatrix) {
        this.viewMatrix = viewMatrix;
    }

    public void updateLightColorFilter(float[] colorCorr) {
        lightFilter = SkARUtil.createLightCorrectionColorFilter(colorCorr);
    }

    public void drawCircle(Canvas canvas) {
        if (modelMatrices.isEmpty()) {
            return;
        }
        Paint p = new Paint();
        p.setColorFilter(lightFilter);
        p.setARGB(180, 100, 0, 0);

        canvas.save();
        canvas.setMatrix(SkARMatrix.createPerspectiveMatrix(modelMatrices.get(0),
                viewMatrix, projectionMatrix, viewportMatrix));
        canvas.drawCircle(0, 0, 0.1f, p);
        canvas.restore();
    }

    public void drawRect(Canvas canvas) {
        if (modelMatrices.isEmpty()) {
            return;
        }
        Paint p = new Paint();
        p.setColorFilter(lightFilter);
        p.setARGB(180, 0, 0, 255);
        canvas.save();
        canvas.setMatrix(SkARMatrix.createPerspectiveMatrix(modelMatrices.get(0),
                viewMatrix, projectionMatrix, viewportMatrix));
        RectF rect = new RectF(0, 0, 0.2f, 0.2f);
        canvas.drawRect(rect, p);
        canvas.restore();
    }

    public void drawText(Canvas canvas, String text) {
        if (modelMatrices.isEmpty()) {
            return;
        }
        Paint p = new Paint();
        float textSize = 100;
        p.setColorFilter(lightFilter);
        p.setARGB(255, 0, 255, 0);
        p.setTextSize(textSize);


        float[] scaleMatrix = getTextScaleMatrix(textSize);
        float[] rotateMatrix = createXYtoXZRotationMatrix();
        float[] actualModel = new float[16];
        android.opengl.Matrix.setIdentityM(actualModel, 0);

        android.opengl.Matrix.multiplyMM(actualModel, 0, scaleMatrix, 0, rotateMatrix, 0);
        android.opengl.Matrix.multiplyMM(actualModel, 0, modelMatrices.get(0), 0, actualModel, 0);

        canvas.save();
        canvas.setMatrix(SkARMatrix.createPerspectiveMatrix(actualModel,
                viewMatrix, projectionMatrix, viewportMatrix, false));
        canvas.drawText(text, 0, 0, p);
        canvas.restore();
    }

    private float[] getTextScaleMatrix(float size) {
        float scaleFactor = 1 / (size * 10);
        float[] initScale = new float[16];
        android.opengl.Matrix.setIdentityM(initScale, 0);
        android.opengl.Matrix.scaleM(initScale, 0, scaleFactor, scaleFactor, scaleFactor);
        return initScale;
    }

    private float[] createXYtoXZRotationMatrix() {
        float[] skiaRotation = new float[16];
        android.opengl.Matrix.setIdentityM(skiaRotation, 0);
        android.opengl.Matrix.rotateM(skiaRotation, 0, 90, 1, 0, 0);
        return skiaRotation;
    }
}
