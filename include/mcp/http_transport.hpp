#pragma once

#include "mcp/mcp_server.hpp"

#include <atomic>
#include <cstdint>
#include <iosfwd>
#include <set>
#include <string>

namespace unityexplorer::mcp {

// Minimal dependency-free Streamable HTTP transport for the local sidecar.
// It intentionally binds to loopback by default.  The protocol core remains
// transport agnostic, so the same McpServer can be used by stdio and HTTP.
struct HttpTransportOptions {
  std::string bindAddress = "127.0.0.1";
  std::uint16_t port = 18765;
  std::string path = "/mcp";
  std::set<std::string> allowedOrigins;
  std::string bearerToken;
  std::string protocolVersion = "2026-07-28";
  std::size_t maxHeaderBytes = 32u * 1024u;
  std::size_t maxBodyBytes = 4u * 1024u * 1024u;
  int backlog = 16;
  bool requireBearerToken = true;
  bool allowMissingOrigin = true;
  bool allowNonLoopback = false;
  // Standard MCP clients may omit this header before negotiating a session.
  // When present it is always checked against protocolVersion.  Set this to
  // true only for deployments that intentionally require the header.
  bool requireProtocolVersion = false;
  bool requireMirroredMetadata = true;
};

class StreamableHttpTransport final {
public:
  explicit StreamableHttpTransport(HttpTransportOptions options = {});
  ~StreamableHttpTransport();

  StreamableHttpTransport(const StreamableHttpTransport &) = delete;
  StreamableHttpTransport &operator=(const StreamableHttpTransport &) = delete;

  // Blocks in an accept loop until Stop() or externalStop is set.  Each
  // accepted connection is handled by a short-lived worker and is closed
  // after one request/response.  This keeps the initial transport bounded and
  // lets several clients connect without sharing socket state.
  int Run(const McpServer &server, const RequestContext &baseContext,
          std::atomic<bool> *externalStop = nullptr,
          std::ostream *error = nullptr);

  int Run(const McpServer &server, std::atomic<bool> *externalStop = nullptr,
          std::ostream *error = nullptr) {
    return Run(server, RequestContext{}, externalStop, error);
  }

  void Stop() noexcept;
  bool IsStopped() const noexcept;
  std::uint16_t BoundPort() const noexcept;

private:
  struct Impl;
  HttpTransportOptions options_;
  Impl *impl_ = nullptr;
};

} // namespace unityexplorer::mcp
