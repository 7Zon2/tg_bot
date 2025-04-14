#pragma once
#include "json_head.hpp"
#include "print.hpp"
#include "head.hpp"
#include "boost/interprocess/detail/os_file_functions.hpp"
#include "boost/interprocess/file_mapping.hpp"
#include "boost/interprocess/mapped_region.hpp"
#include "boost/asio/any_io_executor.hpp"
#include "boost/asio/buffer.hpp"
#include "boost/asio/detached.hpp"
#include "boost/asio/execution_context.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/ssl/context.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/asio/this_coro.hpp"
#include "boost/asio/use_awaitable.hpp"
#include "boost/beast.hpp"
#include "boost/beast/core/flat_buffer.hpp"
#include "boost/beast/core/static_buffer.hpp"
#include "boost/beast/http/status.hpp"
#include "boost/beast/http/write.hpp"
#include "boost/beast/core/detail/base64.hpp"
#include "boost/asio/co_spawn.hpp"
#include "boost/asio/awaitable.hpp"
#include "boost/beast/http/message.hpp"
#include "boost/beast/http/string_body.hpp"
#include "boost/beast/http/verb.hpp"



class session_interface
{
  protected: 

  json::string host_;
  json::string port_;
  json::string certif_;
  int version_;

  ssl::context ctx_;
  boost::asio::any_io_executor ex_;
  tcp::resolver resolver_;

  using stream_type = boost::asio::ssl::stream<beast::tcp_stream>;
  using stream_ptr = std::shared_ptr<stream_type>;
  stream_ptr stream_;

  protected:

  net::awaitable<void>
  virtual resolve()
  {
    print("Resolution...\n\n");
    tcp::resolver::results_type res = co_await resolver_.async_resolve(host_, port_);
    print_result_type(res);
    co_await connect(res);
  }

  
  net::awaitable<void>
  virtual connect
  (const tcp::resolver::results_type& res)
  {
    // Make the connection on the IP address we get from a lookup
    auto ep = co_await beast::get_lowest_layer(*stream_).async_connect(res);
  
    print("Connecting...\n\n", "connected endpoint:\n");
    print_endpoint(ep);
    print("\n\n");

    co_await handshake();
  }


  net::awaitable<void>
  virtual handshake()
  {
    print("Handshake...\n");
   
    boost::asio::mutable_buffer buf;
    json::string str;
    co_await stream_->async_handshake(ssl::stream_base::client, buf, net::use_awaitable);
    
    for(size_t i = 0; i < buf.size(); i++)
    {
      char* ch = static_cast<char*>(buf.data()) + i;
      str.push_back(*ch);
    }
    print(str);
  }

  net::awaitable<void> 
  virtual shutdown()
  {
    std::cout<<"\nShutdown...\n"<<std::endl;

    boost::system::error_code er;
    auto _ = stream_->shutdown(er);
    print(er.what());

    if(er == net::error::eof)
    {
      // Rationale:
      // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
      print("\nconnection was closed with the eof error\n");
      er = {};
    }
    else
    {
        print("\nconnection was closed gracefully\n");
    }
    co_return;
  }

  public:

  [[nodiscard]]
  static json::string 
  encode_base64(const json::string& str)
  {
    json::string dest(str.size()*2,' ');
    boost::beast::detail::base64::encode(dest.data(), str.data(), str.size());
    return dest;
  }


  [[nodiscard]]
  static json::string
  decode_base64(const json::string& str)
  {
    json::string dest(str.size()*2, ' ');
    boost::beast::detail::base64::decode(dest.data(), str.data(), str.size());
    return dest;
  }


  [[nodiscard]]
  static ssl::context
  make_default_context()
  {
    ssl::context ctx{ssl::context::tlsv13_client};
    ctx.set_default_verify_paths();
    ctx.set_verify_mode(ssl::verify_peer);
    return ctx;
  }

  
  [[nodiscard]]
  net::awaitable<http::response<http::string_body>>
  make_simple_get(json::string target, bool is_encode = true)
  {
    auto req = make_header(http::verb::get, host_, target, is_encode);
    auto res = co_await req_res(std::move(req));
    co_return res;
  }


  [[nodiscard]]
  static 
  http::request<http::string_body>
  make_header
  (
    http::verb verb,
    json::string host,
    json::string target,
    bool is_encode = true,
    json::string user_agent = "python-requests/2.32.3"
  )
  {
    http::request<http::string_body> req;
    req.method(verb);

    req.set(http::field::host, host);
    req.set(http::field::user_agent, user_agent);

    if(is_encode)
    {
      target = Pars::MainParser::prepare_url_text(target);
    }

    req.target(target);
    return req;
  }


  static void 
  prepare_multipart
  (
    http::request<http::string_body>& req,
    json::string content_type,
    json::string name,
    json::string filename,
    json::string data,
    Pars::optstr encoding = {},
    json::string boundaries = R"(7b38fa245a9bfa229d3751cf6b3d94fa)"
  )
  {
    static const json::string  crlf{"\r\n"};

    if(encoding)
    {
      req.set(http::field::accept_encoding, encoding.value());
    }

    req.set(http::field::accept, R"(*/*)");
    req.set(http::field::connection, R"(keep-alive)");
    json::string head_content{R"(multipart/form-data; boundary=)"}; head_content += boundaries;

    json::string body{}; 
    json::string b_name{R"(name=)"}; b_name += name; b_name+=";";
    json::string b_filename{R"(filename=)"}; b_filename += filename;

    body += R"(--)"; body+=boundaries;                  body += crlf;
    
    body += R"(Content-Disposition: form-data; )";  
    body += R"(name="upfile"; )"; body += R"(filename="blob")";        body += crlf;

    body += R"(Content-Type: )"; body += content_type;  body += crlf; body += crlf;

    print_response(req);

    body+=std::move(data); body += crlf;

    body += R"(--)"; body += boundaries; body += R"(--)"; body += crlf;

    print("\ncontent_lengh:", body.size(),"\n");
    req.set(http::field::content_length, std::to_string(body.size()));
    req.set(http::field::content_type, head_content);
    
    req.body() = std::move(body);
    Pars::dump_data("search.bin", req);
  }

  protected:

  session_interface
  (
   json::string host,
   json::string port, 
   int version,
   ssl::context ctx,
   net::any_io_executor executor
  )
  noexcept :
  host_(host),
  port_(port),
  version_(version),
  ctx_(std::move(ctx)),
  ex_(executor),
  resolver_(ex_),
  stream_(std::make_shared<stream_type>(ex_, ctx_))
  {}

  public:

  template<typename Derived, typename...Args>
  static 
  net::awaitable<std::shared_ptr<Derived>> 
  make_session
  (
    json::string host,
    json::string port,
    int version,
    bool run = true,
    std::optional<ssl::context> ctx = {},
    Args&&...args
  )
  {
    if(ctx.has_value() == false)
    {
      ctx = make_default_context();
    }

    auto ex = co_await net::this_coro::executor;
    std::shared_ptr<Derived> session = make_shared<Derived>
    (host, port, version, std::move(ctx).value(), ex, std::forward<Args>(args)...);
    
    if(run)
      co_await session->run();

    co_return session;
  }


  net::awaitable<void>
  virtual run()
  {
    // Set SNI Hostname (many hosts need this to handshake successfully)
    if(! SSL_set_tlsext_host_name(stream_->native_handle(), host_.data()))
    {
      throw boost::system::system_error
      (
        static_cast<int>(::ERR_get_error()),
        net::error::get_ssl_category()
      );
    }
    
    co_await resolve();
  }


  net::awaitable<void>
  virtual 
  write_request
  (http::request<http::string_body> req)
  {
    size_t write_data = co_await http::async_write(*stream_, std::move(req));
    print("\nwrite data:", write_data, "\n");
  }
 

  net::awaitable<http::response<http::string_body>> 
  virtual 
  read_response()
  {
    boost::beast::flat_buffer buffer{};
    http::response<http::string_body> resp;
    size_t read_data = co_await http::async_read(*stream_, buffer, resp);
    print("\nread data:", read_data, "\n");
    print_response(resp);
    co_return resp;
  }
  

  net::awaitable<http::response<http::string_body>>
  virtual 
  req_res(http::request<http::string_body> req)
  {
    co_await write_request(std::move(req));
    auto res = co_await read_response();
    co_return res;
  }


  virtual ~session_interface()
  {
    (void) shutdown();
  }

};
