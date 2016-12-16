/** \mainpage

\section intro Introduction

  This is the Reference Manual to the MagiCServer++ library.

  MagiCServer++ is a framework for implementing efficient and flexible
  Internet server applications. It supports both connection-based TCP
  and connectionless UDP datagram protocols in a transparent fashion.

  The main task of the framework is to listen to a server socket and a
  number of connected TCP client sockets. When a client connects to
  the server socket, a new connection socket is created and added to
  the list of established client sockets. When data arrives from a
  client to a connection socket, it is relayed to the user application
  as a request. The application can respond, if necessary. The user
  application is notified also about other important events, such a
  establishment of a new connection, losing an old one, and initiation
  of server shutdown.

  The framework is implemented as a C++ library, which has been kept
  as independent from other libraries as possible, to make reuse
  easier. Error handling is done with error codes; exceptions are not
  used except for constructors. For data structures, low-level C data
  structures are used for most tasks. Simple support tools are
  provided for logging, threading, and queues.

\par Reference Manual features

  This Reference Manual provides the following documentation features:

  \li External and internal documentation
  \li Class documentation
  \li File documentation
  \li Source code browsing
  \li Complete usage cross-references
  \li Include graphds and detailed internal collaboration diagrams
  \li Reference documentation for the sample applications
  \li Indexes

*/
