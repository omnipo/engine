// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of dart_ui;

/// List of predefined color transfer modes. This list comes from Skia's
/// SkXfermode.h and the values (order) should be kept in sync.
/// See [https://skia.org/user/api/skpaint#SkXfermode] for how these
/// transfer modes behave.
enum TransferMode {
  clear,
  src,
  dst,
  srcOver,
  dstOver,
  srcIn,
  dstIn,
  srcOut,
  dstOut,
  srcATop,
  dstATop,
  xor,
  plus,
  modulate,

  // Following blend modes are defined in the CSS Compositing standard.
  screen,  /// The last coeff mode.

  overlay,
  darken,
  lighten,
  colorDodge,
  colorBurn,
  hardLight,
  softLight,
  difference,
  exclusion,
  multiply,  /// The last separable mode.

  hue,
  saturation,
  color,
  luminosity,
}
