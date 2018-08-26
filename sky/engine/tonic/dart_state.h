// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_TONIC_DART_STATE_H_
#define SKY_ENGINE_TONIC_DART_STATE_H_

#include "base/logging.h"
#include "base/memory/weak_ptr.h"
#include "base/supports_user_data.h"
#include "dart/runtime/include/dart_api.h"
#include "sky/engine/tonic/dart_api_scope.h"
#include "sky/engine/tonic/dart_isolate_scope.h"
#include "sky/engine/tonic/dart_persistent_value.h"

namespace blink {
class DartClassLibrary;
class DartExceptionFactory;
class DartLibraryLoader;
class DartStringCache;
class DartTimerHeap;

// DartState represents the state associated with a given Dart isolate. The
// lifetime of this object is controlled by the DartVM. If you want to hold a
// reference to a DartState instance, please hold a base::WeakPtr<DartState>.
//
// DartState is analogous to gin::PerIsolateData and JSC::ExecState.
class DartState : public base::SupportsUserData {
 public:
  class Scope {
   public:
    Scope(DartState* dart_state);
    ~Scope();

   private:
    DartIsolateScope scope_;
    DartApiScope api_scope_;
  };

  DartState();
  virtual ~DartState();

  static DartState* From(Dart_Isolate isolate);
  static DartState* Current();

  base::WeakPtr<DartState> GetWeakPtr();

  Dart_Isolate isolate() { return isolate_; }
  void SetIsolate(Dart_Isolate isolate);

  DartClassLibrary& class_library() { return *class_library_; }
  DartExceptionFactory& exception_factory() { return *exception_factory_; }
  DartLibraryLoader& library_loader() { return *library_loader_; }
  DartStringCache& string_cache() { return *string_cache_; }
  DartTimerHeap& timer_heap() { return *timer_heap_; }

  Dart_Handle index_handle() { return index_handle_.value(); }

  virtual void DidSetIsolate() {}

 private:
  Dart_Isolate isolate_;
  std::unique_ptr<DartClassLibrary> class_library_;
  std::unique_ptr<DartExceptionFactory> exception_factory_;
  std::unique_ptr<DartLibraryLoader> library_loader_;
  std::unique_ptr<DartStringCache> string_cache_;
  std::unique_ptr<DartTimerHeap> timer_heap_;
  DartPersistentValue index_handle_;

 protected:
  base::WeakPtrFactory<DartState> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(DartState);
};

}  // namespace blink

#endif  // SKY_ENGINE_TONIC_DART_STATE_H_
