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

// Contains a list of all tokens in PoiBoi, and functions defining what
// sequences of characters satisfy them.

#ifndef _TOKENS_H_
#define _TOKENS_H_

#include <string>

#include "grammar_piece.h"

namespace pbc {

class TokenPiece : public GrammarPiece {
 public:
  virtual ~TokenPiece() {};
  virtual bool Search(char c) = 0;
  virtual const char* GetContent() const = 0;
  virtual bool IsFinalizable() const = 0;

  std::vector<std::unique_ptr<GrammarPiece>> GetDescendents() const override {
    return {};
  }
};

// A token piece that can be found through simple string matching.
class MatchTokenPiece : public TokenPiece {
 public:
  virtual ~MatchTokenPiece() {};

  bool Search(char c) override {
    return num_chars_processed_ < length_ &&
           c == GetContent()[num_chars_processed_++];
  }

  bool IsFinalizable() const override {
    return num_chars_processed_ >= length_;
  }

 private:
  size_t num_chars_processed_ = 0;
  size_t length_ = strlen(GetContent());
};

// Opens a block of code with {, after an IF, WHILE, or function definition.
class OpenCodeBlock : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return "{"; }
};

// Closes a block of code with }, matches an OpenCodeBlock.
class CloseCodeBlock : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return "}"; }
};

// Ends most lines of code with ;.
class EndStatement : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return ";"; }
};

// Opens the variables list when calling or declaring a function with (.
class OpenFunctionCall : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return "("; }
};

// Closes the variables list when calling or declaring a function with ).
class CloseFunctionCall : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return ")"; }
};

// Separates variables in list when calling or declaring a function with ,.
class ArgumentListSeparator : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return ","; }
};

// Opens the conditional in an IF or WHILE with [.
class OpenConditionalBlock : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return "["; }
};

// Closes OpenConditionalBlock with ].
class CloseConditionalBlock : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return "]"; }
};

// Assigns an RValue to a variable with =.
class Assigner : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return "="; }
};

// Specifies a variable is a local variable.
class KeywordLocal : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return "LOCAL"; }
};

// Starts a WHILE loop.
class KeywordWhile : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return "WHILE"; }
};

// Starts an IF statement.
class KeywordIf : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return "IF"; }
};

// Following an if statement, tarts an ELSE clase.
class KeywordElse : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return "ELSE"; }
};

// Starts following an if statement, starts an "else if" clause.
class KeywordElif : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return "ELIF"; }
};

// Allows early termination from a function.
class KeywordReturn : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return "RETURN"; }
};

// Allows early termination from a loop.
class KeywordBreak : public MatchTokenPiece {
 public:
  const char* GetContent() const override { return "BREAK"; }
};

// Reads a string of anything between and including "", other than newlines.
class QuotedString : public TokenPiece {
 public:
  bool Search(char c) override;

  const char* GetContent() const override { return content_.c_str(); }

  bool IsFinalizable() const override {
    return is_finalized_ && !content_.empty();
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
  bool Search(char c) override;

  const char* GetContent() const override { return content_.c_str(); }

  bool IsFinalizable() const override { return !content_.empty(); }
 private:
  std::string content_;
  bool no_more_ = false;
};

// Builtin functions are all capital letters.
class Builtin : public TokenPiece {
 public:
  bool Search(char c) override;

  const char* GetContent() const override { return content_.c_str(); }

  bool IsFinalizable() const override { return !content_.empty(); }

 private:
  std::string content_;
  bool no_more_ = false;
};

// FunctionNames are all capital and lower case letters. Must start with a
// capital letter, and contain a lower case letter.
class FunctionName : public TokenPiece {
 public:
  bool Search(char c) override;

  const char* GetContent() const override { return content_.c_str(); }

  bool IsFinalizable() const override {
    return !content_.empty() && contains_lower_case_;
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
  bool Search(char c) override { return false; }

  const char* GetContent() const override { return ""; }

  bool IsFinalizable() const override { return true; }
};

}  // namespace pbc

#endif  // #ifndef _TOKENS_H_
