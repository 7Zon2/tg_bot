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
#include <lexbor/html/base.h>
#include <lexbor/core/types.h>
#include <lexbor/dom/collection.h>
#include <lexbor/dom/interface.h>
#include <lexbor/dom/interfaces/element.h>
#include <lexbor/dom/interfaces/node.h>
#include <lexbor/html/interfaces/document.h>
#include <lexbor/html/parser.h>
#include <lexbor/html/serialize.h>

#include <stdexcept>
#include <exception>
#include <string>
#include <vector>

#include "boost/interprocess/detail/os_file_functions.hpp"
#include "boost/interprocess/file_mapping.hpp"
#include "boost/interprocess/mapped_region.hpp"

#include "boost/asio/any_io_executor.hpp"
#include "boost/asio/buffer.hpp"
#include "boost/asio/detached.hpp"
#include "boost/asio/execution_context.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/this_coro.hpp"
#include "boost/asio/awaitable.hpp"
#include "boost/asio/co_spawn.hpp"

#include "boost/beast/core/flat_buffer.hpp"
#include "boost/beast/http/write.hpp"
#include "boost/beast/core/detail/base64.hpp"
#include "boost/beast/core/tcp_stream.hpp"
#include "boost/beast/http/status.hpp"
#include "boost/beast/http/message.hpp"
#include "boost/beast/http/status.hpp"
#include "boost/beast/http/string_body.hpp"
#include "boost/beast/http/verb.hpp"

#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/device/array.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/device/back_inserter.hpp"

#include "boost/system/detail/error_code.hpp"
#include "boost/json/string.hpp"
#include "boost/json/string_view.hpp"
#include "json_head.hpp"
#include "certif.hpp"


 namespace beast = boost::beast;         // from <boost/beast.hpp>
 namespace http = beast::http;           // from <boost/beast/http.hpp>
 namespace net = boost::asio;            // from <boost/asio.hpp>
 namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
 using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


[[nodiscard]]
inline json::string 
load_file(json::string_view filename)
{
  print("\nload file: ",filename,"\n");
  boost::interprocess::file_mapping map{filename.data(), boost::interprocess::read_only};
  boost::interprocess::mapped_region region{map, boost::interprocess::read_only};

  char * addr = static_cast<char*>(region.get_address());
  size_t size = region.get_size();

  json::string data{size, ' '};
  std::copy(addr, addr+size, data.data());
  return data;
}


inline void 
store_file
(json::string_view filename, const char * data, size_t sz)
{
  print("\nstore file: ", filename,"\n");
  std::fstream ff;
  ff.open(filename, std::ios::out|std::ios::binary);
  ff.write(data, sz);
}


template<typename T>
[[nodiscard]]
inline T 
filter_by_extension(T&& cont, std::string_view ext)
{
  T temp{};
  for(size_t i = 0; i < cont.size(); i++)
  {
    std::string_view view = cont[i];
    std::string_view r{view.end() - ext.size(), view.end()};
    if(r == ext)
    {
      temp.push_back(Utils::forward_like<T>(cont[i]));
    }
  }
  return temp;
}


[[nodiscard]]
inline auto 
parse_html(json::string_view html, json::string_view tag ="href", json::string_view value = "http")
{
  print("\n\nStart parsing html...\n\n");

  std::pmr::vector<json::string> vec;

  auto filter = [&vec,&value](json::string_view val)
  {
    if (value.size() > val.size())
      return;

    json::string_view view{val.begin(), val.begin() + value.size()};
    if(view == value)
      vec.push_back(val);
  };

  using unique_html = std::unique_ptr
  <lxb_html_document_t, decltype([](auto* p){lxb_html_document_destroy(p);})>;

  using unique_coll = std::unique_ptr
  <lxb_dom_collection_t, decltype([](auto*p){lxb_dom_collection_destroy(p,true);})>;


  print("\nCreating HTML parser...\n");
  lxb_html_parser_t* parser = lxb_html_parser_create();
  lxb_status_t status = lxb_html_parser_init(parser);
  if(status != LXB_STATUS_OK)
  {
    throw std::runtime_error{"\nFailed to create HTML parser\n"};
  }

  const unsigned char * data = reinterpret_cast<const unsigned char*>(html.data());
  const size_t sz = html.size() - 1;

  print("\nParsing html document...\n");
  unique_html  doc{lxb_html_parse(parser, data , sz)};
  if(doc == nullptr)
  {
    throw std::runtime_error{"\nFaield to create Document object\n"};
  }

  unique_coll collection{lxb_dom_collection_make(&doc->dom_document, 128)};
  auto * body = lxb_html_document_body_element(doc.get());
  auto * element = lxb_dom_interface_element(body);
  status = lxb_dom_elements_by_attr_contain
  (
      element, 
      collection.get(), 
      (const lxb_char_t*)tag.data(), tag.size(),
      (const lxb_char_t*)value.data(),value.size(),
      true
  );
  if(status != LXB_STATUS_OK)
  {
    throw std::runtime_error{"\nFailed to get elements by tag name\n"};
  }

  for(size_t i=0;i<lxb_dom_collection_length(collection.get()); i++)
  {
    element = lxb_dom_collection_element(collection.get(), i);
    auto  * fdata = (const char*) element->first_attr->value->data;
    filter(fdata);
  }

  return vec;
}


[[nodiscard]]
inline size_t 
skip_http (json::string_view v) noexcept
{
  const static json::string barrier{"://"};
  size_t pos = v.find(barrier);
  if(pos != json::string::npos)
  {
    return pos + barrier.size()+1;
  }
  return 0;
};


[[nodiscard]]
inline json::string
make_host(json::string_view url, json::string_view pattern = "/")
{
  size_t skip_pos = skip_http(url);
  size_t pos = url.find(pattern, skip_pos);
  if(pos != json::string::npos)
  {
    return json::string{url.begin()+skip_pos-1,url.begin()+pos};
  }
  return {};
}


[[nodiscard]]
inline json::string 
make_relative_url(const json::string& url, json::string_view pattern = "/")
{
  size_t skip_pos = skip_http(url);
  size_t pos = url.find(pattern, skip_pos);
  json::string temp{};
  if(pos != json::string::npos)
  {
    temp = json::string(url.begin()+pos, url.end()); 
  }

  return temp;
}


inline void 
print_response(auto&& res)
{
  auto body = std::move(res).body();
  print("Response:\n\n====================================================================================\n");
  print(res);
  res.body() = std::move(body);
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
