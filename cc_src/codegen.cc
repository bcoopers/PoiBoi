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

#include "codegen.h"

#include <string>
#include <fstream>
#include <streambuf>

#include "code_suffices.h"
#include "evaluator.h"
#include "function.h"
#include "interpretation_context.h"

namespace pbc {
namespace {

void AddPBStringSrc(std::string& code_out) {
  std::ifstream fh("cc_src/poiboi_string.h");
  code_out.insert(code_out.end(), (std::istreambuf_iterator<char>(fh)),
                  std::istreambuf_iterator<char>());
  std::ifstream fc("cc_src/poiboi_string.cc");
  code_out.insert(code_out.end(), (std::istreambuf_iterator<char>(fc)),
                  std::istreambuf_iterator<char>());

}

std::string GetFunctionDeclaration(const Function& fn) {
  std::string code = kPbStringType + fn.GetName() + kFnSuffix + "(";
  if (fn.GetVariablesList().size() > 0) {
    code += kPbStringType + fn.GetVariablesList()[0] + kLocalVarSuffix;
    for (int i = 1; i < fn.GetVariablesList().size(); ++i) {
      code += ", " + (kPbStringType + fn.GetVariablesList()[i]) + kLocalVarSuffix;
    }
  }
  code += ")";
  return code;
}

ErrorCode GetFunctionDefinition(const Function& fn, CompilationContext& context,
                                std::string& code) {
  code = GetFunctionDeclaration(fn) + "{\n";
  for (const std::string& input_var : fn.GetVariablesList()) {
    context.curr_local_variables.insert(input_var);
  }

  const auto evaluator = CodeBlockEvaluator::TryCreate(fn.GetCode(), context);
  RETURN_EC_IF_FAILURE(evaluator);
  code += evaluator.GetItem().GetCode() + "\nreturn PBString();\n}\n\n\n";
  context.curr_local_variables = {};
  return ErrorCode::Success();
}
}  // namespace

ErrorCode GenerateCode(const std::vector<Module>& modules, std::string& code_out) {
  code_out.clear();
  std::vector<Function> functions = GetFunctionsFromModules(modules);
  if (functions.empty()) {
    return ErrorCode::Success();
  }
  std::unordered_map<std::string, const Function*> functions_dict = GetFunctionsDict(functions);
  std::unordered_set<std::string> global_variables;

  code_out += "#define POIBOI_EXECUTABLE_\n#define POIBOI_INCLUDE_ASSERT_\n";
  AddPBStringSrc(code_out);

  for (const Function& fn : functions) {
    code_out += GetFunctionDeclaration(fn) + ";\n";
  }

  CompilationContext context{.fns = &functions_dict, .all_global_variables = &global_variables};

  std::vector<std::string> fn_definitions;
  for (const Function& fn : functions) {
    RETURN_EC_IF_FAILURE(GetFunctionDefinition(fn, context, fn_definitions.emplace_back()));
  }

  // TODO: Add global variable, fn definitions, and main.

  return ErrorCode::Success();
}

}  // namespace pbc
