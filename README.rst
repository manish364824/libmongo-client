libmongo-client
===============

This is an alternative C driver for `MongoDB`_, with slightly
different goals than the official one:

libmongo-client is meant to be a stable (API, ABI and quality alike),
clean, well documented and well tested shared library, that strives to
make the most common use cases as convenient as possible.

Features
--------

The main purpose of this library is to provide a well maintained
implementation, that suits a wider range of needs than the original:

* Well documented, easy, clean and stable API.
* Comprehensive test suite, with over 90% code coverage, and
  increasing.
* Convenient APIs to work with BSON objects and MongoDB documents.

Apart from these, the library is meant to be used as a shared library,
therefore API and ABI compatibility will be maintained, and only
broken when absolutely neccessary - in which case, a sufficiently
bumped version will be branched off.

Requirements
------------

Apart from the autotools and a C compiler, there are no other hard
dependencies.

To build the documentation, `Doxygen`_ will be needed too.

Installation
------------

The library follows the usual autotools way of installation:

::

 $ git clone git://github.com/algernon/libmongo-client.git
 $ cd libmongo-client
 $ ./autogen.sh
 $ ./configure && make && make install

License
-------

While former versions were released under the Apache License version
2.0, recent versions are released under the GNU Lesser General Public
License, version 3 (or at your opinion, any later version).

See the doc/LICENSE.GPL and doc/LICENSE.LGPL files for license information.

.. _MongoDB: http://www.mongodb.org/
.. _Doxygen: http://www.stack.nl/~dimitri/doxygen/
