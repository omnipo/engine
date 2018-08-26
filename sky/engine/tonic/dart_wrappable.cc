// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/tonic/dart_wrappable.h"

#include "sky/engine/tonic/dart_class_library.h"
#include "sky/engine/tonic/dart_error.h"
#include "sky/engine/tonic/dart_exception_factory.h"
#include "sky/engine/tonic/dart_state.h"
#include "sky/engine/tonic/dart_wrapper_info.h"

namespace blink {

DartWrappable::~DartWrappable() {
  CHECK(!dart_wrapper_);
}

void DartWrappable::AcceptDartGCVisitor(DartGCVisitor& visitor) const {
}

Dart_Handle DartWrappable::CreateDartWrapper(DartState* dart_state) {
  DCHECK(!dart_wrapper_);
  const DartWrapperInfo& info = GetDartWrapperInfo();

  Dart_PersistentHandle type = dart_state->class_library().GetClass(info);
  DCHECK(!LogIfError(type));

  intptr_t native_fields[kNumberOfNativeFields];
  native_fields[kPeerIndex] = reinterpret_cast<intptr_t>(this);
  native_fields[kWrapperInfoIndex] = reinterpret_cast<intptr_t>(&info);
  Dart_Handle wrapper =
      Dart_AllocateWithNativeFields(type, kNumberOfNativeFields, native_fields);
  DCHECK(!LogIfError(wrapper));

  info.ref_object(this);  // Balanced in FinalizeDartWrapper.
  dart_wrapper_ = Dart_NewWeakPersistentHandle(
      wrapper, this, info.size_in_bytes, &FinalizeDartWrapper);

  return wrapper;
}

void DartWrappable::AssociateWithDartWrapper(Dart_NativeArguments args) {
  DCHECK(!dart_wrapper_);

  Dart_Handle wrapper = Dart_GetNativeArgument(args, 0);
  CHECK(!LogIfError(wrapper));

  intptr_t native_fields[kNumberOfNativeFields];
  CHECK(!LogIfError(Dart_GetNativeFieldsOfArgument(
      args, 0, kNumberOfNativeFields, native_fields)));
  CHECK(!native_fields[kPeerIndex]);
  CHECK(!native_fields[kWrapperInfoIndex]);

  const DartWrapperInfo& info = GetDartWrapperInfo();
  CHECK(!LogIfError(Dart_SetNativeInstanceField(
      wrapper, kPeerIndex, reinterpret_cast<intptr_t>(this))));
  CHECK(!LogIfError(Dart_SetNativeInstanceField(
      wrapper, kWrapperInfoIndex, reinterpret_cast<intptr_t>(&info))));

  info.ref_object(this);  // Balanced in FinalizeDartWrapper.
  dart_wrapper_ = Dart_NewWeakPersistentHandle(
      wrapper, this, info.size_in_bytes, &FinalizeDartWrapper);
}

void DartWrappable::FinalizeDartWrapper(void* isolate_callback_data,
                                        Dart_WeakPersistentHandle wrapper,
                                        void* peer) {
  DartWrappable* wrappable = reinterpret_cast<DartWrappable*>(peer);
  wrappable->dart_wrapper_ = nullptr;
  const DartWrapperInfo& info = wrappable->GetDartWrapperInfo();
  info.deref_object(wrappable);  // Balanced in CreateDartWrapper.
}

DartWrappable* DartConverterWrappable::FromDart(Dart_Handle handle) {
  intptr_t peer = 0;
  Dart_Handle result =
      Dart_GetNativeInstanceField(handle, DartWrappable::kPeerIndex, &peer);
  if (Dart_IsError(result))
    return nullptr;
  return reinterpret_cast<DartWrappable*>(peer);
}

DartWrappable* DartConverterWrappable::FromArguments(Dart_NativeArguments args,
                                                     int index,
                                                     Dart_Handle& exception) {
  intptr_t native_fields[DartWrappable::kNumberOfNativeFields];
  Dart_Handle result = Dart_GetNativeFieldsOfArgument(
      args, index, DartWrappable::kNumberOfNativeFields, native_fields);
  if (Dart_IsError(result)) {
    exception = Dart_NewStringFromCString(DartError::kInvalidArgument);
    return nullptr;
  }
  return reinterpret_cast<DartWrappable*>(
      native_fields[DartWrappable::kPeerIndex]);
}

DartWrappable* DartConverterWrappable::FromArgumentsWithNullCheck(
    Dart_NativeArguments args, int index, Dart_Handle& exception) {
  Dart_Handle handle = Dart_GetNativeArgument(args, index);
  if (Dart_IsNull(handle)) {
    DartState* state = DartState::Current();
    exception = state->exception_factory().CreateNullArgumentException(index);
    return nullptr;
  }
  intptr_t native_fields[DartWrappable::kNumberOfNativeFields];
  Dart_Handle result = Dart_GetNativeFieldsOfArgument(
      args, index, DartWrappable::kNumberOfNativeFields, native_fields);
  if (Dart_IsError(result)) {
    exception = Dart_NewStringFromCString(DartError::kInvalidArgument);
    return nullptr;
  }
  return reinterpret_cast<DartWrappable*>(
      native_fields[DartWrappable::kPeerIndex]);
}

}  // namespace blink
