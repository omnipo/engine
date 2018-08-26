// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_ENGINE_TONIC_DART_DEPENDENCY_CATCHER_H_
#define SKY_ENGINE_TONIC_DART_DEPENDENCY_CATCHER_H_

#include <unordered_set>

#include "base/macros.h"

namespace blink {
class DartLibraryLoader;

// A base class to represent a dependency.
class DartDependency {};

// To catch the dependencies for a library, put a DartDependencyCatcher on the
// stack during the call to Dart_LoadLibrary.
class DartDependencyCatcher {
 public:
  explicit DartDependencyCatcher(DartLibraryLoader& loader);
  ~DartDependencyCatcher();

  void AddDependency(DartDependency* dependency);
  const std::unordered_set<DartDependency*>& dependencies() const {
    return dependencies_;
  }

 private:
  DartLibraryLoader& loader_;
  std::unordered_set<DartDependency*> dependencies_;

  DISALLOW_COPY_AND_ASSIGN(DartDependencyCatcher);
};

}  // namespace blink

#endif  // SKY_ENGINE_TONIC_DART_DEPENDENCY_CATCHER_H_
