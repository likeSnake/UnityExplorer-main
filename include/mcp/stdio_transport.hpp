#pragma once

#include "mcp/mcp_server.hpp"

#include <atomic>
#include <iostream>
#include <istream>
#include <ostream>

namespace unityexplorer::mcp {

struct StdioTransportOptions {
  // Protocol framing is newline-delimited JSON-RPC.  A line is never split or
  // buffered across requests, which keeps this transport suitable for MCP
  // clients that launch an elevated sidecar as a child process.
  bool flushEachResponse = true;
  bool ignoreBlankLines = true;
};

class StdioTransport final {
public:
  explicit StdioTransport(StdioTransportOptions options = {});

  // Runs until EOF, I/O failure, or Stop() is requested.  Returns 0 for a
  // clean EOF/stop, 1 for an input/output failure.  Logs are deliberately
  // emitted only through the supplied error stream; stdout remains protocol
  // exclusive when the caller passes std::cin/std::cout.
  int Run(const McpServer &server, const RequestContext &context,
          std::istream &input, std::ostream &output, std::ostream &error);

  int Run(const McpServer &server, std::istream &input = std::cin,
          std::ostream &output = std::cout, std::ostream &error = std::cerr) {
    return Run(server, RequestContext{}, input, output, error);
  }

  void Stop() noexcept { stop_.store(true, std::memory_order_relaxed); }
  bool IsStopped() const noexcept {
    return stop_.load(std::memory_order_relaxed);
  }

private:
  StdioTransportOptions options_;
  std::atomic<bool> stop_{false};
};

} // namespace unityexplorer::mcp
