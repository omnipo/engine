// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MOJO_EDK_SYSTEM_TEST_UTILS_H_
#define MOJO_EDK_SYSTEM_TEST_UTILS_H_

#include "mojo/edk/embedder/simple_platform_support.h"
#include "mojo/public/c/system/types.h"
#include "mojo/public/cpp/system/macros.h"

namespace mojo {
namespace system {
namespace test {

// Deadlines/timeouts and sleeping ---------------------------------------------

MojoDeadline DeadlineFromMilliseconds(unsigned milliseconds);

// A timeout smaller than |TestTimeouts::tiny_timeout()|, as a |MojoDeadline|.
// Warning: This may lead to flakiness, but this is unavoidable if, e.g., you're
// trying to ensure that functions with timeouts are reasonably accurate. We
// want this to be as small as possible without causing too much flakiness.
MojoDeadline EpsilonDeadline();

// |TestTimeouts::tiny_timeout()|, as a |MojoDeadline|. (Expect this to be on
// the order of 100 ms.)
MojoDeadline TinyDeadline();

// |TestTimeouts::action_timeout()|, as a |MojoDeadline|. (Expect this to be on
// the order of 10 s.)
MojoDeadline ActionDeadline();

// Sleeps for at least the specified duration.
void Sleep(MojoDeadline deadline);

// Pseudorandom numbers for testing --------------------------------------------

// Returns a (uniformly) (pseudo)random integer in the interval [min, max].
// Currently, |max - min| must be at most |RAND_MAX| and must also be (strictly)
// less than |INT_MAX|.
int RandomInt(int min, int max);

// Stopwatch -------------------------------------------------------------------

// A simple "stopwatch" for measuring time elapsed from a given starting point.
class Stopwatch {
 public:
  Stopwatch();
  ~Stopwatch();

  void Start();
  // Returns the amount of time elapsed since the last call to |Start()| (in
  // microseconds).
  MojoDeadline Elapsed();

 private:
  // TODO(vtl): We need this for |GetTimeTicksNow()|. Maybe we should have a
  // singleton for tests instead? Or maybe it should be injected?
  embedder::SimplePlatformSupport platform_support_;

  MojoTimeTicks start_time_;

  MOJO_DISALLOW_COPY_AND_ASSIGN(Stopwatch);
};

}  // namespace test
}  // namespace system
}  // namespace mojo

#endif  // MOJO_EDK_SYSTEM_TEST_UTILS_H_
