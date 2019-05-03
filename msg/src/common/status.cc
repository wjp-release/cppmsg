#include "status.h"
#include "def.h"

namespace msg{

const char* status::CopyState(const char* state) {
    uint32_t size = 9+strlen(state+9);
    char* result = new char[size];
    memcpy(result, state, size);
    return result;
}

//0~3 msglen, 4: code, 5+: msg
status::status(Code code, const std::string& msg, uint64_t value) {
    assert(code != StatusSuccess);
    const uint32_t size = msg.size();
    char* result = new char[size + 10];
    memcpy(result, &value, sizeof(value));
    result[8] = static_cast<char>(code);
    memcpy(result + 9, msg.data(), size);
    result[9+size]='\0';
    state_=result;
}

std::string status::str() const {
  if (state_ == NULL) {
    return "OK";
  } else {
    char tmp[30];
    const char* type;
    switch (code()) {
      case StatusSuccess:
        type = "Success";
        break;
      case StatusError:
        type = "Human mistake: ";
        break;
      case StatusFault:
        type = "Discrepancy in code: ";
        break;
      case StatusFailure:
        type = "External behavior is incorrect: ";
        break;
      case StatusUnsupported:
        type = "Not supported yet: ";
        break;
      default:
        snprintf(tmp, sizeof(tmp), "Unknown code(%d): ",
                 static_cast<int>(code()));
        type = tmp;
        break;
    }
    std::string result(type);
    uint32_t length=msglen();
    result.append(state_ + 9, length);
    return result;
  }
}

}