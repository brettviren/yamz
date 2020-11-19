#ifndef YAMZ_ZEROMQ_HPP
#define YAMZ_ZEROMQ_HPP

// We really want to use poller and that currently requires DRAFT
#define ZMQ_BUILD_DRAFT_API 1
#include <zmq.hpp>

// Zyre provides the heavy lifting.
#include <zyre.h>

#endif
