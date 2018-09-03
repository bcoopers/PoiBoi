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

#ifndef _SCANNER_H_
#define _SCANNER_H_

#include "tokens.h"

namespace pbc {

// Scans the code and outputs all the tokens. Returns false on failure.
bool ScanTokens(const std::string& code,
                std::vector<std::unique_ptr<TokenPiece>>& tokens);

}  // namespace pbc

#endif  // #ifndef _SCANNER_H_
