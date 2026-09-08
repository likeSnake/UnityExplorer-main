#include "mcp/http_transport.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <cctype>
#include <chrono>
#include <climits>
#include <cstring>
#include <mutex>
#include <memory>
#include <sstream>
#include <thread>
#include <vector>

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

namespace unityexplorer::mcp {
namespace {

#ifdef _WIN32
using Socket = SOCKET;
constexpr Socket kInvalidSocket = INVALID_SOCKET;
int CloseSocket(Socket socket) { return closesocket(socket); }
int LastSocketError() { return WSAGetLastError(); }
#else
using Socket = int;
constexpr Socket kInvalidSocket = -1;
int CloseSocket(Socket socket) { return close(socket); }
int LastSocketError() { return errno; }
#endif

std::string LowerAscii(std::string value) {
  for (char &character : value) {
    character = static_cast<char>(
        std::tolower(static_cast<unsigned char>(character)));
  }
  return value;
}

std::string Trim(std::string value) {
  const auto notSpace = [](unsigned char character) {
    return std::isspace(character) == 0;
  };
  value.erase(value.begin(),
              std::find_if(value.begin(), value.end(), notSpace));
  value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(),
              value.end());
  return value;
}

bool ContainsToken(const std::string &header, const std::string &token) {
  const std::string lowerHeader = LowerAscii(header);
  const std::string lowerToken = LowerAscii(token);
  std::size_t begin = 0;
  while (begin < lowerHeader.size()) {
    const std::size_t end = lowerHeader.find(',', begin);
    const std::string item = Trim(lowerHeader.substr(
        begin, end == std::string::npos ? std::string::npos : end - begin));
    if (item == lowerToken) {
      return true;
    }
    if (end == std::string::npos) {
      break;
    }
    begin = end + 1;
  }
  return false;
}

bool ConstantTimeEqual(const std::string &left, const std::string &right) {
  const std::size_t length = (std::max)(left.size(), right.size());
  unsigned char difference = static_cast<unsigned char>(left.size() ^ right.size());
  for (std::size_t index = 0; index < length; ++index) {
    const unsigned char leftByte =
        index < left.size() ? static_cast<unsigned char>(left[index]) : 0;
    const unsigned char rightByte =
        index < right.size() ? static_cast<unsigned char>(right[index]) : 0;
    difference = static_cast<unsigned char>(difference | (leftByte ^ rightByte));
  }
  return difference == 0;
}

struct HttpRequest {
  std::string method;
  std::string path;
  std::string version;
  std::map<std::string, std::string> headers;
  std::string body;
};

struct HttpResponse {
  int status = 200;
  std::string reason = "OK";
  std::string contentType = "application/json";
  std::string body;
};

const char *ReasonPhrase(int status) {
  switch (status) {
  case 200:
    return "OK";
  case 400:
    return "Bad Request";
  case 401:
    return "Unauthorized";
  case 403:
    return "Forbidden";
  case 404:
    return "Not Found";
  case 405:
    return "Method Not Allowed";
  case 413:
    return "Payload Too Large";
  case 415:
    return "Unsupported Media Type";
  case 500:
    return "Internal Server Error";
  case 503:
    return "Service Unavailable";
  default:
    return "Error";
  }
}

HttpResponse JsonHttpError(int status, const std::string &code,
                           const std::string &message) {
  HttpResponse response;
  response.status = status;
  response.reason = ReasonPhrase(status);
  response.body = Json{{"ok", false},
                       {"error", Json{{"code", code}, {"message", message}}}}
                     .dump();
  return response;
}

bool SendAll(Socket socket, const char *data, std::size_t size) {
  while (size != 0) {
#ifdef _WIN32
    const int sent = send(socket, data,
                          static_cast<int>((std::min)(size, static_cast<std::size_t>(INT_MAX))),
                          0);
#else
    const ssize_t sent = send(socket, data, size, 0);
#endif
    if (sent <= 0) {
      return false;
    }
    data += sent;
    size -= static_cast<std::size_t>(sent);
  }
  return true;
}

bool ReceiveUntilHeaders(Socket socket, std::string &buffer,
                         std::size_t maxBytes) {
  std::array<char, 4096> chunk{};
  while (buffer.find("\r\n\r\n") == std::string::npos) {
    if (buffer.size() >= maxBytes) {
      return false;
    }
#ifdef _WIN32
    const int received = recv(socket, chunk.data(),
                              static_cast<int>((std::min)(chunk.size(), maxBytes - buffer.size())),
                              0);
#else
    const ssize_t received = recv(socket, chunk.data(),
                                  (std::min)(chunk.size(), maxBytes - buffer.size()),
                                  0);
#endif
    if (received <= 0) {
      return false;
    }
    buffer.append(chunk.data(), static_cast<std::size_t>(received));
  }
  return true;
}

bool ReceiveBody(Socket socket, std::string &buffer, std::size_t bodyOffset,
                 std::size_t bodyLength, std::size_t maxBodyBytes) {
  if (bodyLength > maxBodyBytes) {
    return false;
  }
  std::array<char, 4096> chunk{};
  while (buffer.size() < bodyOffset + bodyLength) {
    const std::size_t remaining = bodyOffset + bodyLength - buffer.size();
#ifdef _WIN32
    const int received = recv(socket, chunk.data(),
                              static_cast<int>((std::min)(chunk.size(), remaining)),
                              0);
#else
    const ssize_t received = recv(socket, chunk.data(),
                                  (std::min)(chunk.size(), remaining), 0);
#endif
    if (received <= 0) {
      return false;
    }
    buffer.append(chunk.data(), static_cast<std::size_t>(received));
  }
  return true;
}

bool ParseRequest(Socket socket, const HttpTransportOptions &options,
                  HttpRequest &request, HttpResponse &errorResponse) {
  std::string buffer;
  if (!ReceiveUntilHeaders(socket, buffer, options.maxHeaderBytes)) {
    errorResponse = JsonHttpError(400, "invalid_http", "invalid or oversized HTTP headers");
    return false;
  }

  const std::size_t delimiter = buffer.find("\r\n\r\n");
  const std::string headerBlock = buffer.substr(0, delimiter);
  const std::size_t firstLineEnd = headerBlock.find("\r\n");
  if (firstLineEnd == std::string::npos) {
    errorResponse = JsonHttpError(400, "invalid_http", "missing HTTP request line");
    return false;
  }

  {
    std::istringstream requestLine(headerBlock.substr(0, firstLineEnd));
    if (!(requestLine >> request.method >> request.path >> request.version) ||
        request.version != "HTTP/1.1") {
      errorResponse = JsonHttpError(400, "invalid_http", "HTTP/1.1 request required");
      return false;
    }
  }

  std::size_t lineBegin = firstLineEnd + 2;
  while (lineBegin < headerBlock.size()) {
    const std::size_t lineEnd = headerBlock.find("\r\n", lineBegin);
    const std::string line = headerBlock.substr(
        lineBegin, lineEnd == std::string::npos ? std::string::npos
                                                 : lineEnd - lineBegin);
    const std::size_t colon = line.find(':');
    if (colon == std::string::npos || colon == 0) {
      errorResponse = JsonHttpError(400, "invalid_http", "malformed HTTP header");
      return false;
    }
    const std::string name = LowerAscii(Trim(line.substr(0, colon)));
    const std::string value = Trim(line.substr(colon + 1));
    if (name.empty() || request.headers.find(name) != request.headers.end()) {
      errorResponse = JsonHttpError(400, "invalid_http", "duplicate or empty HTTP header");
      return false;
    }
    request.headers.emplace(name, value);
    if (lineEnd == std::string::npos) {
      break;
    }
    lineBegin = lineEnd + 2;
  }

  const auto contentLengthIt = request.headers.find("content-length");
  std::size_t contentLength = 0;
  if (contentLengthIt != request.headers.end()) {
    try {
      std::size_t parsed = 0;
      const unsigned long long value =
          std::stoull(contentLengthIt->second, &parsed, 10);
      if (parsed != contentLengthIt->second.size() ||
          value > static_cast<unsigned long long>(options.maxBodyBytes)) {
        errorResponse = JsonHttpError(413, "body_too_large", "request body exceeds configured limit");
        return false;
      }
      contentLength = static_cast<std::size_t>(value);
    } catch (...) {
      errorResponse = JsonHttpError(400, "invalid_http", "invalid Content-Length");
      return false;
    }
  }
  if (request.headers.find("transfer-encoding") != request.headers.end()) {
    errorResponse = JsonHttpError(400, "invalid_http", "Transfer-Encoding is not supported");
    return false;
  }
  if (!ReceiveBody(socket, buffer, delimiter + 4, contentLength,
                   options.maxBodyBytes)) {
    errorResponse = JsonHttpError(400, "invalid_http", "incomplete request body");
    return false;
  }
  request.body = buffer.substr(delimiter + 4, contentLength);
  return true;
}

void SetSocketTimeouts(Socket socket) {
#ifdef _WIN32
  const DWORD timeoutMs = 10000;
  setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO,
             reinterpret_cast<const char *>(&timeoutMs), sizeof(timeoutMs));
  setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO,
             reinterpret_cast<const char *>(&timeoutMs), sizeof(timeoutMs));
#else
  timeval timeout{};
  timeout.tv_sec = 10;
  setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
  setsockopt(socket, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#endif
}

void WriteHttpResponse(Socket socket, const HttpResponse &response,
                       bool sse) {
  const std::string contentType =
      sse ? "text/event-stream" : response.contentType;
  std::ostringstream headers;
  headers << "HTTP/1.1 " << response.status << ' ' << response.reason << "\r\n"
          << "Content-Type: " << contentType << "; charset=utf-8\r\n"
          << "Content-Length: " << response.body.size() << "\r\n"
          << "Cache-Control: no-store\r\n"
          << "Connection: close\r\n"
          << "X-Content-Type-Options: nosniff\r\n\r\n";
  const std::string headerText = headers.str();
  (void)SendAll(socket, headerText.data(), headerText.size());
  (void)SendAll(socket, response.body.data(), response.body.size());
}

bool IsAllowedOrigin(const HttpTransportOptions &options,
                     const HttpRequest &request) {
  const auto originIt = request.headers.find("origin");
  if (originIt == request.headers.end() || originIt->second.empty()) {
    return options.allowMissingOrigin;
  }
  return options.allowedOrigins.find(originIt->second) !=
         options.allowedOrigins.end();
}

bool IsAuthorized(const HttpTransportOptions &options,
                  const HttpRequest &request) {
  if (!options.requireBearerToken) {
    return true;
  }
  if (options.bearerToken.empty()) {
    return false;
  }
  const auto authIt = request.headers.find("authorization");
  if (authIt == request.headers.end()) {
    return false;
  }
  constexpr char prefix[] = "Bearer ";
  if (authIt->second.size() <= sizeof(prefix) - 1 ||
      LowerAscii(authIt->second.substr(0, sizeof(prefix) - 1)) !=
          LowerAscii(prefix)) {
    return false;
  }
  return ConstantTimeEqual(
      authIt->second.substr(sizeof(prefix) - 1), options.bearerToken);
}

bool ValidateMirroredHeaders(const HttpTransportOptions &options,
                             const HttpRequest &request, const Json &body,
                             HttpResponse &errorResponse) {
  // These Mcp-* headers are a legacy convenience used by a few clients, not
  // part of the JSON-RPC envelope.  Validate them when supplied, but never
  // make their absence a protocol error.  A batch has no single method/name,
  // so every applicable entry must agree with the supplied value.
  const auto validateEntries = [&](const auto &validator,
                                   bool requireStructured = true) -> bool {
    if (body.is_array()) {
      for (const Json &entry : body) {
        if (!validator(entry)) {
          return false;
        }
      }
      return true;
    }
    if (body.is_object()) {
      return validator(body);
    }
    if (requireStructured) {
      errorResponse = JsonHttpError(
          400, "invalid_protocol",
          "mirrored MCP headers require a JSON-RPC object or batch");
      return false;
    }
    return true;
  };

  const auto methodIt = request.headers.find("mcp-method");
  if (methodIt != request.headers.end()) {
    if (methodIt->second.empty()) {
      errorResponse = JsonHttpError(400, "invalid_protocol",
                                    "Mcp-Method must not be empty");
      return false;
    }
    if (!validateEntries([&](const Json &entry) {
          if (!entry.is_object() || !entry.contains("method") ||
              !entry["method"].is_string() ||
              entry["method"].get<std::string>() != methodIt->second) {
            errorResponse = JsonHttpError(
                400, "invalid_protocol",
                "Mcp-Method does not match every JSON-RPC method");
            return false;
          }
          return true;
        })) {
      return false;
    }
  }

  const auto versionIt = request.headers.find("mcp-protocol-version");
  if (versionIt != request.headers.end()) {
    if (versionIt->second.empty() ||
        versionIt->second != options.protocolVersion) {
      errorResponse = JsonHttpError(400, "invalid_protocol",
                                    "unsupported MCP-Protocol-Version");
      return false;
    }
  } else if (options.requireProtocolVersion) {
    errorResponse = JsonHttpError(400, "invalid_protocol",
                                  "missing MCP-Protocol-Version");
    return false;
  }

  // There is nothing to mirror when the protocol-version header is absent.
  // In particular, leave malformed/non-object JSON to McpServer so clients
  // still receive the standard JSON-RPC Invalid Request response.
  if (options.requireMirroredMetadata &&
      versionIt != request.headers.end()) {
    const Json *metadata = nullptr;
    const auto validateMetadata = [&](const Json &entry) {
      if (!entry.is_object()) {
        return true;
      }
      metadata = nullptr;
      if (entry.contains("_meta") && entry["_meta"].is_object()) {
        metadata = &entry["_meta"];
      } else if (entry.contains("params") && entry["params"].is_object() &&
                 entry["params"].contains("_meta") &&
                 entry["params"]["_meta"].is_object()) {
        metadata = &entry["params"]["_meta"];
      }
      if (metadata != nullptr && metadata->contains("protocolVersion") &&
          metadata->at("protocolVersion").is_string() &&
          metadata->at("protocolVersion").get<std::string>() !=
              versionIt->second) {
        errorResponse = JsonHttpError(
            400, "invalid_protocol",
            "protocol version metadata does not match header");
        return false;
      }
      return true;
    };
    if (!validateEntries(validateMetadata, false)) {
      return false;
    }
  }

  const auto nameIt = request.headers.find("mcp-name");
  if (nameIt != request.headers.end()) {
    if (nameIt->second.empty()) {
      errorResponse = JsonHttpError(400, "invalid_protocol",
                                    "Mcp-Name must not be empty");
      return false;
    }
    if (!validateEntries([&](const Json &entry) {
          // Entries unrelated to a named MCP method do not consume the
          // request-level Mcp-Name (this permits mixed JSON-RPC batches).
          if (!entry.is_object() || !entry.contains("method") ||
              !entry["method"].is_string()) {
            return true;
          }
          const std::string method = entry["method"].get<std::string>();
          if (method != "tools/call" && method != "resources/read") {
            return true;
          }
          if (!entry.contains("params") || !entry["params"].is_object() ||
              !entry["params"].contains("name") ||
              !entry["params"]["name"].is_string() ||
              entry["params"]["name"].get<std::string>() !=
                  nameIt->second) {
            errorResponse = JsonHttpError(
                400, "invalid_protocol",
                "Mcp-Name does not match every named request");
            return false;
          }
          return true;
        })) {
      return false;
    }
  }
  return true;
}

bool IsLoopbackAddress(const std::string &address) {
  // The first transport revision intentionally supports IPv4 loopback only.
  // Do not silently resolve a user-controlled hostname, which could turn a
  // local-only endpoint into a remote listener through DNS rebinding.
  return address == "127.0.0.1";
}

} // namespace

struct StreamableHttpTransport::Impl {
  struct Worker {
    std::thread thread;
    std::shared_ptr<std::atomic<bool>> done;
  };

  std::atomic<bool> stopped{false};
  std::atomic<bool> running{false};
  std::atomic<std::size_t> activeWorkers{0};
  std::mutex socketMutex;
  Socket listenSocket = kInvalidSocket;
  std::mutex workerMutex;
  std::vector<Worker> workers;
};

StreamableHttpTransport::StreamableHttpTransport(HttpTransportOptions options)
    : options_(std::move(options)), impl_(new Impl()) {}

StreamableHttpTransport::~StreamableHttpTransport() {
  Stop();
  if (impl_ != nullptr) {
    std::lock_guard<std::mutex> lock(impl_->workerMutex);
    for (Impl::Worker &worker : impl_->workers) {
      if (worker.thread.joinable()) {
        worker.thread.join();
      }
    }
    delete impl_;
    impl_ = nullptr;
  }
}

void StreamableHttpTransport::Stop() noexcept {
  if (impl_ == nullptr) {
    return;
  }
  impl_->stopped.store(true, std::memory_order_relaxed);
  std::lock_guard<std::mutex> lock(impl_->socketMutex);
  if (impl_->listenSocket != kInvalidSocket) {
    (void)CloseSocket(impl_->listenSocket);
    impl_->listenSocket = kInvalidSocket;
  }
}

bool StreamableHttpTransport::IsStopped() const noexcept {
  return impl_ == nullptr || impl_->stopped.load(std::memory_order_relaxed);
}

std::uint16_t StreamableHttpTransport::BoundPort() const noexcept {
  if (impl_ == nullptr) {
    return 0;
  }
  std::lock_guard<std::mutex> lock(impl_->socketMutex);
  if (impl_->listenSocket == kInvalidSocket) {
    return 0;
  }
  sockaddr_in address{};
#ifdef _WIN32
  int length = sizeof(address);
#else
  socklen_t length = sizeof(address);
#endif
  if (getsockname(impl_->listenSocket,
                  reinterpret_cast<sockaddr *>(&address), &length) != 0) {
    return 0;
  }
  return ntohs(address.sin_port);
}

int StreamableHttpTransport::Run(const McpServer &server,
                                 const RequestContext &baseContext,
                                 std::atomic<bool> *externalStop,
                                 std::ostream *error) {
  if (impl_ == nullptr || impl_->running.exchange(true)) {
    return 1;
  }
  impl_->stopped.store(false, std::memory_order_relaxed);

  if (!options_.allowNonLoopback && !IsLoopbackAddress(options_.bindAddress)) {
    impl_->running.store(false);
    if (error != nullptr) {
      *error << "http transport: refusing non-loopback bind address '"
             << options_.bindAddress << "'\n";
    }
    return 1;
  }

#ifdef _WIN32
  WSADATA wsaData{};
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    impl_->running.store(false);
    if (error != nullptr) {
      *error << "http transport: WSAStartup failed\n";
    }
    return 1;
  }
#endif

  Socket listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (listenSocket == kInvalidSocket) {
#ifdef _WIN32
    WSACleanup();
#endif
    impl_->running.store(false);
    if (error != nullptr) {
      *error << "http transport: socket creation failed (" << LastSocketError()
             << ")\n";
    }
    return 1;
  }
  int reuse = 1;
  (void)setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR,
#ifdef _WIN32
                   reinterpret_cast<const char *>(&reuse), sizeof(reuse)
#else
                   &reuse, sizeof(reuse)
#endif
  );

  sockaddr_in address{};
  address.sin_family = AF_INET;
  address.sin_port = htons(options_.port);
  if (inet_pton(AF_INET, options_.bindAddress.c_str(), &address.sin_addr) != 1 ||
      bind(listenSocket, reinterpret_cast<const sockaddr *>(&address),
           sizeof(address)) != 0 ||
      listen(listenSocket, options_.backlog) != 0) {
    (void)CloseSocket(listenSocket);
#ifdef _WIN32
    WSACleanup();
#endif
    impl_->running.store(false);
    if (error != nullptr) {
      *error << "http transport: bind/listen failed for " << options_.bindAddress
             << ':' << options_.port << " (" << LastSocketError() << ")\n";
    }
    return 1;
  }
  {
    std::lock_guard<std::mutex> lock(impl_->socketMutex);
    impl_->listenSocket = listenSocket;
  }

  const auto shouldStop = [&]() {
    return impl_->stopped.load(std::memory_order_relaxed) ||
           (externalStop != nullptr &&
            externalStop->load(std::memory_order_relaxed));
  };

  while (!shouldStop()) {
    {
      std::lock_guard<std::mutex> lock(impl_->workerMutex);
      for (auto it = impl_->workers.begin(); it != impl_->workers.end();) {
        if (it->done->load(std::memory_order_acquire)) {
          if (it->thread.joinable()) {
            it->thread.join();
          }
          it = impl_->workers.erase(it);
        } else {
          ++it;
        }
      }
    }
    sockaddr_in peer{};
#ifdef _WIN32
    int peerLength = sizeof(peer);
#else
    socklen_t peerLength = sizeof(peer);
#endif
    const Socket client = accept(listenSocket,
                                 reinterpret_cast<sockaddr *>(&peer),
                                 &peerLength);
    if (client == kInvalidSocket) {
      if (shouldStop()) {
        break;
      }
      continue;
    }
    SetSocketTimeouts(client);

    constexpr std::size_t kMaxConcurrentWorkers = 32;
    const std::size_t active =
        impl_->activeWorkers.fetch_add(1, std::memory_order_acq_rel) + 1;
    if (active > kMaxConcurrentWorkers) {
      impl_->activeWorkers.fetch_sub(1, std::memory_order_acq_rel);
      const HttpResponse busy = JsonHttpError(503, "busy", "too many concurrent HTTP requests");
      WriteHttpResponse(client, busy, false);
      (void)CloseSocket(client);
      continue;
    }

    auto done = std::make_shared<std::atomic<bool>>(false);
    std::thread worker([this, &server, baseContext, client, error,
                        done]() mutable {
      struct Completion final {
        std::shared_ptr<std::atomic<bool>> flag;
        ~Completion() { flag->store(true, std::memory_order_release); }
      } completion{done};
      HttpResponse response;
      HttpRequest request;
      bool sse = false;
      if (!ParseRequest(client, options_, request, response)) {
        WriteHttpResponse(client, response, false);
      } else if (request.path != options_.path) {
        response = JsonHttpError(404, "not_found", "MCP endpoint not found");
        WriteHttpResponse(client, response, false);
      } else if (request.method != "POST") {
        response = JsonHttpError(405, "method_not_allowed",
                                 "only POST /mcp is supported");
        response.contentType = "application/json";
        WriteHttpResponse(client, response, false);
      } else if (!IsAllowedOrigin(options_, request)) {
        response = JsonHttpError(403, "origin_denied", "Origin is not allowlisted");
        WriteHttpResponse(client, response, false);
      } else if (!IsAuthorized(options_, request)) {
        response = JsonHttpError(401, "unauthorized", "Bearer token is invalid or missing");
        WriteHttpResponse(client, response, false);
      } else {
        const auto acceptIt = request.headers.find("accept");
        if (acceptIt == request.headers.end() ||
            (!ContainsToken(acceptIt->second, "application/json") &&
             !ContainsToken(acceptIt->second, "text/event-stream"))) {
          response = JsonHttpError(415, "invalid_accept",
                                   "Accept must include application/json or text/event-stream");
          WriteHttpResponse(client, response, false);
        } else {
          Json body;
          try {
            body = Json::parse(request.body);
          } catch (const std::exception &exception) {
            response = JsonHttpError(400, "parse_error", exception.what());
            WriteHttpResponse(client, response, false);
            (void)CloseSocket(client);
            impl_->activeWorkers.fetch_sub(1, std::memory_order_acq_rel);
            return;
          }
          if (!ValidateMirroredHeaders(options_, request, body, response)) {
            WriteHttpResponse(client, response, false);
          } else {
            RequestContext context = baseContext;
            const auto clientIdIt = request.headers.find("mcp-client-id");
            if (clientIdIt != request.headers.end()) {
              context.clientId = clientIdIt->second;
            }
            if (body.is_object() && body.contains("params") &&
                body["params"].is_object() && body["params"].contains("_meta")) {
              context.metadata = body["params"]["_meta"];
            }
            const std::string result = server.HandleLine(request.body, context);
            const bool wantsSse =
                acceptIt != request.headers.end() &&
                !ContainsToken(acceptIt->second, "application/json") &&
                ContainsToken(acceptIt->second, "text/event-stream");
            sse = wantsSse;
            response.body = wantsSse ? "event: message\ndata: " + result + "\n\n"
                                     : result;
            response.contentType = wantsSse ? "text/event-stream" : "application/json";
            WriteHttpResponse(client, response, wantsSse);
          }
        }
      }
      (void)error;
      (void)sse;
      (void)CloseSocket(client);
      impl_->activeWorkers.fetch_sub(1, std::memory_order_acq_rel);
    });
    std::lock_guard<std::mutex> lock(impl_->workerMutex);
    impl_->workers.push_back(Impl::Worker{std::move(worker), std::move(done)});
  }

  Stop();
  {
    std::lock_guard<std::mutex> lock(impl_->workerMutex);
    for (Impl::Worker &worker : impl_->workers) {
      if (worker.thread.joinable()) {
        worker.thread.join();
      }
    }
    impl_->workers.clear();
  }
#ifdef _WIN32
  WSACleanup();
#endif
  impl_->running.store(false);
  return 0;
}

} // namespace unityexplorer::mcp
