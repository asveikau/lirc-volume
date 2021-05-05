# lirc-volume

This is a small program to read events from a LIRC remote and set the volume on the
mixer when volume keys are pressed.

The location of the `lirc` socket, `/var/run/lirc/lircd`, is hardcoded, as are the
key symbols `KEY_VOLUMEUP` and `KEY_VOLUMEDOWN`.  The program will connect to the
socket, process events from `lirc`, and move the ALSA volume up or down by 5%
increments.

This relies on a few other projects of mine.  [`libaudio`][1] is used to control
the mixer (rather than talk to ALSA directly - theoretically it would be possible
to use this with other audio APIs).  [`libpollster`][2] is used as an event loop
for the Unix domain socket. 

## Building

    $ git submodule update --init
    $ make

Based on my experiences with some of the submodules, old versions of GCC (7 and
earlier) may not work.

[1]: https://github.com/asveikau/audio
[2]: https://github.com/asveikau/pollster
