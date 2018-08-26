// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "dart_service_isolate.h"

#include "base/logging.h"
#include "dart/runtime/include/dart_api.h"
#include "sky/engine/tonic/dart_error.h"
#include "sky/engine/tonic/dart_library_natives.h"
#include "sky/engine/tonic/dart_string.h"

#define RETURN_ERROR_HANDLE(handle)                             \
  if (Dart_IsError(handle)) {                                   \
    return handle;                                              \
  }

#define SHUTDOWN_ON_ERROR(handle)                               \
  if (Dart_IsError(handle)) {                                   \
    *error = strdup(Dart_GetError(handle));                     \
    Dart_ExitScope();                                           \
    Dart_ShutdownIsolate();                                     \
    return false;                                               \
  }

#define kLibrarySourceNamePrefix "/dart_service_isolate"
static const char* kServiceIsolateScript = "main.dart";

struct ResourcesEntry {
  const char* path_;
  const char* resource_;
  int length_;
};

namespace mojo {
  namespace dart {
    extern ResourcesEntry __sky_embedder_service_isolate_resources_[];
  }
}

namespace blink {
namespace {

static Dart_LibraryTagHandler g_embedder_tag_handler;
static DartLibraryNatives* g_natives;

Dart_NativeFunction GetNativeFunction(Dart_Handle name,
                                      int argument_count,
                                      bool* auto_setup_scope) {
  CHECK(g_natives);
  return g_natives->GetNativeFunction(name, argument_count, auto_setup_scope);
}

const uint8_t* GetSymbol(Dart_NativeFunction native_function) {
  CHECK(g_natives);
  return g_natives->GetSymbol(native_function);
}

}  // namespace

class Resources {
 public:
  static const int kNoSuchInstance = -1;
  static int ResourceLookup(const char* path, const char** resource) {
    ResourcesEntry* table = ResourcesTable();
    for (int i = 0; table[i].path_ != NULL; i++) {
      const ResourcesEntry& entry = table[i];
      if (strcmp(path, entry.path_) == 0) {
        *resource = entry.resource_;
        DCHECK(entry.length_ > 0);
        return entry.length_;
      }
    }
    return kNoSuchInstance;
  }

  static const char* Path(int idx) {
    DCHECK(idx >= 0);
    ResourcesEntry* entry = At(idx);
    if (entry == NULL) {
      return NULL;
    }
    DCHECK(entry->path_ != NULL);
    return entry->path_;
  }

 private:
  static ResourcesEntry* At(int idx) {
    DCHECK(idx >= 0);
    ResourcesEntry* table = ResourcesTable();
    for (int i = 0; table[i].path_ != NULL; i++) {
      if (idx == i) {
        return &table[i];
      }
    }
    return NULL;
  }
  static ResourcesEntry* ResourcesTable() {
    return &mojo::dart::__sky_embedder_service_isolate_resources_[0];
  }
};

void DartServiceIsolate::TriggerResourceLoad(Dart_NativeArguments args) {
  Dart_Handle library = Dart_RootLibrary();
  DCHECK(!Dart_IsError(library));
  Dart_Handle result = LoadResources(library);
  DCHECK(!Dart_IsError(result));
}

void DartServiceIsolate::NotifyServerState(Dart_NativeArguments args) {
  // NO-OP.
}

void DartServiceIsolate::Shutdown(Dart_NativeArguments args) {
  // NO-OP.
}

bool DartServiceIsolate::Startup(std::string server_ip,
                                 intptr_t server_port,
                                 Dart_LibraryTagHandler embedder_tag_handler,
                                 bool running_precompiled,
                                 char** error) {
  Dart_Isolate isolate = Dart_CurrentIsolate();
  CHECK(isolate);

  // Remember the embedder's library tag handler.
  g_embedder_tag_handler = embedder_tag_handler;
  CHECK(g_embedder_tag_handler);

  // Setup native entries.
  if (!g_natives) {
    g_natives = new DartLibraryNatives();
    g_natives->Register({
      {"ServiceIsolate_TriggerResourceLoad", TriggerResourceLoad, 0, true },
      {"ServiceIsolate_NotifyServerState", NotifyServerState, 2, true },
      {"ServiceIsolate_Shutdown", Shutdown, 0, true },
    });
  }

  Dart_Handle result;

  if (running_precompiled) {
    Dart_Handle uri = Dart_NewStringFromCString("dart:vmservice_sky");
    Dart_Handle library = Dart_LookupLibrary(uri);
    SHUTDOWN_ON_ERROR(library);
    result = Dart_SetRootLibrary(library);
    SHUTDOWN_ON_ERROR(result);
    result = Dart_SetNativeResolver(library, GetNativeFunction, GetSymbol);
    SHUTDOWN_ON_ERROR(result);
  } else {
    // Use our own library tag handler when loading service isolate sources.
    Dart_SetLibraryTagHandler(DartServiceIsolate::LibraryTagHandler);
    // Load main script.
    Dart_Handle library = LoadScript(kServiceIsolateScript);
    DCHECK(library != Dart_Null());
    SHUTDOWN_ON_ERROR(library);
    // Setup native entry resolution.
    result = Dart_SetNativeResolver(library, GetNativeFunction, GetSymbol);

    SHUTDOWN_ON_ERROR(result);
    // Finalize loading.
    result = Dart_FinalizeLoading(false);
    SHUTDOWN_ON_ERROR(result);
  }

  // Make runnable.
  Dart_ExitScope();
  Dart_ExitIsolate();
  bool retval = Dart_IsolateMakeRunnable(isolate);
  if (!retval) {
    Dart_EnterIsolate(isolate);
    Dart_ShutdownIsolate();
    *error = strdup("Invalid isolate state - Unable to make it runnable.");
    return false;
  }
  Dart_EnterIsolate(isolate);
  Dart_EnterScope();

  Dart_Handle library = Dart_RootLibrary();
  SHUTDOWN_ON_ERROR(library);

  // Set the HTTP server's ip.
  result = Dart_SetField(library,
                         Dart_NewStringFromCString("_ip"),
                         Dart_NewStringFromCString(server_ip.c_str()));
  SHUTDOWN_ON_ERROR(result);
  // If we have a port specified, start the server immediately.
  bool auto_start = server_port >= 0;
  if (server_port < 0) {
    // Adjust server_port to port 0 which will result in the first available
    // port when the HTTP server is started.
    server_port = 0;
  }
  // Set the HTTP's servers port.
  result = Dart_SetField(library,
                         Dart_NewStringFromCString("_port"),
                         Dart_NewInteger(server_port));
  SHUTDOWN_ON_ERROR(result);
  result = Dart_SetField(library,
                         Dart_NewStringFromCString("_autoStart"),
                         Dart_NewBoolean(auto_start));
  SHUTDOWN_ON_ERROR(result);
  return true;
}

Dart_Handle DartServiceIsolate::GetSource(const char* name) {
  const intptr_t kBufferSize = 512;
  char buffer[kBufferSize];
  snprintf(&buffer[0], kBufferSize-1, "%s/%s", kLibrarySourceNamePrefix, name);
  const char* vmservice_source = NULL;
  int r = Resources::ResourceLookup(buffer, &vmservice_source);
  DCHECK(r != Resources::kNoSuchInstance);
  return Dart_NewStringFromCString(vmservice_source);
}

Dart_Handle DartServiceIsolate::LoadScript(const char* name) {
  Dart_Handle url = Dart_NewStringFromCString("dart:vmservice_sky");
  Dart_Handle source = GetSource(name);
  return Dart_LoadScript(url, source, 0, 0);
}

Dart_Handle DartServiceIsolate::LoadSource(Dart_Handle library, const char* name) {
  Dart_Handle url = Dart_NewStringFromCString(name);
  Dart_Handle source = GetSource(name);
  return Dart_LoadSource(library, url, source, 0, 0);
}

Dart_Handle DartServiceIsolate::LoadResource(Dart_Handle library,
                                    const char* resource_name) {
  // Prepare for invoke call.
  Dart_Handle name = Dart_NewStringFromCString(resource_name);
  RETURN_ERROR_HANDLE(name);
  const char* data_buffer = NULL;
  int data_buffer_length = Resources::ResourceLookup(resource_name,
                                                     &data_buffer);
  DCHECK(data_buffer_length != Resources::kNoSuchInstance);
  Dart_Handle data_list = Dart_NewTypedData(Dart_TypedData_kUint8,
                                            data_buffer_length);
  RETURN_ERROR_HANDLE(data_list);
  Dart_TypedData_Type type = Dart_TypedData_kInvalid;
  void* data_list_buffer = NULL;
  intptr_t data_list_buffer_length = 0;
  Dart_Handle result = Dart_TypedDataAcquireData(data_list, &type,
                                                 &data_list_buffer,
                                                 &data_list_buffer_length);
  RETURN_ERROR_HANDLE(result);
  DCHECK(data_buffer_length == data_list_buffer_length);
  DCHECK(data_list_buffer != NULL);
  DCHECK(type = Dart_TypedData_kUint8);
  memmove(data_list_buffer, &data_buffer[0], data_buffer_length);
  result = Dart_TypedDataReleaseData(data_list);
  RETURN_ERROR_HANDLE(result);

  // Make invoke call.
  const intptr_t kNumArgs = 2;
  Dart_Handle args[kNumArgs] = { name, data_list };
  result = Dart_Invoke(library, Dart_NewStringFromCString("_addResource"),
                       kNumArgs, args);
  return result;
}

Dart_Handle DartServiceIsolate::LoadResources(Dart_Handle library) {
  Dart_Handle result = Dart_Null();
  intptr_t prefixLen = strlen(kLibrarySourceNamePrefix);
  for (intptr_t i = 0; Resources::Path(i) != NULL; i++) {
    const char* path = Resources::Path(i);
    // If it doesn't begin with kLibrarySourceNamePrefix it is a frontend
    // resource.
    if (strncmp(path, kLibrarySourceNamePrefix, prefixLen) != 0) {
      result = LoadResource(library, path);
      if (Dart_IsError(result)) {
        break;
      }
    }
  }
  return result;
}

Dart_Handle DartServiceIsolate::LibraryTagHandler(Dart_LibraryTag tag,
                                                  Dart_Handle library,
                                                  Dart_Handle url) {
  if (!Dart_IsLibrary(library)) {
    return Dart_NewApiError("not a library");
  }
  if (!Dart_IsString(url)) {
    return Dart_NewApiError("url is not a string");
  }
  const char* url_string = NULL;
  Dart_Handle result = Dart_StringToCString(url, &url_string);
  if (Dart_IsError(result)) {
    return result;
  }
  Dart_Handle library_url = Dart_LibraryUrl(library);
  const char* library_url_string = NULL;
  result = Dart_StringToCString(library_url, &library_url_string);
  if (Dart_IsError(result)) {
    return result;
  }
  if (tag == Dart_kImportTag) {
    // Embedder handles all requests for external libraries.
    return g_embedder_tag_handler(tag, library, url);
  }
  DCHECK((tag == Dart_kSourceTag) || (tag == Dart_kCanonicalizeUrl));
  if (tag == Dart_kCanonicalizeUrl) {
    // url is already canonicalized.
    return url;
  }
  // Get source from builtin resources.
  Dart_Handle source = GetSource(url_string);
  if (Dart_IsError(source)) {
    return source;
  }
  return Dart_LoadSource(library, url, source, 0, 0);
}


}  // namespace blink
