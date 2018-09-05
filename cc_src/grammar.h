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

#include "grammar_piece.h"

namespace pbc {

// Represents a valid PoiBoi file. Just a series of function definitions.
class Module : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new Module);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override { return GrammarLabel::MODULE; }
  const char* DebugDescription() const override {
    return "Zero or more function definitions, eg: functionDefinition1() {} "
           "functionDefinition2() {}";
  }
};

// Function definitions have the function name, argument list, and the code
// which defines the function.
class FunctionDefinition : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new FunctionDefinition);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::FUNCTION_DEFINITION;
  }
  const char* DebugDescription() const override {
    return "A function definition, eg: functionDefinition() {}";
  }
};

// Variable lists are either:
// 1. Empty
// 2. Contain exactly one variable
// 3. Contain multiple variables separated by commas.
class VariablesList : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new VariablesList);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::VARIABLES_LIST;
  }
  const char* DebugDescription() const override {
    return "A list of zero or more variables, eg: a, b, c";
  }
};

class VariablesListExpansion : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new VariablesListExpansion);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::VARIABLES_LIST_EXPANSION;
  }
  const char* DebugDescription() const override {
    return "A variables list, eg: a, b, c";
  }
};

// A code block is all valid code surrounded by {}.
class CodeBlock : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new CodeBlock);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::CODE_BLOCK;
  }
  const char* DebugDescription() const override {
    return "A code block, eg: {RETURN \"foobar\";}";
  }
};

// A StatementList is all valid code, or empty.
class StatementList : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new StatementList);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::STATEMENT_LIST;
  }
  const char* DebugDescription() const override {
    return "A list of zero or more statements, eg: foo = \"bar\"; return foo;";
  }
};

// A statement is variable assignment, function call,
// a while loop, an if/else statement, a return statement.
class Statement : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new Statement);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::STATEMENT;
  }
  const char* DebugDescription() const override {
    return "A valid statement, eg: foo = CONCAT(\"bar\", \"baz\");";
  }
};

// Assigns either a global or local variable to an RValue.
class VariableAssignment : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new VariableAssignment);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::VARIABLE_ASSIGNMENT;
  }
  const char* DebugDescription() const override {
    return "A variable assignment, eg: foo = \"bar\"";
  }
};

// Assigns either a global or local variable to an RValue.
class FunctionCall : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new FunctionCall);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::FUNCTION_CALL;
  }
  const char* DebugDescription() const override {
    return "A function or builtin call, eg: PRINT(\"foobar\")";
  }
};

// For evaluating an if/elif/while statement. Looks like: [RValue].
class ConditionalEvaluator : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new ConditionalEvaluator);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::CONDITIONAL_EVALUATOR;
  }
  const char* DebugDescription() const override {
    return "A conditional evaluator, eg: [\"TRUE\"]";
  }
};

// Can be empty, or an ELSE{}, or an ELSE IF [] {}
class ElseStatement : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new ElseStatement);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::ELSE_STATEMENT;
  }
  const char* DebugDescription() const override {
    return "An else statement, eg: ELIF [\"TRUE\"] {}";
  }
};

// An RValue is something which returns a string. So it's a raw string,
// a variable, or a function call.
class RValue : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new RValue);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::RVALUE;
  }
  const char* DebugDescription() const override {
    return "An RValue- either a string, a variable, or a function call. eg: "
           "\"TRUE\"";
  }
};

// An RValue list is either empty, contains one RValue, or multiple RValues
// separated by ,
class RValueList : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new RValueList);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::RVALUE_LIST;
  }
  const char* DebugDescription() const override {
    return "A list of zero or more RValues- strings, variables, and function "
           "calls. eg: \"TRUE\", EQUAL(\"1\", \"2\"), foo(\"BAR\")";
  }
};

class RValueListExpansion : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new RValueListExpansion);
    CloneGPBase(gp.get());
    return gp;
  }
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::RVALUE_LIST_EXPANSION;
  }
  const char* DebugDescription() const override {
    return "A list of RValues- strings, variables, and function calls. eg: "
           "\"TRUE\", EQUAL(\"1\", \"2\"), foo(\"BAR\")";
  }
};

}  // namespace pbc

#endif  // #ifndef _GRAMMAR_H_
