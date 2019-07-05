/*
 * Copyright 2013 Google Inc.
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

#include "include/private/SkOnce.h"
#include "src/core/SkTLS.h"

#include <switch.h>

static s32 gSkTLSSlot = -1;

void* SkTLS::PlatformGetSpecific(bool forceCreateTheSlot) {

    if(forceCreateTheSlot) {
        gSkTLSSlot = threadTlsAlloc(SkTLS::Destructor);
    }

    if(gSkTLSSlot < 0)
    {
        return nullptr;
    }

    return threadTlsGet(gSkTLSSlot);
}

void SkTLS::PlatformSetSpecific(void* ptr) {
    threadTlsSet(gSkTLSSlot, ptr);
}
