Probe Examples
==============

Example application to test probe, nexus and shell. Requires libcaf with the components libcaf_riac, nexus and shell. Implemented examples so far:
* `probed_calculator`
* `ping_pong`

Build
-----
 ```
./configure
 make
 ```

Start
-----
To start `probed_calculator`, you need to start a nexus fist.
It is included in CAF and per default located in the directory `build/bin`:
 ```
 ./build/bin/nexus <nexus-port>
 ```

Next, start the `probed_calculator`:
```
./build/probed_calculator --server --port=<calculator-port> --caf-nexus-host=<nexus-host> --caf-nexus-port=<nexus-port>
```

Usage
-----
You can start clients and also interact with the running system using CAF's shell. To start the calculator as client and add two numbers (e.g. 10 + 20), run:

```
./build/probed_calculator --host=<calculator-host> --port=<calculator-port> --caf-nexus-host=<nexus-host> --caf-nexus-port=<nexus-port> -x 10 -y 20
```

Dependencies
------------
* https://github.com/actor-framework/actor-framework

Please have a look in actor-frameworks [wiki](https://github.com/actor-framework/actor-framework/wiki).
With the following submodules:
* `libcaf_riac`
* `nexus`
* `shell`

