// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <UIKit/UIKit.h>
#import "sky/shell/shell_view.h"

@interface SkySurface : UIView

-(instancetype) initWithShellView:(sky::shell::ShellView *) shellView;

@end
