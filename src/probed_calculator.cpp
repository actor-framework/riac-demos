/******************************************************************************\
 * This program is a distributed version of the math_actor example.           *
 * Client and server use a stateless request/response protocol and the client *
 * is failure resilient by using a FIFO request queue.                        *
 * The client auto-reconnects and also allows for server reconfiguration.     *
 *                                                                            *
 * Run server at port 4242:                                                   *
 * - ./build/bin/distributed_calculator -s -p 4242                            *
 *                                                                            *
 * Run client at the same host:                                               *
 * - ./build/bin/distributed_calculator -c -p 4242                            *
\ ******************************************************************************/

#include <vector>
#include <string>
#include <sstream>
#include <cassert>
#include <iostream>
#include <functional>

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/probe/all.hpp"
#include "caf/probe_event/all.hpp"

// the options_description API is deprecated
#include "cppa/opt.hpp"


using namespace std;
using namespace caf;

// our "service"
void calculator(event_based_actor* self) {
  self->become (
    on(atom("plus"), arg_match) >> [](int a, int b) -> message {
      return make_message(atom("result"), a + b);
    },
    on(atom("minus"), arg_match) >> [](int a, int b) -> message {
      return make_message(atom("result"), a - b);
    },
    on(atom("quit")) >> [=] {
      self->quit();
    }
  );
}

void seeker(event_based_actor* self, const string& host, uint16_t port) {
  auto srvr = io::remote_actor(host, port);
  self->sync_send(srvr, atom("plus"), 21, 21).then(
    on(atom("result"), arg_match) >> [=] (int result) {
      cout << "seeker found the answer: " << result << endl;
    }
  );
}

void spawner(event_based_actor* self, const string& host, uint16_t port) {
  self->become(
    on(atom("spawn")) >> [=] {
      // self->send(srvr, atom("plus"), 21, 21);
      spawn(seeker, host, port);
      self->delayed_send(self, chrono::seconds(1), atom("spawn"));
    }
  );
}

// ##### command line argument stuff #####

struct calc_config {
  uint16_t port;
  std::string host;
  bool srvr;
  inline calc_config() : port(0), srvr(false) { }
  inline bool valid() const {
    return port != 0;
  }
};

const char host_arg[] = "--host=";
const char port_arg[] = "--port=";
const char srvr_arg[] = "--server";

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

void from_args(calc_config& conf, int argc, char** argv) {
  for (auto i = argv; i != argv + argc; ++i) {
    if (is_substr(host_arg, *i)) {
      conf.host.assign(*i + cstr_len(host_arg));
    } else if (is_substr(port_arg, *i)) {
      int p = std::stoi(*i + cstr_len(port_arg));
      conf.port = static_cast<uint16_t>(p);
    } else if (is_substr(srvr_arg, *i)) {
      conf.srvr=true;
    }
  }
}

// ##### main #####

int main(int argc, char** argv) {
  probe_event::announce_types();
  probe::init(argc,argv);
  calc_config conf;
  from_args(conf, argc, argv);
  if (!conf.valid()) {
    cerr << "Requires port (host defaults to 'localhost')\n"
            "  supported: '--host=','--port=' and '--server'\n"
            "  connect to nexus: '--caf_probe_host=' and '--caf_probe_port='"
         << endl;
    return 1;
  }
  if (conf.srvr) {
    cout << "Starting server on "
         << "localhost:" << conf.port << endl;

    try {
      // try to publish math actor at given port
      io::publish(spawn(calculator), conf.port);
    }
    catch (exception& e) {
      cerr << "*** unable to publish math actor at port " << conf.port << "\n"
         << to_verbose_string(e) // prints exception type and e.what()
         << endl;
    }
  } else {
    cout << "Starting client and connecting to "
         << conf.host << ":" << conf.port << endl;
    if (conf.host.empty()) {
      conf.host.assign("localhost");
    }
    scoped_actor self;
    //auto client_spawner = spawn(spawner, conf.host, conf.port);
    //self->delayed_send(client_spawner, chrono::seconds(1), atom("spawn"));
    auto srvr = io::remote_actor(conf.host, conf.port);
    self->sync_send(srvr, atom("plus"), 29, 13).await(
      on(atom("result"), arg_match) >> [=] (int result) {
        cout << "seeker found the answer: " << result << endl;
      }
    );
  }
  await_all_actors_done();
  shutdown();
  return 0;
}
