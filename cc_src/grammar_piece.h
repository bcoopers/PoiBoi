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

// Defines the interface that all pieces of the PoiBoi grammar must satisfy.

#ifndef _GRAMMAR_PIECE_H_
#define _GRAMMAR_PIECE_H_

#include <memory>
#include <string>
#include <vector>

namespace pbc {

class GrammarPiece {
 public:
  virtual ~GrammarPiece() {}
  virtual std::vector<std::unique_ptr<GrammarPiece>> GetDescendents() const = 0;

  size_t line_number() const { return line_number_; }
  void set_line_number(size_t line_num) { line_number_ = line_num; }

  const std::string& file_name() const { return file_name_; }
  void set_file_name(std::string fname) { file_name_ = std::move(fname); }

 protected:
  size_t line_number_ = 0;
  std::string file_name_;
};

}  // namespace pbc

#endif  // #ifndef _GRAMMAR_PIECE_H_
