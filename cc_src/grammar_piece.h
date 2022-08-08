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

// Defines the interface that all pieces of the PoiBoi grammar must satisfy.

#ifndef POIBOIC_GRAMMAR_PIECE_H_
#define POIBOIC_GRAMMAR_PIECE_H_

#include <memory>
#include <string>
#include <vector>

namespace pbc {

enum class GrammarLabel {
  // Tokens.
  OPEN_CODE_BLOCK,
  CLOSE_CODE_BLOCK,
  END_STATEMENT,
  OPEN_FUNCTION_CALL,
  CLOSE_FUNCTION_CALL,
  ARGUMENT_LIST_SEPARATOR,
  OPEN_CONDITIONAL_BLOCK,
  CLOSE_CONDITIONAL_BLOCK,
  ASSIGNER,
  KEYWORD_GLOBAL,
  KEYWORD_WHILE,
  KEYWORD_IF,
  KEYWORD_ELSE,
  KEYWORD_ELIF,
  KEYWORD_RETURN,
  KEYWORD_BREAK,
  QUOTED_STRING,
  VARIABLE,
  BUILTIN,
  FUNCTION_NAME,
  END_OF_FILE,
  // Expandable grammar pieces.
  MODULE,
  FUNCTION_DEFINITION,
  VARIABLES_LIST,
  VARIABLES_LIST_EXPANSION,
  CODE_BLOCK,
  STATEMENT_LIST,
  STATEMENT,
  VARIABLE_ASSIGNMENT,
  GLOBAL_DECLARATION,
  FUNCTION_CALL,
  CONDITIONAL_EVALUATOR,
  ELSE_STATEMENT,
  RVALUE,
  RVALUE_LIST,
  RVALUE_LIST_EXPANSION,
};

class GrammarPiece {
 public:
  using Children = std::vector<std::unique_ptr<GrammarPiece>>;
  using Descendents = std::vector<Children>;

  virtual ~GrammarPiece() {}
  GrammarPiece() = default;
  GrammarPiece(const GrammarPiece& other) {
    other.CloneGPBase(this);
  }
  GrammarPiece& operator=(const GrammarPiece& other) {
    other.CloneGPBase(this);
    return *this;
  }
  GrammarPiece(GrammarPiece&& other) = default;
  GrammarPiece& operator=(GrammarPiece&& other) = default;

  virtual std::vector<std::vector<std::unique_ptr<GrammarPiece>>>
  GetDescendents() const = 0;

  virtual std::unique_ptr<GrammarPiece> Clone() const = 0;

  virtual GrammarLabel GetLabel() const = 0;

  bool IsToken() const {
    return GetLabel() < GrammarLabel::MODULE;
  }

  virtual const char* GetContent() const {
    assert(false);
    return nullptr;
  }

  virtual const char* DebugDescription() const = 0;

  const std::vector<std::unique_ptr<GrammarPiece>>& GetChildren() const {
    return children_;
  }

  void MoveIntoChildren(
    std::vector<std::unique_ptr<GrammarPiece>>& new_children) {
    using ChildT = std::vector<std::unique_ptr<GrammarPiece>>::iterator;
    children_.insert(
        children_.end(),
        std::move_iterator<ChildT>(new_children.begin()),
        std::move_iterator<ChildT>(new_children.end()));
    new_children.clear();
  }

  size_t line_number() const { return line_number_; }
  void set_line_number(size_t line_num) { line_number_ = line_num; }

  const std::string& file_name() const { return file_name_; }
  void set_file_name(std::string fname) { file_name_ = std::move(fname); }

 protected:
  void CloneGPBase(GrammarPiece* new_gp) const {
    new_gp->line_number_ = line_number_;
    new_gp->file_name_ = file_name_;
    new_gp->children_.clear();
    for (const auto& child : children_) {
      new_gp->children_.push_back(child->Clone());
    }
  }

  size_t line_number_ = 0;
  std::string file_name_;
  std::vector<std::unique_ptr<GrammarPiece>> children_;
};

}  // namespace pbc

#endif  // #ifndef POIBOIC_GRAMMAR_PIECE_H_
