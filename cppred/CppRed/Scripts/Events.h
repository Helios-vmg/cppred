#pragma once

#define DECLARE_EVENT(name) extern const char * const name

namespace CppRed{
namespace Scripts{

DECLARE_EVENT(event_received_starter);
DECLARE_EVENT(event_got_pokeballs_from_oak);
DECLARE_EVENT(event_pallet_after_getting_pokeballs);
DECLARE_EVENT(event_followed_oak_into_lab);
DECLARE_EVENT(event_oak_appeared_in_pallet);

}
}
