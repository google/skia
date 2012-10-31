/*
 * Copyright 2012 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "SkCondVar.h"

SkCondVar::SkCondVar() {
    pthread_mutex_init(&fMutex, NULL /* default mutex attr */);
    pthread_cond_init(&fCond, NULL /* default cond attr */);
}

SkCondVar::~SkCondVar() {
    pthread_mutex_destroy(&fMutex);
    pthread_cond_destroy(&fCond);
}

void SkCondVar::lock() {
    pthread_mutex_lock(&fMutex);
}

void SkCondVar::unlock() {
    pthread_mutex_unlock(&fMutex);
}

void SkCondVar::wait() {
    pthread_cond_wait(&fCond, &fMutex);
}

void SkCondVar::signal() {
    pthread_cond_signal(&fCond);
}

void SkCondVar::broadcast() {
    pthread_cond_broadcast(&fCond);
}
