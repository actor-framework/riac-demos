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
        chrono::seconds pause(1);
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

int main(int argc, char** argv) {
  string nexus_host = "localhost";
  uint16_t nexus_port = 1345;
  bool is_server = false;
  string host = "localhost";
  uint16_t port = 1897;
  auto res = message_builder(argv + 1, argv + argc).extract_opts({
    {"caf-nexus-host,N", "nexus host (DEFAULT: localhost)", nexus_host},
    {"caf-nexus-port,P", "nextus port (DEFAULT: 1345)", nexus_port},
    {"help,h", "print help"},
    {"host,H", "calculator server host (DEFAULT: localhost)", host},
    {"port,p", "calculator server port (DEFAULT: 1897)", port},
    {"server,S", "run as server", is_server}
  });
  if (res.opts.count("help") > 0) {
    cerr << res.helptext << endl;
  }
  if (!res.error.empty()) {
    cerr << "*** invalid command line options" << endl;
    cerr << res.error << endl;
    cerr << res.helptext << endl;
  }
  if (!riac::init_probe(nexus_host, nexus_port)) {
    cerr << "probe::init failed" << endl;
    return 1;
  }
  if (is_server) {
    cout << "Starting server on port " << port << endl;
    try {
      // try to publish math actor at given port
      io::publish(spawn<ponger>(), port);
    }
    catch (exception& e) {
      cerr << "*** unable to publish math actor at port " << port << endl
           << to_verbose_string(e) << endl;
      return EXIT_FAILURE;
    }
  } else {
    cout << "Starting client and connecting to "
         << host << " on port " << port << endl;
    scoped_actor self;
    auto cli = spawn<ponger>();
    auto srvr = io::remote_actor(host, port);
    self->send(cli, atom("init"), srvr);
  }
  await_all_actors_done();
  shutdown();
}
