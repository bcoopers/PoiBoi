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

#ifndef POIBOIC_EVALUATOR_H_
#define POIBOIC_EVALUATOR_H_

#include <string>
#include <vector>

#include "error_code.h"
#include "grammar.h"
#include "tokens.h"
#include "interpretation_context.h"

namespace pbc {

class StatementEvaluator;

class CodeBlockEvaluator {
 public:
  static ErrorOr<CodeBlockEvaluator> TryCreate(
    const CodeBlock& code_block, CompilationContext& context);
  std::string GetCode() const;
 private:
  std::vector<std::unique_ptr<StatementEvaluator>> evaluators_;
};

}  // namespace pbc
#endif  // #ifndef POIBOIC_EVALUATOR_H_
