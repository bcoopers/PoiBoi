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

// Contains a list of all tokens in PoiBoi, and functions defining what
// sequences of characters satisfy them.

#ifndef POIBOIC_TOKENS_H_
#define POIBOIC_TOKENS_H_

#include <string>

#include "grammar_piece.h"

namespace pbc {

class TokenPiece : public GrammarPiece {
 public:
  virtual ~TokenPiece() {};
  virtual bool Search(char c) = 0;
  virtual size_t GetLength() const = 0;
  virtual bool IsFinalizable() const = 0;

  std::vector<std::vector<std::unique_ptr<GrammarPiece>>>
  GetDescendents() const override { return {}; }
 protected:
  void CloneTPBase(TokenPiece* tp) const {
    CloneGPBase(tp);
  }
};

// A token piece that can be found through simple string matching.
template<typename Derived>
class MatchTokenPiece : public TokenPiece {
 public:
  virtual ~MatchTokenPiece() {};

  bool Search(char c) override {
    if (num_chars_processed_ < GetLength()) {
      hit_error_ = hit_error_ || c != GetContent()[num_chars_processed_];
      if (!hit_error_) {
        num_chars_processed_++;
        return true;
      }
    }
    return false;
  }

  bool IsFinalizable() const override {
    return num_chars_processed_ >= GetLength();
  }

  const char* DebugDescription() const override {
    return GetContent();
  }

  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<MatchTokenPiece<Derived>> gp(new Derived);
    CloneTPBase(gp.get());
    gp->hit_error_ = hit_error_;
    gp->num_chars_processed_ = num_chars_processed_;
    return std::unique_ptr<GrammarPiece>(gp.release());
  }

 private:
  size_t num_chars_processed_ = 0;
  bool hit_error_ = false;
};

// Opens a block of code with {, after an IF, WHILE, or function definition.
class OpenCodeBlock : public MatchTokenPiece<OpenCodeBlock> {
 public:
  size_t GetLength() const override { return 1; }
  const char* GetContent() const override { return "{"; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::OPEN_CODE_BLOCK;
  }
};

// Closes a block of code with }, matches an OpenCodeBlock.
class CloseCodeBlock : public MatchTokenPiece<CloseCodeBlock> {
 public:
  size_t GetLength() const override { return 1; }
  const char* GetContent() const override { return "}"; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::CLOSE_CODE_BLOCK;
  }
};

// Ends most lines of code with ;.
class EndStatement : public MatchTokenPiece<EndStatement> {
 public:
  size_t GetLength() const override { return 1; }
  const char* GetContent() const override { return ";"; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::END_STATEMENT;
  }
};

// Opens the variables list when calling or declaring a function with (.
class OpenFunctionCall : public MatchTokenPiece<OpenFunctionCall> {
 public:
  size_t GetLength() const override { return 1; }
  const char* GetContent() const override { return "("; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::OPEN_FUNCTION_CALL;
  }
};

// Closes the variables list when calling or declaring a function with ).
class CloseFunctionCall : public MatchTokenPiece<CloseFunctionCall> {
 public:
  size_t GetLength() const override { return 1; }
  const char* GetContent() const override { return ")"; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::CLOSE_FUNCTION_CALL;
  }
};

// Separates variables in list when calling or declaring a function with ,.
class ArgumentListSeparator : public MatchTokenPiece<ArgumentListSeparator> {
 public:
  size_t GetLength() const override { return 1; }
  const char* GetContent() const override { return ","; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::ARGUMENT_LIST_SEPARATOR;
  }
};

// Opens the conditional in an IF or WHILE with [.
class OpenConditionalBlock : public MatchTokenPiece<OpenConditionalBlock> {
 public:
  size_t GetLength() const override { return 1; }
  const char* GetContent() const override { return "["; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::OPEN_CONDITIONAL_BLOCK;
  }
};

// Closes OpenConditionalBlock with ].
class CloseConditionalBlock : public MatchTokenPiece<CloseConditionalBlock> {
 public:
  size_t GetLength() const override { return 1; }
  const char* GetContent() const override { return "]"; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::CLOSE_CONDITIONAL_BLOCK;
  }
};

// Assigns an RValue to a variable with =.
class Assigner : public MatchTokenPiece<Assigner> {
 public:
  size_t GetLength() const override { return 1; }
  const char* GetContent() const override { return "="; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::ASSIGNER;
  }
};

// Specifies a variable is a local variable.
// TODO: This will be deprecated.
class KeywordLocal : public MatchTokenPiece<KeywordLocal> {
 public:
  size_t GetLength() const override { return 5; }
  const char* GetContent() const override { return "LOCAL"; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::KEYWORD_LOCAL;
  }
};

// Specifies a variable is a global variable.
class KeywordGlobal : public MatchTokenPiece<KeywordGlobal> {
 public:
  size_t GetLength() const override { return 6; }
  const char* GetContent() const override { return "GLOBAL"; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::KEYWORD_GLOBAL;
  }
};

// Starts a WHILE loop.
class KeywordWhile : public MatchTokenPiece<KeywordWhile> {
 public:
  size_t GetLength() const override { return 5; }
  const char* GetContent() const override { return "WHILE"; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::KEYWORD_WHILE;
  }
};

// Starts an IF statement.
class KeywordIf : public MatchTokenPiece<KeywordIf> {
 public:
  size_t GetLength() const override { return 2; }
  const char* GetContent() const override { return "IF"; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::KEYWORD_IF;
  }
};

// Following an if statement, tarts an ELSE clase.
class KeywordElse : public MatchTokenPiece<KeywordElse> {
 public:
  size_t GetLength() const override { return 4; }
  const char* GetContent() const override { return "ELSE"; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::KEYWORD_ELSE;
  }
};

// Starts following an if statement, starts an "else if" clause.
class KeywordElif : public MatchTokenPiece<KeywordElif> {
 public:
  size_t GetLength() const override { return 4; }
  const char* GetContent() const override { return "ELIF"; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::KEYWORD_ELIF;
  }
};

// Allows early termination from a function.
class KeywordReturn : public MatchTokenPiece<KeywordReturn> {
 public:
  size_t GetLength() const override { return 6; }
  const char* GetContent() const override { return "RETURN"; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::KEYWORD_RETURN;
  }
};

// Allows early termination from a loop.
class KeywordBreak : public MatchTokenPiece<KeywordBreak> {
 public:
  size_t GetLength() const override { return 5; }
  const char* GetContent() const override { return "BREAK"; }
  GrammarLabel GetLabel() const override {
    return GrammarLabel::KEYWORD_BREAK;
  }
};

// Reads a string of anything between and including "", other than newlines.
class QuotedString : public TokenPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<QuotedString> gp(new QuotedString);
    CloneTPBase(gp.get());
    gp->content_ = content_;
    gp->num_backslashes_in_row_ = num_backslashes_in_row_;
    gp->no_more_ = no_more_;
    gp->is_finalized_ = is_finalized_;
    return std::unique_ptr<GrammarPiece>(gp.release());
  }

  bool Search(char c) override;

  const char* GetContent() const override { return content_.c_str(); }

  size_t GetLength() const override { return content_.size(); }

  bool IsFinalizable() const override {
    return is_finalized_ && !content_.empty();
  }

  GrammarLabel GetLabel() const override {
    return GrammarLabel::QUOTED_STRING;
  }

  const char* DebugDescription() const override {
    return "\"A quoted string- like this.\"";
  }

 private:
  std::string content_;
  size_t num_backslashes_in_row_ = 0;
  bool no_more_ = false;
  bool is_finalized_ = false;
};

// Reads in the name of a variable. Must only contain letters and must start
// with a lower case letter.
class Variable : public TokenPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<Variable> gp(new Variable);
    CloneTPBase(gp.get());
    gp->content_ = content_;
    gp->no_more_ = no_more_;
    return std::unique_ptr<GrammarPiece>(gp.release());
  }

  bool Search(char c) override;

  const char* GetContent() const override { return content_.c_str(); }

  size_t GetLength() const override { return content_.size(); }

  bool IsFinalizable() const override { return !content_.empty(); }

  GrammarLabel GetLabel() const override {
    return GrammarLabel::VARIABLE;
  }

  const char* DebugDescription() const override {
    return "aValidVariableName";
  }

 private:
  std::string content_;
  bool no_more_ = false;
};

// Builtin functions are all capital letters.
class Builtin : public TokenPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<Builtin> gp(new Builtin);
    CloneTPBase(gp.get());
    gp->content_ = content_;
    gp->no_more_ = no_more_;
    return std::unique_ptr<GrammarPiece>(gp.release());
  }

  bool Search(char c) override;

  const char* GetContent() const override { return content_.c_str(); }

  size_t GetLength() const override { return content_.size(); }

  bool IsFinalizable() const override { return !content_.empty(); }

  GrammarLabel GetLabel() const override {
    return GrammarLabel::BUILTIN;
  }

  const char* DebugDescription() const override {
    return "AVALIDBUILTINNAME";
  }

 private:
  std::string content_;
  bool no_more_ = false;
};

// FunctionNames are all capital and lower case letters. Must start with a
// capital letter, and contain a lower case letter.
class FunctionName : public TokenPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<FunctionName> gp(new FunctionName);
    CloneTPBase(gp.get());
    gp->content_ = content_;
    gp->no_more_ = no_more_;
    gp->contains_lower_case_ = contains_lower_case_;
    return std::unique_ptr<GrammarPiece>(gp.release());
  }

  bool Search(char c) override;

  const char* GetContent() const override { return content_.c_str(); }

  size_t GetLength() const override { return content_.size(); }

  bool IsFinalizable() const override {
    return !content_.empty() && contains_lower_case_;
  }

  GrammarLabel GetLabel() const override {
    return GrammarLabel::FUNCTION_NAME;
  }

  const char* DebugDescription() const override {
    return "AValidFunctionName";
  }

 private:
  std::string content_;
  bool no_more_ = false;
  bool contains_lower_case_ = false;
};

// Marks the end of the file. Is artificially added at the end if everything
// is scanned correctly.
class EndOfFile : public TokenPiece {
 public:
  std::unique_ptr<GrammarPiece> Clone() const override {
    std::unique_ptr<EndOfFile> gp(new EndOfFile);
    CloneTPBase(gp.get());
    return std::unique_ptr<GrammarPiece>(gp.release());
  }

  bool Search(char c) override { return false; }

  const char* GetContent() const override { return ""; }

  size_t GetLength() const override { return 0; }

  bool IsFinalizable() const override { return true; }

  const char* DebugDescription() const override {
    return "The end of the file";
  }

  GrammarLabel GetLabel() const override {
    return GrammarLabel::END_OF_FILE;
  }
};

}  // namespace pbc

#endif  // #ifndef POIBOIC_TOKENS_H_
