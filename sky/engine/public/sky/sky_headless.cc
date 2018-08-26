// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/public/sky/sky_headless.h"

#include "sky/engine/core/script/dart_controller.h"
#include "sky/engine/core/script/dom_dart_state.h"
#include "sky/engine/core/window/window.h"
#include "sky/engine/wtf/MakeUnique.h"

namespace blink {

SkyHeadless::SkyHeadless(Client* client) : client_(client) {
}

SkyHeadless::~SkyHeadless() {
}

void SkyHeadless::Init(const String& name) {
  DCHECK(!dart_controller_);

  dart_controller_ = WTF::MakeUnique<DartController>();
  dart_controller_->CreateIsolateFor(WTF::MakeUnique<DOMDartState>(
      WTF::MakeUnique<Window>(this), name));

  DOMDartState* dart_state = dart_controller_->dart_state();
  DartState::Scope scope(dart_state);
  dart_state->window()->DidCreateIsolate();
  client_->DidCreateIsolate(dart_state->isolate());
}

void SkyHeadless::RunFromSnapshotBuffer(const uint8_t* buffer, size_t size) {
  dart_controller_->RunFromSnapshotBuffer(buffer, size);
}

void SkyHeadless::ScheduleFrame() {
}

void SkyHeadless::Render(Scene* scene) {
}

} // namespace blink
