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
#include <variant>

#include "function.h"
#include "grammar.h"
#include "tokens.h"
#include "interpretation_context.h"

namespace pbc {
namespace {
constexpr char kPbStringType[] = "PBString ";
constexpr char kFnSuffix[] = "_poiboi_fn";
constexpr char kLocalVarSuffix[] = "_local_poiboivar";

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

class StatementEvaluator {
 public:
  static std::unique_ptr<StatementEvaluator> Create(const Statement& statement);
  virtual ErrorCode GetCode(CompilationContext& context, std::string& code) const = 0;
};


class CodeBlockEvaluator {
 public:
  CodeBlockEvaluator(const CodeBlock& code_block);
  ErrorCode GetCode(CompilationContext& context, std::string& code) const;
 private:
  std::vector<std::unique_ptr<StatementEvaluator>> evaluators_;
};

class FunctionCallEvaluator;

class RValueEvaluator {
 public:
  RValueEvaluator(const RValue& rv);
  ErrorCode GetCode(CompilationContext& context, std::string& code) const;
 private:
  std::variant<QuotedString, Variable, std::unique_ptr<FunctionCallEvaluator>> op_;
};

class VariableAssignmentEvaluator : public StatementEvaluator {
 public:
  VariableAssignmentEvaluator(const VariableAssignment& va);
  ErrorCode GetCode(CompilationContext& context, std::string& code) const override;
 private:
  struct VarAssign {
    bool is_local = true;
    std::string name;
    RValueEvaluator e;
  };
  std::vector<VarAssign> var_assigns_;
};

class FunctionCallEvaluator : public StatementEvaluator {
   public:
    FunctionCallEvaluator(const FunctionCall& fc);
    ErrorCode GetCode(CompilationContext& context, std::string& code) const override;
   private:
    bool is_builtin = true;
    std::string name;
    std::vector<RValueEvaluator> args;
};

class WhileEvaluator : public StatementEvaluator {
 public:
  WhileEvaluator(const ConditionalEvaluation& ce, const CodeBlock& cb);
  ErrorCode GetCode(CompilationContext& context, std::string& code) const override;
 private:
  RValueEvaluator conditional_;
  CodeBlockEvaluator cbe_;
};

class ElseEvaluator {
 public:
  ElseEvaluator(const ElseStatement& ee);
  ErrorCode GetCode(CompilationContext& context, std::string& code);
 private:
  std::unique_ptr<RValueEvaluator> maybe_conditional_;
  CodeBlockEvaluator cbe_;
  std::unique_ptr<ElseEvaluator> maybe_else_;
};

class IfEvaluator : public StatementEvaluator {
 public:
  IfEvaluator(const ConditionalEvaluation& ce, const CodeBlock& cb, const ElseStatement& ee);
  ErrorCode GetCode(CompilationContext& context, std::string& code) const override;
 private:
  RValueEvaluator conditional_;
  CodeBlockEvaluator cbe_;
  std::unique_ptr<ElseEvaluator> maybe_else_;
};

class ReturnEvaluator : public StatementEvaluator {
 public:
  ReturnEvaluator(const RValue& rvalue);
  ErrorCode GetCode(CompilationContext& context, std::string& code) const override;
 private:
  RValueEvaluator rve_;
};

class BreakEvaluator : public StatementEvaluator {
 public:
  BreakEvaluator();
  ErrorCode GetCode(CompilationContext& context, std::string& code) const override;
};

ErrorCode GetFunctionDefinition(const Function& fn, CompilationContext& context,
                                std::string& code) {
  code = GetFunctionDeclaration(fn) + "{\n";

  // TODO: Flesh this out.
  code += "}\n\n";
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
  std::vector<std::string> global_variables;

  code_out += "#define POIBOI_EXECUTABLE_\n#define POIBOI_INCLUDE_ASSERT_\n";
  AddPBStringSrc(code_out);

  for (const Function& fn : functions) {
    code_out += GetFunctionDeclaration(fn) + ";\n";
  }

  CompilationContext context{.fns = &functions_dict, .global_variables = &global_variables};

  std::vector<std::string> fn_definitions;
  for (const Function& fn : functions) {
    const ErrorCode code = GetFunctionDefinition(fn, context, fn_definitions.emplace_back());
    if (code.IsFailure()) {
      return code;
    }
  }

  // TODO: Add global variable, fn definitions, and main.

  return ErrorCode::Success();
}

}  // namespace pbc
