#pragma once

#include <string>

namespace msg{

/*
Failure : It is the inability of a system or component to perform required function according to its specification. 

Fault : It is a condition that causes the software to fail to perform its required function. 

Error : Refers to difference between Actual Output and Expected output. 


IEEE Definitions
Failure: External behavior is incorrect
Fault: Discrepancy in code that causes a failure.
Error: Human mistake that caused fault
*/

class status {
public:

    // Create a success status.
    status() : state_(nullptr) {}
    ~status() { delete[] state_; }

    // Copy the specified status.
    status(const status& s);
    void operator=(const status& s);

    // Return a success status.
    static status OK() { return status(); }
    // A fault happens when system behavior violates its specification. It means the code is 
    static status Fault(const std::string& msg, const std::string& msg2 = std::string()) {
        return status(StatusFault, msg, msg2);
    }
    static status InvalidArgument(const std::string& msg, const std::string& msg2 = std::string()) {
        return status(kInvalidArgument, msg, msg2);
    }
    static status IOError(const std::string& msg, const std::string& msg2 = std::string()) {
    return status(kIOError, msg, msg2);
    }

  // Returns true iff the status indicates success.
  bool ok() const { return (state_ == NULL); }

  // Returns true iff the status indicates an InvalidArgument.
  bool IsInvalidArgument() const { return code() == kInvalidArgument; }

  // Return a string representation of this status suitable for printing.
  // Returns the string "OK" for success.
  std::string str() const;

 private:
  // OK status has a NULL state_.  Otherwise, state_ is a new[] array
  // of the following form:
  //    state_[0..3] == length of message
  //    state_[4]    == code
  //    state_[5..]  == message
  const char* state_;

  enum Code {
    StatusOK = 0,
    StatusFault = 1,
    StatusFailure = 2,
    StatusNotSupportedYet = 3,
    StatusInvalidInput = 4,
  };

  Code code() const {
    return (state_ == NULL) ? StatusOK : static_cast<Code>(state_[4]);
  }

  status(Code code, const std::string& msg, const std::string& msg2);
  static const char* CopyState(const char* s);
};

inline status::status(const status& s) {
  state_ = (s.state_ == NULL) ? NULL : CopyState(s.state_);
}
inline void status::operator=(const status& s) {
  // The following condition catches both aliasing (when this == &s),
  // and the common case where both s and *this are ok.
  if (state_ != s.state_) {
    delete[] state_;
    state_ = (s.state_ == NULL) ? NULL : CopyState(s.state_);
  }
}

}