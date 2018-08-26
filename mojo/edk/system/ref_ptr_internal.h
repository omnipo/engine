// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_EDK_SYSTEM_REF_PTR_INTERNAL_H_
#define MOJO_EDK_SYSTEM_REF_PTR_INTERNAL_H_

#include <utility>

#include "mojo/public/cpp/system/macros.h"

namespace mojo {
namespace system {

template <typename T>
class RefPtr;

template <typename T>
RefPtr<T> AdoptRef(T* ptr);

namespace internal {

// This is a wrapper class that can be friended for a particular |T|, if you
// want to make |T|'s constructor private, but still use |MakeRefCounted()|
// (below). (You can't friend partial specializations.) See |MakeRefCounted()|
// and |FRIEND_MAKE_REF_COUNTED()|.
template <typename T>
class MakeRefCountedHelper {
 public:
  template <typename... Args>
  static RefPtr<T> MakeRefCounted(Args&&... args) {
    return AdoptRef<T>(new T(std::forward<Args>(args)...));
  }
};

}  // namespace internal
}  // namespace system
}  // namespace mojo

#endif  // MOJO_EDK_SYSTEM_REF_PTR_INTERNAL_H_
