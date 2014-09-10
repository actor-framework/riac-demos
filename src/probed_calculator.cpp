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

// our "service"
void calculator(event_based_actor* self) {
  self->become (
    on(atom("plus"), arg_match) >> [](int a, int b) -> message {
      return make_message(atom("result"), a + b);
    },
    on(atom("minus"), arg_match) >> [](int a, int b) -> message {
      return make_message(atom("result"), a - b);
    }
  );
}

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
  optional<int> x;
  optional<int> y;
  bool args_valid = match_stream<std::string>(argv + 1, argv + argc) (
    on_opt1('N', "caf-nexus-host", &desc, "nexus host") >> rd_arg(nexus_host),
    on_opt1('P', "caf-nexus-port", &desc, "nexus port") >> rd_arg(nexus_port),
    on_opt0('h', "help", &desc, "print help") >> print_desc_and_exit(&desc),
    on_opt1('H', "host", &desc, "calculator server host") >> rd_arg(host),
    on_opt1('p', "port", &desc, "set server port") >> rd_arg(port),
    on_opt0('S', "server", &desc, "run as server") >> set_flag(is_server),
    on_opt1('x', "x-value", &desc, "set X value", "client options") >> rd_arg(x),
    on_opt1('y', "y-value", &desc, "set Y value", "client options") >> rd_arg(y)
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
    require(x, "x", y, "y");
  }
  if (!riac::init_probe(nexus_host, *nexus_port)) {
    cerr << "probe::init failed" << endl;
    return 1;
  }
  if (is_server) {
    cout << "Starting server on port " << *port << endl;
    try {
      // try to publish math actor at given port
      io::publish(spawn(calculator), *port);
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
    auto srvr = io::remote_actor(host, *port);
    self->sync_send(srvr, atom("plus"), *x, *y).await(
      on(atom("result"), arg_match) >> [&] (int result) {
        cout << *x << " + " << *y << " = " << result << endl;
      }
    );
  }
  await_all_actors_done();
  shutdown();
}
