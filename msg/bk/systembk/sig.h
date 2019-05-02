#pragma once

namespace msg{

class event;
event* create_signalfd_event(int epollfd);

}