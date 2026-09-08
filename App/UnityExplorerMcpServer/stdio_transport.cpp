#include "mcp/stdio_transport.hpp"

#include <string>

namespace unityexplorer::mcp {

StdioTransport::StdioTransport(StdioTransportOptions options)
    : options_(options) {}

int StdioTransport::Run(const McpServer &server,
                        const RequestContext &context, std::istream &input,
                        std::ostream &output, std::ostream &error) {
  stop_.store(false, std::memory_order_relaxed);
  std::string line;
  while (!IsStopped() && std::getline(input, line)) {
    if (line.empty() && options_.ignoreBlankLines) {
      continue;
    }
    const std::string response = server.HandleLine(line, context);
    if (response.empty()) {
      continue; // JSON-RPC notification.
    }
    output << response << '\n';
    if (options_.flushEachResponse) {
      output.flush();
    }
    if (!output.good()) {
      error << "stdio transport: failed to write response\n";
      return 1;
    }
  }

  if (input.bad()) {
    error << "stdio transport: failed to read request\n";
    return 1;
  }
  return 0;
}

} // namespace unityexplorer::mcp

