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

// Defines the full expansion of the PoiBoi grammar. Everything terminates
// as tokens.

#ifndef POIBOIC_GRAMMAR_H_
#define POIBOIC_GRAMMAR_H_

#include <memory>
#include <string>
#include <vector>

#include "grammar_piece.h"

namespace pbc {

template<typename Descendent>
class ExpandableGrammarPiece : public GrammarPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<GrammarPiece> gp(new Descendent);
    CloneGPBase(gp.get());
    return gp;
  }
};

// Represents a valid PoiBoi file. Just a series of function definitions.
class Module : public ExpandableGrammarPiece<Module> {
 public:
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override { return GrammarLabel::MODULE; }
  const char* DebugDescription() const override {
    return "Zero or more function definitions, eg: functionDefinition1() {} "
           "functionDefinition2() {}";
  }
};

// Function definitions have the function name, argument list, and the code
// which defines the function.
class FunctionDefinition : public ExpandableGrammarPiece<FunctionDefinition> {
 public:
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
class VariablesList : public ExpandableGrammarPiece<VariablesList> {
 public:
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::VARIABLES_LIST;
  }
  const char* DebugDescription() const override {
    return "A list of zero or more variables, eg: a, b, c";
  }
};

class VariablesListExpansion
      : public ExpandableGrammarPiece<VariablesListExpansion> {
 public:
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::VARIABLES_LIST_EXPANSION;
  }
  const char* DebugDescription() const override {
    return "A variables list, eg: a, b, c";
  }
};

// A code block is all valid code surrounded by {}.
class CodeBlock : public ExpandableGrammarPiece<CodeBlock> {
 public:
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::CODE_BLOCK;
  }
  const char* DebugDescription() const override {
    return "A code block, eg: {RETURN \"foobar\";}";
  }
};

// A StatementList is all valid code, or empty.
class StatementList : public ExpandableGrammarPiece<StatementList> {
 public:
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
class Statement : public ExpandableGrammarPiece<Statement> {
 public:
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::STATEMENT;
  }
  const char* DebugDescription() const override {
    return "A valid statement, eg: foo = CONCAT(\"bar\", \"baz\");";
  }
};

// Assigns either a global or local variable to an RValue.
class VariableAssignment : public ExpandableGrammarPiece<VariableAssignment> {
 public:
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::VARIABLE_ASSIGNMENT;
  }
  const char* DebugDescription() const override {
    return "A variable assignment, eg: foo = \"bar\"";
  }
};

// Assigns either a global or local variable to an RValue.
class FunctionCall : public ExpandableGrammarPiece<FunctionCall> {
 public:
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::FUNCTION_CALL;
  }
  const char* DebugDescription() const override {
    return "A function or builtin call, eg: PRINT(\"foobar\")";
  }
};

// For evaluating an if/elif/while statement. Looks like: [RValue].
class ConditionalEvaluation
      : public ExpandableGrammarPiece<ConditionalEvaluation> {
 public:
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::CONDITIONAL_EVALUATOR;
  }
  const char* DebugDescription() const override {
    return "A conditional evaluator, eg: [\"TRUE\"]";
  }
};

// Can be empty, or an ELSE{}, or an ELSE IF [] {}
class ElseStatement : public ExpandableGrammarPiece<ElseStatement> {
 public:
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
class RValue : public ExpandableGrammarPiece<RValue> {
 public:
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
class RValueList : public ExpandableGrammarPiece<RValueList> {
 public:
  Descendents GetDescendents() const override;
  GrammarLabel GetLabel() const override {
    return GrammarLabel::RVALUE_LIST;
  }
  const char* DebugDescription() const override {
    return "A list of zero or more RValues- strings, variables, and function "
           "calls. eg: \"TRUE\", EQUAL(\"1\", \"2\"), foo(\"BAR\")";
  }
};

class RValueListExpansion : public ExpandableGrammarPiece<RValueListExpansion> {
 public:
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

#endif  // #ifndef POIBOIC_GRAMMAR_H_
