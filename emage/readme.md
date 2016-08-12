# EmPOWER Agent project

The EmPOWER Agent (EMAge for friends) is an integrable piece of software which provides the 'slave' counterpart in the controller-agent communication of EmPOWER. EMAge is technology agnostic: this means that it does not depends on a particular implementation of LTE stack, but can be integrated with any custom software. Just a minimal modification of the stack itself is necessary in order to have the agent working (see documentation for more information).

### Compatibility
This software has been developed and tested for Linux, but it's minimal pre-requisites and the clean arcitecture allows to port it on different OS.

### Pre-requisites
In order to successfully buil EMAge you need:
* Linux standard build suite (GCC, LD, AR, etc...).
* Pthread library, necessary to handle multithreading.
* Protobuf-c, Google protocol buffer implementation for C.

### Build instructions
In order to use EMAge the necessary steps to do are:
* Clone this repository.
* Build the protocols used for network communication by invoking the `make` command inside the proto directory (will be built as a library).
* Build the agent by invoking the `make` command inside the agent directory (will be built as library).
* Include both the libraries when you are embedding the agent in your project (see Documentation for more information about this).

### Run the agent
Thake in account that in order to run the agent you must prepare a configuration file which has to be named `agent.conf` and placed under the `/etc/empower` directory. The syntax of such configuration file is, for the moment, outrageously simple, and just contains the necessary information to reach the controller. Copying the empower folder which is present in this repository `conf` directory under `etc` usually is the only step to do.

Cheers,
Kewin R.