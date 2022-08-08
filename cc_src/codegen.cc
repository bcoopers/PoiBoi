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

#include <algorithm>
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
  auto maybe_fns_dict = GetFunctionsDict(functions);
  RETURN_EC_IF_FAILURE(maybe_fns_dict);
  std::unordered_map<std::string, const Function*> functions_dict = std::move(maybe_fns_dict.GetItem());
  std::unordered_set<std::string> global_variables;

  const Function* main_fn = functions_dict["Main"];
  if (main_fn == nullptr) {
    return ErrorCode::Failure("No Main fn defined");
  }

  const int num_main_args = main_fn->GetVariablesList().size();
  if (num_main_args > 1) {
    return ErrorCode::Failure("File: " + main_fn->GetFileName() + "; line: " + std::to_string(main_fn->GetLineNum()) +
                              "; Main accepts too many args: " + std::to_string(num_main_args));
  }

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

  std::vector<std::string> sorted_globals(global_variables.begin(), global_variables.end());
  std::sort(sorted_globals.begin(), sorted_globals.end());
  for (const std::string& global : sorted_globals) {
    code_out += std::string(kPbStringType) + global + kGlobalVarSuffix +";\n";
  }

  for (const std::string& fn_def : fn_definitions) {
    code_out += fn_def + "\n\n\n";
  }

  const std::string main_cc_fn = std::string("Main") + kFnSuffix;

  if (num_main_args == 0) {
    code_out += "int main(int argc, char** argv) {\n";
    code_out += main_cc_fn + "();\nreturn 0;\n}";
  } else {
    code_out += "int main(int argc, char** argv) {\n";
    code_out += "if (argc == 1) {\n";
    code_out += main_cc_fn + "(PBString());\n} else {\n";
    code_out += main_cc_fn + "(PBString::NewStaticString(argv[1]));\n}";
    code_out += "\nreturn 0;\n}";
  }

  return ErrorCode::Success();
}

}  // namespace pbc
