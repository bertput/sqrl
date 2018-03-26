# README for sqrl native linux client

This is a native linux SQRL client that implements the SQRL protocol written by Steve Gibson.
See his [grc.com website](https://www.grc.com/sqrl/sqrl.htm) for more details.

## Building the client

This client is written in C and should be buildable on linux with a simple command:

    cd src
    make

Before you take these steps, you will need to verify the location of your `libsqrl` library after you build it and modify the `Makefile` with the location of your `libsqrl` library.

## Prerequisites

SQRL depends on `libsqrl`, `libsodium`, and `liburiparser`.

Grab the sources for `libsqrl` from github and build it.
When you do that, `libsqrl` will retrieve the correct version of `libsodium` as part of the build process and you will then have a nice shiny new `libsqrl` to link to.

You should be able to get `liburiparser` from your linux repos.  You will probably need to get the `-dev` version of the package if it is available on your distro.

## Connecting your browser to SQRL

To log into a site that supports SQRL, you will need to set up your browser to direct the `sqrl://` scheme to this executable.
While this version of `SQRL` is originally command line, it will ultimately implement a GUI to accept user input to complete the login process, similar to Steve's windows client.

## Running the client

The client needs to be able to get to libsqrl and libsodium libraries at run time.

A quick way to do that is with this command:

    export LD_LIBRARY_PATH=/home/bput/projects/libsqrl/build/lib/
    (or wherever your library is)

Another way to do that is to install the `libsqrl` shared library by running `make install` in the libsqrl directory *as root*.

    cd /home/bput/projects/libsqrl/build
    sudo make install


