#pragma once
#ifndef SUBARULINK_EXCEPTIONS_HPP
#define SUBARULINK_EXCEPTIONS_HPP

#include <stdexcept>
#include <string>

namespace subarulink {

  class SubaruException : public std::runtime_error {
  public:
    explicit SubaruException(const std::string& message) : std::runtime_error(message), _message(message) {}
    const std::string& message() const { return _message; }
  private:
    std::string _message;
  };

  class InvalidPIN : public SubaruException {
  public:
    explicit InvalidPIN(const std::string& message) : SubaruException(message) {}
  };

  class IncompleteCredentials : public SubaruException {
  public:
    explicit IncompleteCredentials(const std::string& message) : SubaruException(message) {}
  };

  class InvalidCredentials : public SubaruException {
  public:
    explicit InvalidCredentials(const std::string& message) : SubaruException(message) {}
  };

  class PINLockoutProtect : public SubaruException {
  public:
    explicit PINLockoutProtect(const std::string& message) : SubaruException(message) {}
  };

  class VehicleNotSupported : public SubaruException {
  public:
    explicit VehicleNotSupported(const std::string& message) : SubaruException(message) {}
  };

  class RemoteServiceFailure : public SubaruException {
  public:
    explicit RemoteServiceFailure(const std::string& message) : SubaruException(message) {}
  };

} // namespace subarulink

#endif // SUBARULINK_EXCEPTIONS_HPP