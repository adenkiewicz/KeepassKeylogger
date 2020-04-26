# KeepassKeylogger
This is a simple POC to show that Master Password can be easily keylogged from "Secure desktop".

The "secure desktop" is not really Winlogon screen (the one used by UAC prompt or ctrl+alt+delete sequence), 
but rather another desktop - of random name - created by the keepass. The user running KeePass must have access 
to the desktop, thus we can create keylogger that attaches to new desktops and keyloggs Master Password.

This is _not_ a security issue in KeePass - it's just how it works.

Still, be sure to enable "Secure desktop" feature - at least must keyloggers won't work out of box.
