agere14 examples
================

Example application to test the CAF probe, nexus and shell.
Requires libcaf installed with the components probe, probe-event, nexus and shell.

Build
-----

 ```
./configure
 make
 ```


Start
----

You need to start a nexus fist.
It is included in CAF (start it from the CAF root with)
 ```
 ./build/bin/nexus <nexus-port>
 ```

Start the  ```verbose_probe``` from this repo.
Currently this will only tell when it receives a message from the nexus.
Once the shell is running, it should be used instead.
 ```
 ./build/verbose_probe --host=<nexus-host> --port=<nexus-port>
 ```

The ```probed_calculator``` will spawn new actor and sent messages,
both can be observed in the verbose_probe or the shell.
You need to start a server ...
```
./build/probed_calculator --server --host=<calculator-host> --port=<calculator-port> --caf_probe_host=<nexus-host> --caf_probe_port=<nexus-port>
```
... and a client
```
while :; do ./build/probed_calculator --host=<calculator-host> --port=<calculator-port> --caf_probe_host=<nexus-host> --caf_probe_port=<nexus-port>; sleep 1; done
```

Dependencies
------------

* https://github.com/actor-framework/actor-framework

with the submodules

* probe
* probe-event
* nexus
* shell

