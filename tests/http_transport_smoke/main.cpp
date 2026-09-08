#include "mcp/http_transport.hpp"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <chrono>
#include <climits>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace {

#ifdef _WIN32
using Socket = SOCKET;
constexpr Socket kInvalidSocket = INVALID_SOCKET;
void CloseSocket(Socket socket) { closesocket(socket); }
#else
using Socket = int;
constexpr Socket kInvalidSocket = -1;
void CloseSocket(Socket socket) { close(socket); }
#endif

bool Expect(bool condition, const char *message) {
  if (condition) {
    return true;
  }
  std::cerr << "[http_transport] FAIL: " << message << '\n';
  return false;
}

bool SendAll(Socket socket, const std::string &value) {
  std::size_t offset = 0;
  while (offset < value.size()) {
#ifdef _WIN32
    const int sent = send(
        socket, value.data() + offset,
        static_cast<int>((std::min)(value.size() - offset,
                                     static_cast<std::size_t>(INT_MAX))),
        0);
#else
    const ssize_t sent = send(socket, value.data() + offset,
                              value.size() - offset, 0);
#endif
    if (sent <= 0) {
      return false;
    }
    offset += static_cast<std::size_t>(sent);
  }
  return true;
}

bool HasHeader(const std::string &headers, const char *name) {
  const std::string lowerName = [&] {
    std::string value(name);
    for (char &character : value) {
      character = static_cast<char>(std::tolower(
          static_cast<unsigned char>(character)));
    }
    return value;
  }();
  std::size_t begin = 0;
  while (begin < headers.size()) {
    const std::size_t end = headers.find("\r\n", begin);
    const std::string line = headers.substr(
        begin, end == std::string::npos ? std::string::npos : end - begin);
    const std::size_t colon = line.find(':');
    if (colon != std::string::npos) {
      std::string lineName = line.substr(0, colon);
      for (char &character : lineName) {
        character = static_cast<char>(std::tolower(
            static_cast<unsigned char>(character)));
      }
      if (lineName == lowerName) {
        return true;
      }
    }
    if (end == std::string::npos) {
      break;
    }
    begin = end + 2;
  }
  return false;
}

std::string Request(std::uint16_t port, const std::string &body,
                    const std::string &extraHeaders = {},
                    const std::string &method = "POST",
                    const std::string &path = "/mcp") {
  Socket socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (socket == kInvalidSocket) {
    return {};
  }

#ifdef _WIN32
  const DWORD timeoutMs = 3000;
  setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO,
             reinterpret_cast<const char *>(&timeoutMs), sizeof(timeoutMs));
#else
  timeval timeout{};
  timeout.tv_sec = 3;
  setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(port);
  (void)inet_pton(AF_INET, "127.0.0.1", &address.sin_addr);
  if (connect(socket, reinterpret_cast<const sockaddr *>(&address),
              sizeof(address)) != 0) {
    CloseSocket(socket);
    return {};
  }

  std::ostringstream request;
  request << method << ' ' << path << " HTTP/1.1\r\n"
          << "Host: 127.0.0.1\r\n"
          << (HasHeader(extraHeaders, "authorization")
                  ? ""
                  : "Authorization: Bearer smoke-token\r\n")
          << (HasHeader(extraHeaders, "accept") ? ""
                                                   : "Accept: application/json\r\n")
          << "Content-Type: application/json\r\n"
          << "Content-Length: " << body.size() << "\r\n"
          << extraHeaders << "\r\n" << body;
  const std::string wire = request.str();
  if (!SendAll(socket, wire)) {
    CloseSocket(socket);
    return {};
  }

  std::string response;
  char buffer[4096];
  for (;;) {
#ifdef _WIN32
    const int received = recv(socket, buffer, sizeof(buffer), 0);
#else
    const ssize_t received = recv(socket, buffer, sizeof(buffer), 0);
#endif
    if (received <= 0) {
      break;
    }
    response.append(buffer, static_cast<std::size_t>(received));
  }
  CloseSocket(socket);
  return response;
}

bool HasStatus(const std::string &response, int status) {
  return response.rfind("HTTP/1.1 " + std::to_string(status) + " ", 0) == 0;
}

bool HasBody(const std::string &response, const std::string &value) {
  const std::size_t separator = response.find("\r\n\r\n");
  return separator != std::string::npos &&
         response.find(value, separator + 4) != std::string::npos;
}

} // namespace

int main() {
#ifdef _WIN32
  WSADATA wsaData{};
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    std::cerr << "[http_transport] WSAStartup failed\n";
    return 1;
  }
#endif

  using namespace unityexplorer::mcp;
  bool ok = true;
  McpServer server;
  ok &= Expect(server.RegisterTool(ToolDefinition{
                          "echo", "Echo one value",
                          Json{{"type", "object"},
                               {"properties", Json{{"value", Json{{"type", "string"}}}}},
                               {"required", Json::array({"value"})},
                               {"additionalProperties", false}},
                          Json{{"type", "object"}}, {},
                          [](const Json &arguments, const RequestContext &) {
                            return ToolResult::Success(
                                Json{{"value", arguments.at("value")}});
                          }}),
                     "echo tool should register");
  HttpTransportOptions options;
  options.port = 0;
  options.bearerToken = "smoke-token";
  options.requireBearerToken = true;
  options.allowMissingOrigin = true;
  StreamableHttpTransport transport(options);
  std::atomic<bool> stop{false};
  std::ostringstream errors;
  std::thread serverThread([&] {
    (void)transport.Run(server, RequestContext{}, &stop, &errors);
  });

  std::uint16_t port = 0;
  for (int attempt = 0; attempt < 100 && port == 0; ++attempt) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    port = transport.BoundPort();
  }
  ok &= Expect(port != 0, "HTTP transport should bind an ephemeral port");

  if (port != 0) {
    const std::string ping =
        R"({"jsonrpc":"2.0","id":1,"method":"ping"})";
    const std::string pingResponse = Request(port, ping);
    ok &= Expect(HasStatus(pingResponse, 200),
                 "standard request without custom headers should succeed");
    ok &= Expect(HasBody(pingResponse, R"("id":1)"),
                 "standard request should return JSON-RPC response");

    const std::string batch =
        R"([{"jsonrpc":"2.0","id":2,"method":"ping"},{"jsonrpc":"2.0","method":"notifications/initialized"}])";
    const std::string batchResponse = Request(port, batch);
    ok &= Expect(HasStatus(batchResponse, 200),
                 "JSON-RPC batch without custom headers should succeed");
    ok &= Expect(HasBody(batchResponse, R"("id":2)"),
                 "JSON-RPC batch should return non-notification response");

    const std::string toolCall =
        R"({"jsonrpc":"2.0","id":3,"method":"tools/call","params":{"name":"echo","arguments":{"value":"ok"}}})";
    const std::string toolResponse = Request(port, toolCall);
    ok &= Expect(HasStatus(toolResponse, 200) &&
                     HasBody(toolResponse, R"("isError":false)"),
                 "tools/call should not require Mcp-Name");

    const std::string namedToolResponse =
        Request(port, toolCall, "Mcp-Name: echo\r\n");
    ok &= Expect(HasStatus(namedToolResponse, 200),
                 "matching optional Mcp-Name should succeed");

    const std::string wrongNameResponse =
        Request(port, toolCall, "Mcp-Name: other\r\n");
    ok &= Expect(HasStatus(wrongNameResponse, 400) &&
                     HasBody(wrongNameResponse, "invalid_protocol"),
                 "present mismatched Mcp-Name should be rejected");

    const std::string mirroredResponse = Request(
        port, ping, "Mcp-Method: ping\r\nMCP-Protocol-Version: 2026-07-28\r\n");
    ok &= Expect(HasStatus(mirroredResponse, 200),
                 "matching optional mirrored headers should succeed");

    const std::string mismatchResponse =
        Request(port, ping, "Mcp-Method: tools/list\r\n");
    ok &= Expect(HasStatus(mismatchResponse, 400) &&
                     HasBody(mismatchResponse, "invalid_protocol"),
                 "present mismatched Mcp-Method should be rejected");

    const std::string versionResponse =
        Request(port, ping, "MCP-Protocol-Version: unsupported\r\n");
    ok &= Expect(HasStatus(versionResponse, 400) &&
                     HasBody(versionResponse, "invalid_protocol"),
                 "present mismatched protocol version should be rejected");

    const std::string noAuth =
        Request(port, ping, "Authorization: Bearer wrong-token\r\n");
    ok &= Expect(HasStatus(noAuth, 401),
                 "invalid bearer token should remain rejected");

    const std::string deniedOrigin =
        Request(port, ping, "Origin: https://unexpected.invalid\r\n");
    ok &= Expect(HasStatus(deniedOrigin, 403),
                 "non-allowlisted Origin should remain rejected");

    const std::string wrongAccept = Request(
        port, ping, "Accept: text/plain\r\n");
    ok &= Expect(HasStatus(wrongAccept, 415),
                 "unsupported Accept should remain rejected");

    const std::string wrongPath = Request(port, ping, {}, "POST", "/other");
    ok &= Expect(HasStatus(wrongPath, 404), "wrong MCP path should be rejected");

    const std::string wrongMethod = Request(port, {}, {}, "GET", "/mcp");
    ok &= Expect(HasStatus(wrongMethod, 405), "non-POST method should be rejected");
  }

  stop.store(true, std::memory_order_relaxed);
  transport.Stop();
  if (serverThread.joinable()) {
    serverThread.join();
  }
  ok &= Expect(errors.str().empty(), "HTTP transport should not report startup errors");

#ifdef _WIN32
  WSACleanup();
#endif
  return ok ? 0 : 1;
}
