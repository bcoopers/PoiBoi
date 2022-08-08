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

#include "function.h"

namespace pbc {
namespace {
std::vector<std::string> GenerateVariablesList(const VariablesList& variables) {
  std::vector<std::string> names;
  const auto& children = variables.GetChildren();
  if (children.size() == 0) {
    return names;
  }

  assert(children.size() == 2);
  assert(children[0]->GetLabel() == GrammarLabel::VARIABLE);
  names.push_back(children[0]->GetContent());

  const auto* expansion = &dynamic_cast<const VariablesListExpansion&>(*children[1]);
  for (; ;) {
    const auto& exp_children = expansion->GetChildren();
    if (exp_children.empty()) {
      return names;
    }
    assert(exp_children[1]->GetLabel() == GrammarLabel::VARIABLE);
    names.push_back(exp_children[1]->GetContent());
    expansion = &dynamic_cast<const VariablesListExpansion&>(*exp_children[2]);
  }
}
}  // namespace

Function::Function(const FunctionDefinition& fn_def) {
  const auto& children = fn_def.GetChildren();
  assert(children.size() == 5);
  assert(children[0]->GetLabel() == GrammarLabel::FUNCTION_NAME);
  name_ = children[0]->GetContent();

  variables_list_ = GenerateVariablesList(dynamic_cast<const VariablesList&>(*children[2]));

  assert(children[4]->GetLabel() == GrammarLabel::CODE_BLOCK);
  code_ = &dynamic_cast<const CodeBlock&>(*children[4]);

  file_name_ = fn_def.file_name();
  line_num_ = fn_def.line_number();
}

std::vector<Function> GetFunctionsFromModules(const std::vector<Module>& modules) {
  std::vector<Function> functions;
  for (const Module& root : modules) {
    const Module* curr_module = &root;
    for (; ;) {
      const auto& children = curr_module->GetChildren();
      if (children.size() == 1) {
        assert(children[0]->GetLabel() == GrammarLabel::END_OF_FILE);
        break;
      }
      assert(children.size() == 2);
      functions.emplace_back(dynamic_cast<const FunctionDefinition&>(*children[0]));
      curr_module = &dynamic_cast<const Module&>(*children[1]);
    }
  }
  return functions;
}

ErrorOr<std::unordered_map<std::string, const Function*>> GetFunctionsDict(
    const std::vector<Function>& fns_list) {
  std::unordered_map<std::string, const Function*> fns_dict;
  for (const Function& fn : fns_list) {
    const Function*& val = fns_dict[fn.GetName()];
    if (val != nullptr) {
      return ErrorCode::Failure("File: " + fn.GetFileName() + "; line: " + std::to_string(fn.GetLineNum()) +
                                "; Function " + fn.GetName() + " defined twice. Previously at file: " +
                                val->GetFileName() + "; line: " + std::to_string(val->GetLineNum()));
    }
    val = &fn;
  }
  return fns_dict;
}

}  // namespace pbc
