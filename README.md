BatrachSecure
=============

Command-line app in qt5 for [TextSecure](https://whispersystems.org/).

Mainly a project by Batrachian Excellence. Help was got on IRC's #qt, from the
emissaries of event-based programming, dark hacks, and certain psychoactive
toads. They might not describe themselves as such. This is ok.

You may question why such a thing should exist, or even the sanity of whoever
undertakes such an enterprise. The reason is, that the [Jolla](http://jolla.com/)
smartphone does not have a native app for TextSecure. BatrachSecure therefore
exists to provide an API client that can be later used in other projects,
like a smartphone app.

Someone involved in the project was a huge fan of a certain Whatsapp client,
until it was banned. That sparked the inspiration to start the work on this.

Few people would use BatrachSecure, or anything else on the CLI directly, but
those who have used it have never been the same.  Nor have they been able to do
so yet, as the app is far from complete. Yet you know who you are.

Getting started
---------------

Set up a [server](https://github.com/WhisperSystems/TextSecure-Server/wiki/Using-your-own-server)
or [docker](https://github.com/janimo/textsecure-docker/) to test against.

For your server, consider @janimo's [fork](https://github.com/janimo/TextSecure-Server)
as it provides the ``dev`` registration mode. Once you have seen it in the wild,
your perception of registration will be altered for the rest of your life.

Clone [libcurve25519](https://github.com/CODeRUS/libcurve25519)
and [libaxolotl](https://github.com/CODeRUS/libaxolotl) onto the same level as this code
and compile them in that order.

    $ LD_LIBRARY_PATH=../libcurve25519/:../libaxolotl-qt5/:$LD_LIBRARY_PATH \
      ./BtxSecure http://localhost:8080 --get-verification-code --transport dev \
      --number +358501234567; echo

TODO
----

Everything except receiving a verification code on your phone. As the frogs
willed it, that code is available.

_iä ribitt_

