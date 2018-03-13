#include "Events.h"

#define DEFINE_EVENT(name) const char * const name = #name

namespace CppRed{
namespace Scripts{

DEFINE_EVENT(event_received_starter);
DEFINE_EVENT(event_got_pokeballs_from_oak);
DEFINE_EVENT(event_pallet_after_getting_pokeballs);
DEFINE_EVENT(event_followed_oak_into_lab);
DEFINE_EVENT(event_oak_appeared_in_pallet);

}
}
