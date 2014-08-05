
#include <iostream>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/probe_event/all.hpp"

using std::cout;
using std::cerr;
using std::endl;

using namespace caf;

struct probe_config {
  uint16_t port;
  std::string host;
  std::string config_file_path;
  inline probe_config() : port(0) { } 
  inline bool valid() const {
    return !host.empty() && port != 0;
  }
};

//const char conf_file_arg[] = "--caf_probe_config_file=";
const char host_arg[] = "--host=";
const char port_arg[] = "--port=";

template<size_t Size>
bool is_substr(const char (&needle)[Size], const char* haystack) {
  // compare without null terminator
  if (strncmp(needle, haystack, Size - 1) == 0) {
    return true;
  }
  return false;
}

template<size_t Size>
size_t cstr_len(const char (&)[Size]) {
  return Size - 1;
}

void from_args(probe_config& conf, int argc, char** argv) {
  for (auto i = argv; i != argv + argc; ++i) {
    if (is_substr(host_arg, *i)) {
      conf.host.assign(*i + cstr_len(host_arg));
    } else if (is_substr(port_arg, *i)) {
      int p = std::stoi(*i + cstr_len(port_arg));
      conf.port = static_cast<uint16_t>(p);
    }   
  }
}

void printer(event_based_actor* self, const probe_event::nexus_type& nex) {
  self->send(nex, probe_event::add_listener{self});
  self->become(
    others() >> [] {
      cout << "new message" << endl;
    }
  );
}

int main(int argc, char** argv) {
  probe_event::announce_types();
  probe_config conf;
  from_args(conf, argc, argv);
  if (!conf.valid()) {
    cerr << "port and host required (can be set with '--host=' and '--port=')" 
         << endl;
    return 1;
  }
  auto nex = io::typed_remote_actor<probe_event::nexus_type>(conf.host,
                                                             conf.port);
  //auto nex = io::remote_actor(conf.host, conf.port);
  auto ver = spawn(printer, nex);
  await_all_actors_done();
  shutdown();
  return 0;
}


