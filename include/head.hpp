#pragma once
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/beast/core/buffers_generator.hpp>
#include <boost/config.hpp>
#include <lexbor/core/base.h>
#include <lexbor/core/types.h>
#include <lexbor/html/interfaces/document.h>
#include <lexbor/html/parser.h>
#include <lexbor/html/serialize.h>
#include <stdexcept>
#include "boost/beast/http/status.hpp"
#include <lexbor/html/html.h>
#include "json_head.hpp"


 namespace beast = boost::beast;         // from <boost/beast.hpp>
 namespace http = beast::http;           // from <boost/beast/http.hpp>
 namespace net = boost::asio;            // from <boost/asio.hpp>
 namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
 using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>




[[nodiscard]]
inline auto 
parse_html(json::string_view html)
{
  auto callback = [](const lxb_char_t *data, size_t len, void* ctx) -> lxb_status_t
  {
    printf("%.*s", (int)len, (const char *) data);
    return LXB_STATUS_OK;
  };

  using unique_html = std::unique_ptr
  <lxb_html_document_t, decltype([](auto* p){lxb_html_document_destroy(p);})>;

  lxb_html_parser_t* parser = lxb_html_parser_create();
  lxb_status_t status = lxb_html_parser_init(parser);
  if(status != LXB_STATUS_OK)
    throw std::runtime_error{"\nFailed to create HTML parser\n"};

  const unsigned char * data = reinterpret_cast<const unsigned char*>(html.data());
  const size_t sz = html.size();

  unique_html  doc{lxb_html_parse(parser, data , sz)};
  if(doc == nullptr)
    throw std::runtime_error{"\nFaield to create Document object\n"};

  lxb_dom_node_t * node = lxb_dom_interface_node(doc.get());
  lxb_html_serialize_cb(node, callback, nullptr);

  return doc;
}


[[nodiscard]]
inline json::string 
make_relative_url(json::string&& url, json::string pattern = {"://"}) noexcept
{
  size_t pos = url.find(pattern);
  if(pos != json::string::npos)
  {
    url.erase(0, pos); 
  }

  return std::move(url);
}



inline void 
print_response(auto&& res, bool only_headers = false)
{
  print("Response:\n\n====================================================================================\n");
  for(auto&& i : res)
  {
    print
    (
      "field name: ",  i.name_string(),"\t",
      "field value: ", i.value(),"\n"
    );
  }
  print(res,"\n=========================================================================================\n");
}


inline void print_endpoint(const tcp::endpoint& ep)
{
    print("address:", ep.address().to_string(),"\n");
    print("port:", ep.port(),"\n");
    print("protocol:", ep.protocol().protocol(),"\n");
    print("protocol type:", ep.protocol().type(), "\n");
    print("protocol family:", ep.protocol().family(),"\n");
}


inline void print_result_type(const tcp::resolver::results_type& res)
{
    print("\nserver endpoints:\n");
    print("number of endpoints:", res.size(),"\n");
    for(auto && i : res)
    {
        print("\nendpoint:\n");
        print("service_name:",i.service_name(),"\n");
        print("host_name:", i.host_name(),"\n");
        print_endpoint(i.endpoint());
    }
}


enum class RequestErrors : char
{
    bad_request,
    not_found,
    server_error,
    ok
};


http::message_generator
inline prepare_response
(
    RequestErrors status
)
{

    // Returns a bad request response
    auto const bad_request =
    [&](beast::string_view why)
    {
        http::response<http::string_body> res{http::status::bad_request, 11};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(true);
        res.body() = std::string(why);
        res.prepare_payload();
        return res;
    };


    // Returns a not found response
    auto const not_found =
    [&](beast::string_view what)
    {
        http::response<http::string_body> res{http::status::not_found, 11};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(true);
        res.body() = std::string(what);
        res.prepare_payload();
        return res;
    };


    // Returns a server error response
    auto const server_error =
    [](beast::string_view what)
    {
        http::response<http::string_body> res{http::status::internal_server_error, 11};
        res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
        res.set(http::field::content_type, "text/html");
        res.keep_alive(true);
        res.body() = std::string(what);
        res.prepare_payload();
        return res;
    };


    auto const server_ok =
    []()
    {
      http::response<http::empty_body> res{http::status::ok, 11};
      print(res);
      return res;
    };


    switch (status)
    {
        case RequestErrors::bad_request  : return bad_request("Request is wrong");
        
        case RequestErrors::not_found    : return not_found("Requested file is not found");

        case RequestErrors::server_error : return server_error("Internal server error");

        case RequestErrors::ok : return server_ok();

        default: 
            return server_error("Unknown error");
    }

}
