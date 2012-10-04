THINKLIGHT
----------

Thinklight is an utility which enables the use of Thinkpad Light for blinking notifications on Lenovo/IBM laptops. It has been tested on Lenovo/IBM T60p.

You need root privileges to use this utility. It is best to add an entry at the end of the `/etc/sudoers` file, like this:

    your-username ALL=NOPASSWD: /usr/local/bin/thinklight

and replace your-username with your real username.

You can then call the utility by running `sudo thinklight`.


HOW TO BUILD
------------

1) make
2) sudo make install (this is not yet implemented, you will have to manually copy the thinklight binary to `/usr/local/bin/` folder)

HOW TO STOP THE BLINKING LIGHT
------------------------------

You can use CTRL+C or run the `sudo thinklight -q` which will check if there is a running thinklight process and kill it. Only one thinklight process can be run at the same time.

You can map any hotkey to disable the blinking, but if you're only using tty, you can map the ThinkVantage key to `sudo thinklight -q` command. Here is how it's done on Ubuntu:

Add the following two lines to the `/etc/acpi/events/thinkvantage` file

    event=ibm/hotkey HKEY 00000080 00001018
    action=/usr/local/bin/thinklight -q

and run `sudo service acpi restart`.
