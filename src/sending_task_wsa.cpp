#ifdef _WIN32

#include "sending_task_wsa.h"
#include <iostream>

namespace networking{
namespace udp{

template struct SendingTask<bool>;

}
}


#endif

