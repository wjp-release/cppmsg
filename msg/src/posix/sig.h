#pragma once

namespace msg{namespace posix{

class event;
event* create_signalfd_event();

}}