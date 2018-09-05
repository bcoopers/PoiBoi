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

#ifndef _ERROR_CODE_H_
#define _ERROR_CODE_H_

#include <string>

namespace pbc {

class ErrorCode {
 public:
  static ErrorCode Success() {
    ErrorCode ec;
    ec.success_ = true;
    return ec;
  }

  static ErrorCode Failure(std::string msg) {
    ErrorCode ec;
    ec.success_ = false;
    ec.error_msg_ = std::move(msg);
    return ec;
  }

  bool IsSuccess() const { return success_; }

  bool IsFailure() const { return !success_; }

  const std::string& ErrorMessage() const { return error_msg_; }


 private:
  ErrorCode() {}
  bool success_ = false;
  std::string error_msg_;
};

#define RETURN_EC_IF_FAILURE(ec) {  \
  const ErrorCode local_ec = ec; \
  if (local_ec.IsFailure()) { \
    return local_ec; \
  } \
}

}  // namespace pbc

#endif  // #ifndef _ERROR_CODE_H_
