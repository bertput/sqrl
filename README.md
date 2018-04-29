# README for sqrl native linux client

This is a native linux SQRL client that implements the SQRL protocol written by Steve Gibson.
See his [grc.com website](https://www.grc.com/sqrl/sqrl.htm) for more details.

This version enables the GUI -- which is very basic, but is a starting point.
This version also enables client/server mode by forking the process to run the GUI/server in the case where the GUI process is not already running.

**Note:**  If the GUI exits unexpectedly (Ctrl-C, or similar), it will leave the FIFO at `/tmp/sqrl.FIFO` and prevent a subsequent startup.  Simply remove this file to resume normal operations.  I'll be adding an exit hook soon that will fix this issue. :-)

## Prerequisites

SQRL depends on `GTK-2.0`, `libsqrl`, `libsodium`, and `liburiparser`.

You should be able to get `GTK-2.0` and `liburiparser` from your linux repos.  You will probably need to get the `-dev` version of the package if it is available on your distro.

Build `libsqrl` and `libsodium`:

    git clone https://github.com/Novators/libsqrl.git
    cd libsqrl
    mkdir build
    cd build
    cmake ..
    make

You may choose to install libsqrl with the command `sudo make install`, in which case the client will link to the installed dynamic library.  If you'd rather not, you'll have some more options in the next section.

## Building the client

If you did not install libsqrl, or want to statically link it, you'll need to set an environment variable.  Otherwise, you can skip this step.

    export LIBSQRL=/path/to/your/libsqrl/directory


This client is written in C and should be buildable on linux with a simple command:

    cd src
    make


The client includes a sample SQRL id and a configuration file that must be installed for it to work.  You have a few options here:

    # To install everything to your home directory:
    make install
    # Or, to install the binary in /usr/local/bin and data files to your home directory:
    PREFIX=/usr/local make install
    # Or, to install only the data files:
    make installdata

The `make install` command will not overwrite existing data files (but will update the binary).

## Using your own SQRL id

You can point the client to a different ID by editing the `~/.sqrl/sqrl.ini` file.

## Connecting your browser to SQRL

To log into a site that supports SQRL, you will need to set up your browser to direct the `sqrl://` scheme to this executable.
While this version of `SQRL` is originally command line, it will ultimately implement a GUI to accept user input to complete the login process, similar to Steve's windows client.

## Running the client

After setting up your browser (in the previous section), this should be automatic when you click a sqrl:// link.

You can also run it from the command line: `sqrl sqrl://...`
