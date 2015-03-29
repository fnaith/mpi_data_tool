#include <boost/tr1/unordered_map.hpp>

namespace mdt_core {

#if (BOOST_VERSION % 100) > 55
using boost::unordered_map;
#else
using std::unordered_map;
#endif

}
