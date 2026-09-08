#include "mcp/mcp_server.hpp"
#include "mcp/stdio_transport.hpp"

#include <iostream>
#include <sstream>

namespace {

bool Expect(bool condition, const char *message) {
  if (condition) {
    return true;
  }
  std::cerr << "[mcp_protocol] FAIL: " << message << '\n';
  return false;
}

} // namespace

int main() {
  using namespace unityexplorer::mcp;
  bool ok = true;

  McpServerOptions options;
  options.serverInfo.name = "SmokeServer";
  options.serverInfo.version = "test";
  McpServer server(options);
  ok &= Expect(server.RegisterTool(ToolDefinition{
                          "echo", "Echo one message",
                          Json{{"type", "object"},
                               {"properties", Json{{"message", Json{{"type", "string"}}}}},
                               {"required", Json::array({"message"})},
                               {"additionalProperties", false}},
                          Json{{"type", "object"}}, {"read"},
                          [](const Json &args, const RequestContext &) {
                            return ToolResult::Success(
                                Json{{"echo", args.at("message")}});
                          }}),
                     "echo tool should register");

  RequestContext readContext;
  readContext.clientId = "test-client";
  readContext.scopes.insert("read");

  Json discover = Json::parse(server.HandleLine(
      R"({"jsonrpc":"2.0","id":1,"method":"server/discover","params":{}})",
      readContext));
  ok &= Expect(discover["result"]["serverInfo"]["name"] == "SmokeServer",
               "discover should return server info");

  Json listed = Json::parse(server.HandleLine(
      R"({"jsonrpc":"2.0","id":"list","method":"tools/list","params":{}})",
      readContext));
  ok &= Expect(listed["result"]["tools"].size() == 1,
               "tools/list should return one tool");

  Json called = Json::parse(server.HandleLine(
      R"({"jsonrpc":"2.0","id":2,"method":"tools/call","params":{"name":"echo","arguments":{"message":"hello"}}})",
      readContext));
  ok &= Expect(called["result"]["isError"] == false,
               "echo should succeed");
  ok &= Expect(called["result"]["structuredContent"]["data"]["echo"] ==
                   "hello",
               "echo result should be structured");

  Json invalid = Json::parse(server.HandleLine(
      R"({"jsonrpc":"2.0","id":3,"method":"tools/call","params":{"name":"echo","arguments":{"message":"hello","extra":true}}})",
      readContext));
  ok &= Expect(invalid["error"]["code"] == -32602,
               "unknown argument should fail schema validation");

  RequestContext noScope;
  Json denied = Json::parse(server.HandleLine(
      R"({"jsonrpc":"2.0","id":4,"method":"tools/call","params":{"name":"echo","arguments":{"message":"hello"}}})",
      noScope));
  ok &= Expect(denied["error"]["data"]["code"] == "permission_denied",
               "missing scope should be denied");

  ok &= Expect(server.HandleLine(
                   R"({"jsonrpc":"2.0","method":"notifications/initialized"})",
                   readContext)
                   .empty(),
               "notification should not produce a response");

  const std::string batch = server.HandleLine(
      R"([{"jsonrpc":"2.0","id":5,"method":"ping"},{"jsonrpc":"2.0","method":"notifications/initialized"}])",
      readContext);
  Json batchResult = Json::parse(batch);
  ok &= Expect(batchResult.is_array() && batchResult.size() == 1,
               "batch should omit notification response");

  std::istringstream input(
      R"json({"jsonrpc":"2.0","id":6,"method":"ping"}
{"jsonrpc":"2.0","method":"notifications/initialized"}
)json");
  std::ostringstream output;
  std::ostringstream errors;
  StdioTransport transport;
  ok &= Expect(transport.Run(server, readContext, input, output, errors) == 0,
               "stdio transport should drain input");
  ok &= Expect(output.str().find("\"id\":6") != std::string::npos,
               "stdio transport should frame response by newline");
  ok &= Expect(errors.str().empty(), "stdio transport should keep stderr clean");

  return ok ? 0 : 1;
}
