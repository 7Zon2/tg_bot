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
#include <ios>
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
#include "boost/beast/http/status.hpp"
#include "boost/beast/http/message.hpp"
#include "boost/beast/http/status.hpp"
#include "boost/beast/http/string_body.hpp"

#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/device/array.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/device/back_inserter.hpp"

#include "boost/json/string.hpp"
#include "boost/json/string_view.hpp"
#include "json_head.hpp"
#include "certif.hpp"


 namespace beast = boost::beast;         // from <boost/beast.hpp>
 namespace http = beast::http;           // from <boost/beast/http.hpp>
 namespace net = boost::asio;            // from <boost/asio.hpp>
 namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
 using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>



template<typename T>
concept is_http_message = requires(T&& mes)
{
  typename std::decay_t<T>::fields_type;
  typename std::decay_t<T>::header_type;
  typename std::decay_t<T>::body_type;
};


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
(json::string_view filename, const char * data, size_t sz, bool append)
{
  std::fstream ff;

  std::ios::openmode mode = std::ios::out | std::ios::binary;
  if(append)
  {
    mode|=std::ios::app;
  }

  ff.open(filename, mode);
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


enum class RequestErrors
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
