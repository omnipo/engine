// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/core/script/dart_init.h"

#include <dlfcn.h>

#include "base/bind.h"
#include "base/logging.h"
#include "base/single_thread_task_runner.h"
#include "base/trace_event/trace_event.h"
#include "dart/runtime/bin/embedded_dart_io.h"
#include "dart/runtime/include/dart_mirrors_api.h"
#include "gen/sky/platform/RuntimeEnabledFeatures.h"
#include "sky/engine/bindings/dart_mojo_internal.h"
#include "sky/engine/bindings/dart_runtime_hooks.h"
#include "sky/engine/bindings/dart_ui.h"
#include "sky/engine/core/script/dart_debugger.h"
#include "sky/engine/core/script/dart_service_isolate.h"
#include "sky/engine/core/script/dom_dart_state.h"
#include "sky/engine/tonic/dart_api_scope.h"
#include "sky/engine/tonic/dart_class_library.h"
#include "sky/engine/tonic/dart_dependency_catcher.h"
#include "sky/engine/tonic/dart_error.h"
#include "sky/engine/tonic/dart_invoke.h"
#include "sky/engine/tonic/dart_io.h"
#include "sky/engine/tonic/dart_isolate_scope.h"
#include "sky/engine/tonic/dart_library_loader.h"
#include "sky/engine/tonic/dart_snapshot_loader.h"
#include "sky/engine/tonic/dart_state.h"
#include "sky/engine/tonic/dart_wrappable.h"

namespace blink {

Dart_Handle DartLibraryTagHandler(Dart_LibraryTag tag,
                                  Dart_Handle library,
                                  Dart_Handle url) {
  return DartLibraryLoader::HandleLibraryTag(tag, library, url);
}

void EnsureHandleWatcherStarted() {
  static bool handle_watcher_started = false;
  if (handle_watcher_started)
    return;

  // TODO(dart): Call Dart_Cleanup (ensure the handle watcher isolate is closed)
  // during shutdown.
  Dart_Handle mojo_core_lib = Dart_LookupLibrary(ToDart("dart:mojo.internal"));
  CHECK(!LogIfError((mojo_core_lib)));
  Dart_Handle handle_watcher_type = Dart_GetType(
      mojo_core_lib,
      Dart_NewStringFromCString("MojoHandleWatcher"),
      0,
      nullptr);
  CHECK(!LogIfError(handle_watcher_type));
  CHECK(!LogIfError(Dart_Invoke(
      handle_watcher_type,
      Dart_NewStringFromCString("_start"),
      0,
      nullptr)));

  // RunLoop until the handle watcher isolate is spun-up.
  CHECK(!LogIfError(Dart_RunLoop()));
  handle_watcher_started = true;
}

namespace {

void CreateEmptyRootLibraryIfNeeded() {
  if (Dart_IsNull(Dart_RootLibrary())) {
    Dart_LoadScript(Dart_NewStringFromCString("dart:empty"), Dart_EmptyString(),
                    0, 0);
  }
}

static const char* kDartArgs[] = {
    "--enable_mirrors=false",
    // Dart assumes ARM devices are insufficiently powerful and sets the
    // default profile period to 100Hz. This number is suitable for older
    // Raspberry Pi devices but quite low for current smartphones.
    "--profile_period=1000",
#if (WTF_OS_IOS || WTF_OS_MACOSX) && !defined(NDEBUG)
    // On platforms where LLDB is the primary debugger, SIGPROF signals
    // overwhelm LLDB. Since in debug builds, CPU profiling information is
    // less useful anyway, vm profiling is disabled to enable a usable debugger.
    "--no-profile",
#endif
};

static const char* kDartPrecompilationArgs[] {
  "--precompilation",
};

static const char* kDartCheckedModeArgs[] = {
  "--enable_asserts",
  "--enable_type_checks",
  "--error_on_bad_type",
  "--error_on_bad_override",
};

void UnhandledExceptionCallback(Dart_Handle error) {
  LOG(ERROR) << Dart_GetError(error);
}

void IsolateShutdownCallback(void* callback_data) {
  // TODO(dart)
}

bool IsServiceIsolateURL(const char* url_name) {
  return url_name != nullptr &&
      String(url_name) == DART_VM_SERVICE_ISOLATE_NAME;
}

static const uint8_t* PrecompiledInstructionsSymbolIfPresent() {
  dlerror();  // clear previous errors on thread
  void* sym = nullptr;
#ifdef RTLD_SELF
  static const char kInstructionsSnapshotSymbolName[] = "kInstructionsSnapshot";
  sym = dlsym(RTLD_SELF, kInstructionsSnapshotSymbolName);
#endif
  return (dlerror() != nullptr) ? nullptr : reinterpret_cast<uint8_t * >(sym);
}

static bool IsRunningPrecompiledCode() {
  return PrecompiledInstructionsSymbolIfPresent() != nullptr;
}

// TODO(rafaelw): Right now this only supports the creation of the handle
// watcher isolate and the service isolate. Presumably, we'll want application
// isolates to spawn their own isolates.
Dart_Isolate IsolateCreateCallback(const char* script_uri,
                                   const char* main,
                                   const char* package_root,
                                   const char** package_map,
                                   Dart_IsolateFlags* flags,
                                   void* callback_data,
                                   char** error) {
  if (IsServiceIsolateURL(script_uri)) {
    CHECK(kDartIsolateSnapshotBuffer);
    DartState* dart_state = new DartState();
    Dart_Isolate isolate =
        Dart_CreateIsolate(script_uri, "main", kDartIsolateSnapshotBuffer,
                           nullptr, nullptr, error);
    CHECK(isolate) << error;
    dart_state->SetIsolate(isolate);
    CHECK(Dart_IsServiceIsolate(isolate));
    CHECK(!LogIfError(Dart_SetLibraryTagHandler(DartLibraryTagHandler)));
    {
      DartApiScope dart_api_scope;
      DartIO::InitForIsolate();
      DartUI::InitForIsolate();
      DartMojoInternal::InitForIsolate();
      DartRuntimeHooks::Install(DartRuntimeHooks::DartIOIsolate);
      // Start the handle watcher from the service isolate so it isn't available
      // for debugging or general Observatory interaction.
      EnsureHandleWatcherStarted();
      if (RuntimeEnabledFeatures::observatoryEnabled()) {
        std::string ip = "127.0.0.1";
        const intptr_t port = 8181;
        const bool service_isolate_booted =
            DartServiceIsolate::Startup(ip, port, DartLibraryTagHandler,
                                        IsRunningPrecompiledCode(), error);
        CHECK(service_isolate_booted) << error;
      }
    }
    Dart_ExitIsolate();
    return isolate;
  }

  // Create & start the handle watcher isolate
  CHECK(kDartIsolateSnapshotBuffer);
  // TODO(abarth): Who deletes this DartState instance?
  DartState* dart_state = new DartState();
  Dart_Isolate isolate =
      Dart_CreateIsolate("sky:handle_watcher", "", kDartIsolateSnapshotBuffer,
                         nullptr, dart_state, error);
  CHECK(isolate) << error;
  dart_state->SetIsolate(isolate);

  CHECK(!LogIfError(Dart_SetLibraryTagHandler(DartLibraryTagHandler)));

  {
    DartApiScope dart_api_scope;
    DartIO::InitForIsolate();
    DartUI::InitForIsolate();
    DartMojoInternal::InitForIsolate();

    if (!script_uri)
      CreateEmptyRootLibraryIfNeeded();
  }

  Dart_ExitIsolate();

  CHECK(Dart_IsolateMakeRunnable(isolate));
  return isolate;
}

} // namespace

void InitDartVM() {
  dart::bin::BootstrapDartIo();

  bool enable_checked_mode = RuntimeEnabledFeatures::dartCheckedModeEnabled();
#if ENABLE(DART_STRICT)
  enable_checked_mode = true;
#endif

  Vector<const char*> args;
  args.append(kDartArgs, arraysize(kDartArgs));

  if (IsRunningPrecompiledCode())
    args.append(kDartPrecompilationArgs, arraysize(kDartPrecompilationArgs));

  if (enable_checked_mode)
    args.append(kDartCheckedModeArgs, arraysize(kDartCheckedModeArgs));

  CHECK(Dart_SetVMFlags(args.size(), args.data()));
  // This should be called before calling Dart_Initialize.
  DartDebugger::InitDebugger();
  CHECK(Dart_Initialize(
      kDartVmIsolateSnapshotBuffer,
      PrecompiledInstructionsSymbolIfPresent(),
      IsolateCreateCallback,
      nullptr,  // Isolate interrupt callback.
      UnhandledExceptionCallback, IsolateShutdownCallback,
      // File IO callbacks.
      nullptr, nullptr, nullptr, nullptr,
      // Entroy source
      nullptr,
      // VM service assets archive
      nullptr) == nullptr);
  // Wait for load port- ensures handle watcher and service isolates are
  // running.
  Dart_ServiceWaitForLoadPort();
}

} // namespace blink
