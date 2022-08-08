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

#ifndef POIBOIC_FUNCTION_H_
#define POIBOIC_FUNCTION_H_

#include <string>
#include <unordered_map>
#include <vector>

#include "error_code.h"
#include "grammar.h"

namespace pbc {

class Function {
 public:
  Function(const FunctionDefinition& fn_def);
  const std::string& GetName() const { return name_; }
  const std::vector<std::string>& GetVariablesList() const { return variables_list_; }
  const CodeBlock& GetCode() const { return *code_; }

  const std::string& GetFileName() const { return file_name_; }
  size_t GetLineNum() const { return line_num_; };


 private:
  std::string name_;
  std::vector<std::string> variables_list_;
  const CodeBlock* code_{};

  std::string file_name_;
  size_t line_num_{};
};

std::vector<Function> GetFunctionsFromModules(const std::vector<Module>& modules);

ErrorOr<std::unordered_map<std::string, const Function*>> GetFunctionsDict(const std::vector<Function>& fns_list);

}  // namespace pbc

#endif  // #ifndef POIBOIC_FUNCTION_H_
