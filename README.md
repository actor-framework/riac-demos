Probe Examples
==============

Example application to test probe, nexus and shell.
Requires libcaf with the components probe, probe-event, nexus and shell.

Build
-----

 ```
./configure
 make
 ```


Start
-----

You need to start a nexus fist.
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

With the following submodules

* `libcaf_probe`
* `libcaf_probe_event`
* `nexus`
* `shell`

