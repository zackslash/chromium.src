// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "headless/lib/browser/headless_devtools.h"

#include "base/files/file_path.h"
#include "components/devtools_http_handler/devtools_http_handler.h"
#include "components/devtools_http_handler/devtools_http_handler_delegate.h"
#include "content/public/browser/browser_context.h"
#include "content/public/browser/devtools_frontend_host.h"
#include "content/public/browser/navigation_entry.h"
#include "headless/lib/browser/headless_browser_context.h"
#include "net/base/net_errors.h"
#include "net/socket/tcp_server_socket.h"
#include "ui/base/resource/resource_bundle.h"

using devtools_http_handler::DevToolsHttpHandler;

namespace headless {

namespace {

const int kBackLog = 10;

class TCPServerSocketFactory : public DevToolsHttpHandler::ServerSocketFactory {
 public:
  TCPServerSocketFactory(const net::IPEndPoint& endpoint)
      : endpoint_(endpoint) {
    DCHECK(endpoint_.address().IsValid());
  }

 private:
  // DevToolsHttpHandler::ServerSocketFactory implementation:
  scoped_ptr<net::ServerSocket> CreateForHttpServer() override {
    scoped_ptr<net::ServerSocket> socket(
        new net::TCPServerSocket(nullptr, net::NetLog::Source()));
    if (socket->Listen(endpoint_, kBackLog) != net::OK)
      return scoped_ptr<net::ServerSocket>();

    return socket;
  }

  net::IPEndPoint endpoint_;

  DISALLOW_COPY_AND_ASSIGN(TCPServerSocketFactory);
};

class HeadlessDevToolsDelegate
    : public devtools_http_handler::DevToolsHttpHandlerDelegate {
 public:
  HeadlessDevToolsDelegate();
  ~HeadlessDevToolsDelegate() override;

  // devtools_http_handler::DevToolsHttpHandlerDelegate implementation:
  std::string GetDiscoveryPageHTML() override;
  std::string GetFrontendResource(const std::string& path) override;
  std::string GetPageThumbnailData(const GURL& url) override;
  content::DevToolsExternalAgentProxyDelegate* HandleWebSocketConnection(
      const std::string& path) override;

 private:
  DISALLOW_COPY_AND_ASSIGN(HeadlessDevToolsDelegate);
};

HeadlessDevToolsDelegate::HeadlessDevToolsDelegate() {}

HeadlessDevToolsDelegate::~HeadlessDevToolsDelegate() {}

std::string HeadlessDevToolsDelegate::GetDiscoveryPageHTML() {
  return std::string();
}

std::string HeadlessDevToolsDelegate::GetFrontendResource(
    const std::string& path) {
  return content::DevToolsFrontendHost::GetFrontendResource(path).as_string();
}

std::string HeadlessDevToolsDelegate::GetPageThumbnailData(const GURL& url) {
  return std::string();
}

content::DevToolsExternalAgentProxyDelegate*
HeadlessDevToolsDelegate::HandleWebSocketConnection(const std::string& path) {
  return nullptr;
}

}  // namespace

scoped_ptr<DevToolsHttpHandler> CreateLocalDevToolsHttpHandler(
    HeadlessBrowserContext* browser_context) {
  const net::IPEndPoint& endpoint =
      browser_context->options().devtools_endpoint;
  scoped_ptr<DevToolsHttpHandler::ServerSocketFactory> socket_factory(
      new TCPServerSocketFactory(endpoint));
  return make_scoped_ptr(new DevToolsHttpHandler(
      std::move(socket_factory), std::string(), new HeadlessDevToolsDelegate(),
      browser_context->GetPath(), base::FilePath(), std::string(),
      browser_context->options().user_agent));
}

}  // namespace headless
