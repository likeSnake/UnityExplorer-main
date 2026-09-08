#include "mcp/mcp_server.hpp"

#include <exception>

namespace unityexplorer::mcp {
namespace {

bool IsInteger(const Json &value) {
  return value.is_number_integer() || value.is_number_unsigned();
}

std::string JsonTypeName(const Json &value) {
  if (value.is_object()) {
    return "object";
  }
  if (value.is_array()) {
    return "array";
  }
  if (value.is_string()) {
    return "string";
  }
  if (value.is_boolean()) {
    return "boolean";
  }
  if (value.is_number()) {
    return "number";
  }
  if (value.is_null()) {
    return "null";
  }
  return "unknown";
}

} // namespace

ToolResult ToolResult::Success(Json data, std::string text) {
  ToolResult result;
  result.isError = false;
  result.structuredContent = Json{{"ok", true}, {"data", std::move(data)}};
  if (text.empty()) {
    text = result.structuredContent.dump();
  }
  result.text = std::move(text);
  result.content = Json::array({Json{{"type", "text"},
                                     {"text", result.text}}});
  return result;
}

ToolResult ToolResult::Failure(std::string code, std::string message,
                               Json data) {
  ToolResult result;
  result.isError = true;
  result.structuredContent =
      Json{{"ok", false},
           {"error", Json{{"code", std::move(code)},
                           {"message", std::move(message)}}},
           {"data", std::move(data)}};
  result.text = result.structuredContent.dump();
  result.content = Json::array({Json{{"type", "text"},
                                     {"text", result.text}}});
  return result;
}

Json ToolResult::ToMcpResult() const {
  Json result = Json{{"content", content},
                     {"structuredContent", structuredContent},
                     {"isError", isError}};
  return result;
}

McpServer::McpServer(McpServerOptions options) : options_(std::move(options)) {}

bool McpServer::RegisterTool(ToolDefinition definition) {
  if (definition.name.empty() || !definition.handler) {
    return false;
  }
  if (!definition.inputSchema.is_object()) {
    definition.inputSchema = Json{{"type", "object"},
                                  {"properties", Json::object()},
                                  {"additionalProperties", false}};
  }
  if (!definition.outputSchema.is_object()) {
    definition.outputSchema = Json{{"type", "object"}};
  }
  tools_[definition.name] = std::move(definition);
  return true;
}

bool McpServer::RemoveTool(const std::string &name) {
  return tools_.erase(name) != 0;
}

void McpServer::ClearTools() { tools_.clear(); }

std::vector<ToolDefinition> McpServer::ListToolDefinitions() const {
  std::vector<ToolDefinition> definitions;
  definitions.reserve(tools_.size());
  for (const auto &entry : tools_) {
    definitions.push_back(entry.second);
  }
  return definitions;
}

Json McpServer::MakeErrorResponse(const Json &id, int code,
                                  const std::string &message,
                                  Json data) const {
  Json error = Json{{"code", code}, {"message", message}};
  if (!data.is_null() && !(data.is_object() && data.empty())) {
    error["data"] = std::move(data);
  }
  return Json{{"jsonrpc", "2.0"}, {"id", id}, {"error", std::move(error)}};
}

Json McpServer::MakeResultResponse(const Json &id, Json result) const {
  return Json{{"jsonrpc", "2.0"}, {"id", id}, {"result", std::move(result)}};
}

Json McpServer::BuildCapabilities() const {
  Json capabilities = Json{{"tools", Json::object()}};
  if (options_.exposeResourcesCapability) {
    capabilities["resources"] = Json::object();
  }
  if (options_.exposePromptsCapability) {
    capabilities["prompts"] = Json::object();
  }
  return capabilities;
}

Json McpServer::BuildDiscoverResult() const {
  return Json{{"protocolVersion", options_.serverInfo.protocolVersion},
              {"serverInfo", Json{{"name", options_.serverInfo.name},
                                   {"version", options_.serverInfo.version}}},
              {"capabilities", BuildCapabilities()}};
}

Json McpServer::BuildInitializeResult(const Json & /*params*/) const {
  return BuildDiscoverResult();
}

Json McpServer::BuildToolsListResult() const {
  Json list = Json::array();
  for (const auto &entry : tools_) {
    const ToolDefinition &tool = entry.second;
    Json item{{"name", tool.name},
              {"description", tool.description},
              {"inputSchema", tool.inputSchema}};
    if (!tool.outputSchema.is_null() && !tool.outputSchema.empty()) {
      item["outputSchema"] = tool.outputSchema;
    }
    list.push_back(std::move(item));
  }
  return Json{{"tools", std::move(list)}, {"resultType", "complete"}};
}

bool McpServer::IsValidRequestId(const Json &id) {
  return id.is_null() || id.is_string() || IsInteger(id) || id.is_number_float();
}

bool McpServer::ValidateSchema(const Json &value, const Json &schema,
                               std::string &error, const std::string &path) {
  if (!schema.is_object()) {
    return true;
  }

  if (schema.contains("type") && schema["type"].is_string()) {
    const std::string type = schema["type"].get<std::string>();
    bool matches = false;
    if (type == "object") {
      matches = value.is_object();
    } else if (type == "array") {
      matches = value.is_array();
    } else if (type == "string") {
      matches = value.is_string();
    } else if (type == "integer") {
      matches = IsInteger(value);
    } else if (type == "number") {
      matches = value.is_number();
    } else if (type == "boolean") {
      matches = value.is_boolean();
    } else if (type == "null") {
      matches = value.is_null();
    } else {
      // Unknown extension types are left to the backend.  This keeps the core
      // forward-compatible with richer JSON Schema dialects.
      matches = true;
    }
    if (!matches) {
      error = path + " must be " + type + ", got " + JsonTypeName(value);
      return false;
    }
  }

  if (schema.contains("enum") && schema["enum"].is_array()) {
    bool found = false;
    for (const Json &candidate : schema["enum"]) {
      if (candidate == value) {
        found = true;
        break;
      }
    }
    if (!found) {
      error = path + " is not one of the allowed values";
      return false;
    }
  }

  if (value.is_object()) {
    if (schema.contains("required") && schema["required"].is_array()) {
      for (const Json &required : schema["required"]) {
        if (!required.is_string()) {
          continue;
        }
        const std::string key = required.get<std::string>();
        if (!value.contains(key)) {
          error = path + " is missing required property '" + key + "'";
          return false;
        }
      }
    }

    const Json properties = schema.value("properties", Json::object());
    const bool hasProperties = properties.is_object();
    const bool additionalAllowed =
        schema.value("additionalProperties", true);
    for (auto it = value.begin(); it != value.end(); ++it) {
      if (hasProperties && properties.contains(it.key())) {
        if (!ValidateSchema(it.value(), properties.at(it.key()), error,
                            path + "." + it.key())) {
          return false;
        }
      } else if (!additionalAllowed) {
        error = path + " has unknown property '" + it.key() + "'";
        return false;
      }
    }
  }

  if (value.is_array() && schema.contains("items")) {
    std::size_t index = 0;
    for (const Json &item : value) {
      if (!ValidateSchema(item, schema["items"], error,
                          path + "[" + std::to_string(index) + "]")) {
        return false;
      }
      ++index;
    }
  }

  if (value.is_string()) {
    if (schema.contains("minLength") && schema["minLength"].is_number_unsigned() &&
        value.get<std::string>().size() < schema["minLength"].get<std::size_t>()) {
      error = path + " is shorter than minLength";
      return false;
    }
  }

  return true;
}

bool McpServer::HasRequiredScopes(const RequestContext &context,
                                  const ToolDefinition &tool,
                                  std::string &missingScope) {
  for (const std::string &scope : tool.requiredScopes) {
    if (context.scopes.find(scope) == context.scopes.end()) {
      missingScope = scope;
      return false;
    }
  }
  return true;
}

RequestContext McpServer::ContextFromRequest(const Json &request,
                                             const RequestContext &base) {
  RequestContext context = base;
  if (!request.contains("params") || !request["params"].is_object()) {
    return context;
  }
  const Json &params = request["params"];
  if (params.contains("_meta") && params["_meta"].is_object()) {
    context.metadata = params["_meta"];
  }
  return context;
}

Json McpServer::HandleRequest(const Json &request,
                              const RequestContext &context) const {
  if (request.is_array()) {
    if (request.empty()) {
      return MakeErrorResponse(nullptr, -32600, "Invalid Request");
    }
    Json responses = Json::array();
    for (const Json &entry : request) {
      const Json response = HandleRequest(entry, context);
      if (!response.is_null()) {
        responses.push_back(response);
      }
    }
    return responses.empty() ? Json() : responses;
  }
  if (!request.is_object()) {
    return MakeErrorResponse(nullptr, -32600, "Invalid Request");
  }

  const bool hasId = request.contains("id");
  const Json id = hasId ? request.at("id") : Json(nullptr);
  if (hasId && !IsValidRequestId(id)) {
    return hasId ? MakeErrorResponse(id, -32600, "Invalid Request") : Json();
  }
  if (!request.contains("jsonrpc") || request["jsonrpc"] != "2.0" ||
      !request.contains("method") || !request["method"].is_string()) {
    return hasId ? MakeErrorResponse(id, -32600, "Invalid Request") : Json();
  }

  const std::string method = request["method"].get<std::string>();
  const RequestContext requestContext = ContextFromRequest(request, context);
  const Json params = request.value("params", Json::object());
  auto protocolFailureWithData = [&](int code, const std::string &message,
                                     Json data) -> Json {
    return hasId ? MakeErrorResponse(id, code, message, std::move(data))
                 : Json();
  };
  auto protocolFailure = [&](int code, const std::string &message) -> Json {
    return protocolFailureWithData(code, message, Json::object());
  };
  auto success = [&](Json result) -> Json {
    return hasId ? MakeResultResponse(id, std::move(result)) : Json();
  };

  if (method == "notifications/initialized" || method == "initialized") {
    return Json();
  }
  if (method == "ping") {
    return success(Json::object());
  }
  if (method == "server/discover") {
    if (!params.is_object()) {
      return protocolFailure(-32602, "params must be an object");
    }
    return success(BuildDiscoverResult());
  }
  if (method == "initialize") {
    if (!options_.allowLegacyInitialize) {
      return protocolFailure(-32601, "Method not found");
    }
    if (!params.is_object()) {
      return protocolFailure(-32602, "params must be an object");
    }
    return success(BuildInitializeResult(params));
  }
  if (method == "tools/list") {
    if (!params.is_object()) {
      return protocolFailure(-32602, "params must be an object");
    }
    return success(BuildToolsListResult());
  }
  if (method == "tools/call") {
    if (!params.is_object() || !params.contains("name") ||
        !params["name"].is_string()) {
      return protocolFailure(-32602,
                             "tools/call params.name must be a string");
    }
    const std::string name = params["name"].get<std::string>();
    const auto toolIt = tools_.find(name);
    if (toolIt == tools_.end()) {
      return protocolFailure(-32601, "Unknown tool: " + name);
    }
    const ToolDefinition &tool = toolIt->second;
    Json arguments = params.value("arguments", Json::object());
    if (!arguments.is_object()) {
      return protocolFailure(-32602, "tools/call arguments must be an object");
    }
    std::string schemaError;
    if (!ValidateSchema(arguments, tool.inputSchema, schemaError, "$")) {
      return protocolFailure(-32602, schemaError);
    }
    std::string missingScope;
    if (!HasRequiredScopes(requestContext, tool, missingScope)) {
      // Authorization failures are protocol-level failures because the tool
      // was not executed and its name/arguments were otherwise valid.
      return protocolFailureWithData(
          -32602, "permission denied: missing scope " + missingScope,
          Json{{"code", "permission_denied"}, {"scope", missingScope}});
    }
    try {
      return success(tool.handler(arguments, requestContext).ToMcpResult());
    } catch (const std::exception &exception) {
      return success(ToolResult::Failure("internal_error", exception.what())
                         .ToMcpResult());
    } catch (...) {
      return success(ToolResult::Failure("internal_error",
                                         "tool handler threw an unknown exception")
                         .ToMcpResult());
    }
  }

  return protocolFailure(-32601, "Method not found: " + method);
}

std::string McpServer::HandleLine(const std::string &line,
                                  const RequestContext &context) const {
  if (line.empty()) {
    return {};
  }
  if (line.size() > options_.maxLineBytes) {
    return MakeErrorResponse(nullptr, -32600,
                             "Request exceeds configured line limit")
        .dump();
  }

  Json request;
  try {
    request = Json::parse(line);
  } catch (const std::exception &exception) {
    return MakeErrorResponse(nullptr, -32700, "Parse error",
                             Json{{"detail", exception.what()}})
        .dump();
  }

  const Json response = HandleRequest(request, context);
  return response.is_null() ? std::string{} : response.dump();
}

} // namespace unityexplorer::mcp
