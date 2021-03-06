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

#ifndef POIBOIC_SCANNER_H_
#define POIBOIC_SCANNER_H_

#include "error_code.h"
#include "tokens.h"

namespace pbc {

// Scans the code and outputs all the tokens. Returns false on failure.
ErrorCode ScanTokens(const std::string& code,
                std::vector<std::unique_ptr<TokenPiece>>& tokens);

}  // namespace pbc

#endif  // #ifndef POIBOIC_SCANNER_H_
