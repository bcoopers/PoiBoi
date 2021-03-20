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

#ifndef POIBOIC_ERROR_CODE_H_
#define POIBOIC_ERROR_CODE_H_

#include <optional>
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

template<typename T>
class ErrorOr {
 public:
  ErrorOr(T t) {
    item_ = std::move(t);
  }

  static ErrorOr Failure(std::string msg) {
    ErrorOr e;
    e.error_msg_ = std::move(msg);
    return e;
  }

  bool IsSuccess() const { return item_.has_value(); }

  bool IsFailure() const { return !IsSuccess(); }

  T& GetItem() { return item_.get(); }

  const T& GetItem() const { return item_.get(); }

  const std::string& ErrorMessage() const { return error_msg_; }

 private:
  std::optional<T> item_;
  std::string error_msg_;
};

#define RETURN_EC_IF_FAILURE(ec) {  \
  const ErrorCode local_ec_x20x20x = (ec); \
  if (local_ec_x20x20x.IsFailure()) { \
    return local_ec_x20x20x; \
  } \
}

}  // namespace pbc

#endif  // #ifndef POIBOIC_ERROR_CODE_H_
