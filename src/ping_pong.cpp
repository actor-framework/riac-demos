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

#include <cstdlib>
#include <cassert>

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <functional>

#include "cppa/opt.hpp"

#include "caf/all.hpp"
#include "caf/io/all.hpp"
#include "caf/riac/all.hpp"

using namespace std;
using namespace caf;

class ponger : public event_based_actor {
 public:
  behavior make_behavior() override {
    return {
      on(atom("ping")) >> [=] {
        return make_message(atom("pong"));
      },
      on(atom("pong")) >> [=] {
        std::chrono::seconds pause(1);
        this_thread::sleep_for(pause);
        return make_message(atom("pong"));
      },
      on(atom("init"), arg_match) >> [=](const actor& srv) {
        send(srv, atom("ping"));
      }
    };
  } 
 private:
};

std::function<void()> verbose_exit;

template <class T>
void require(const optional<T>& arg, const char* arg_name) {
  if (!arg) {
    cerr << "*** " << arg_name << " missing" << endl;
    verbose_exit();
  }
}

void require(const std::string& arg, const char* arg_name) {
  if (arg.empty()) {
    cerr << "*** " << arg_name << " missing" << endl;
    verbose_exit();
  }
}

template <class T, class V, class... Vs>
void require(T&& a0, const char* a1, V&& a2, const char* a3, Vs&&... as) {
  require(std::forward<T>(a0), a1);
  require(std::forward<V>(a2), a3, std::forward<Vs>(as)...);
}

int main(int argc, char** argv) {
  std::string nexus_host;
  optional<uint16_t> nexus_port;
  bool is_server = false;
  std::string host = "localhost";
  optional<uint16_t> port;
  options_description desc;
  bool args_valid = match_stream<std::string>(argv + 1, argv + argc) (
    on_opt1('N', "caf-nexus-host", &desc, "nexus host") >> rd_arg(nexus_host),
    on_opt1('P', "caf-nexus-port", &desc, "nexus port") >> rd_arg(nexus_port),
    on_opt0('h', "help", &desc, "print help") >> print_desc_and_exit(&desc),
    on_opt1('H', "host", &desc, "calculator server host") >> rd_arg(host),
    on_opt1('p', "port", &desc, "set server port") >> rd_arg(port),
    on_opt0('S', "server", &desc, "run as server") >> set_flag(is_server)
  );
  verbose_exit = print_desc_and_exit(&desc, std::cerr, EXIT_FAILURE);
  if (!args_valid) {
    cerr << "*** invalid command line options" << endl;
    verbose_exit();
  }
  require(port, "port",
          nexus_port, "nexus port",
          nexus_host, "nexus host");
  if (!is_server) {
  }
  if (!riac::init_probe(nexus_host, *nexus_port)) {
    cerr << "probe::init failed" << endl;
    return 1;
  }
  if (is_server) {
    cout << "Starting server on port " << *port << endl;
    try {
      // try to publish math actor at given port
      io::publish(spawn<ponger>(), *port);
    }
    catch (exception& e) {
      cerr << "*** unable to publish math actor at port " << *port << endl
           << to_verbose_string(e) << endl;
      return EXIT_FAILURE;
    }
  } else {
    cout << "Starting client and connecting to "
         << host << " on port " << *port << endl;
    scoped_actor self;
    auto cli = spawn<ponger>();
    auto srvr = io::remote_actor(host, *port);
    self->send(cli, atom("init"), srvr);
  }
  await_all_actors_done();
  shutdown();
}
