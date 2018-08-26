// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef __SKY_SHELL_MAC_SKY_APPLICATION__
#define __SKY_SHELL_MAC_SKY_APPLICATION__

#import <AppKit/AppKit.h>

#include "base/mac/scoped_sending_event.h"
#include "base/message_loop/message_pump_mac.h"

// A specific subclass of NSApplication is necessary on Mac in order to
// interact correctly with the main runloop.
@interface SkyApplication : NSApplication<CrAppProtocol, CrAppControlProtocol>
@end

#endif /* defined(__SKY_SHELL_MAC_SKY_APPLICATION__) */
