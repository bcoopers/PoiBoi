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

#include "evaluator.h"

#include <variant>

#include "tokens.h"

namespace pbc {

class StatementEvaluator {
 public:
  static ErrorOr<std::unique_ptr<StatementEvaluator>> TryCreate(
    const Statement& statement, CompilationContext& context);
  virtual std::string GetCode() const = 0;
};


class FunctionCallEvaluator;

class RValueEvaluator {
 public:
  static ErrorOr<RValueEvaluator> TryCreate(const RValue& rv, CompilationContext& context);
  std::string GetCode() const;
 private:
  std::variant<QuotedString, Variable, std::unique_ptr<FunctionCallEvaluator>> op_;
};

class VariableAssignmentEvaluator : public StatementEvaluator {
 public:
  static ErrorOr<VariableAssignmentEvaluator> TryCreate(const VariableAssignment& va, CompilationContext& context);
  std::string GetCode() const override;
 private:
  struct VarAssign {
    bool is_local = true;
    bool is_defined = true;
    std::string name;
    RValueEvaluator e;
  };
  std::vector<VarAssign> var_assigns_;
};

class FunctionCallEvaluator : public StatementEvaluator {
   public:
    static ErrorOr<FunctionCallEvaluator> TryCreate(const FunctionCall& fc, CompilationContext& context);
    std::string GetCode() const override;
   private:
    bool is_builtin = true;
    std::string name;
    std::vector<RValueEvaluator> args;
};

class WhileEvaluator : public StatementEvaluator {
 public:
  static ErrorOr<WhileEvaluator> TryCreate(const ConditionalEvaluation& ce, const CodeBlock& cb, CompilationContext& context);
  std::string GetCode() const override;
 private:
  RValueEvaluator conditional_;
  CodeBlockEvaluator cbe_;
};

class ElseEvaluator {
 public:
  static ErrorOr<ElseEvaluator> TryCreate(const ElseStatement& ee, CompilationContext& context);
  std::string GetCode() const;
 private:
  std::unique_ptr<RValueEvaluator> maybe_conditional_;
  CodeBlockEvaluator cbe_;
  std::unique_ptr<ElseEvaluator> maybe_else_;
};

class IfEvaluator : public StatementEvaluator {
 public:
  static ErrorOr<IfEvaluator> TryCreate(const ConditionalEvaluation& ce, const CodeBlock& cb, const ElseStatement& ee, CompilationContext& context);
  std::string GetCode() const override;
 private:
  RValueEvaluator conditional_;
  CodeBlockEvaluator cbe_;
  std::unique_ptr<ElseEvaluator> maybe_else_;
};

class ReturnEvaluator : public StatementEvaluator {
 public:
  static ErrorOr<ReturnEvaluator> TryCreate(const RValue& rvalue, CompilationContext& context);
  std::string GetCode() const override;
 private:
  RValueEvaluator rve_;
};

class BreakEvaluator : public StatementEvaluator {
 public:
  static ErrorOr<BreakEvaluator> TryCreate(CompilationContext& context);
  std::string GetCode() const override;
};

}  // namespace pbc
