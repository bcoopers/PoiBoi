/*
Copyright 2018 Brian Coopersmith

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

// Defines the full expansion of the PoiBoi grammar. Everything terminates
// as tokens.

#ifndef _GRAMMAR_H_
#define _GRAMMAR_H_

#include <memory>
#include <string>
#include <vector>

#include "tokens.h"

namespace pbc {


class Module : public GrammarPiece {
  std::vector<std::vector<std::unique_ptr<GrammarPiece>>>
  GetDescendents() const override {
    return {{std::unique_ptr<EndOfEndOfFile>(new EndOfFile)},
            {std::unique_ptr<FunctionDefinition>(new FunctionDefinition)};
  }
};

}  // namespace pbc

#endif  // #ifndef _GRAMMAR_H_
