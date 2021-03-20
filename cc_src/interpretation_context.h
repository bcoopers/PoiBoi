/*
Copyright 2021 Brian Coopersmith

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef POIBOIC_INTERPRETATION_CONTEXT_H_
#define POIBOIC_INTERPRETATION_CONTEXT_H_

#include <unordered_map>

#include "function.h"

namespace pbc {

// Context used for compilation which is not available locally.
struct CompilationContext {
  const std::unordered_map<std::string, const Function*>* fns{};
  std::vector<std::string>* global_variables{};
  std::vector<std::string> local_variables;
  bool is_in_loop = false;
};

}  // namespace pbc

#endif  // #ifndef POIBOIC_INTERPRETATION_CONTEXT_H_
