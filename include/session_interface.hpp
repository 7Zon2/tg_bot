#pragma once
#include "json_head.hpp"
#include "print.hpp"
#include "head.hpp"

class session_base
{

  public:

  session_base
  (
    int version,
    json::string host,
    json::string port,
    boost::asio::any_io_executor ex
  )
  noexcept :
  version_(version),
  host_(host),
  port_(port),
  ex_(ex),
  resolver_(ex_)
  {}

  virtual ~session_base() = 0;


  net::awaitable<void>
  virtual run () = 0;


  void
  virtual shutdown() = 0;

  public:

  net::awaitable<void> 
  virtual write_request
  (http::request<http::string_body>&& req) = 0;


  net::awaitable<void> 
  virtual write_request
  (http::request<http::string_body>& req) = 0;


  net::awaitable<http::response<http::string_body>> 
  virtual read_response() = 0;


  net::awaitable<http::response<http::string_body>>
  virtual req_res(http::request<http::string_body>& req) = 0;


  net::awaitable<http::response<http::string_body>>
  virtual req_res(http::request<http::string_body>&& req) = 0;


  net::awaitable<http::response<http::string_body>>
  redirect(auto&& req, json::string relative_url, auto...statuses)
  {
    print("\n\nRedirect...\n\n");
    auto temp_req{std::move(req)};
    http::response<http::string_body> res;
    auto status = http::status::temporary_redirect;
    while
    (
      (status == http::status::temporary_redirect)||
      (status == http::status::permanent_redirect)||
      ((status == statuses) && ...)
    )
    {
      res = co_await req_res(temp_req);
      status = res.result();
      auto it = res.find(http::field::location);
      if (it == res.end())
        break;

      
      json::string url = it->value();
      url = make_relative_url(std::move(url), relative_url);
      print("RELATIVE REDIRECT URL:", url,"\n");
      temp_req.target(std::move(url));


      it = res.find(http::field::connection);
      if (it != res.end() && it->value() == "close")
      {
        print("\nreq before reconnection\n", temp_req);
        print("\nConnection was closed. Try to reconnect...\n");
        co_await run();
        print("\nreq after reconnection\n",temp_req);
      }
       
    }

    co_return std::move(res);
  }
 

  net::awaitable<void> 
  static write_request
  (auto& stream, http::request<http::string_body>& req)
  {
    size_t write_data = co_await http::async_write(stream, std::move(req));
    print("\nwrite data:", write_data, "\n");
  }


  net::awaitable<void> 
  static write_request
  (auto& stream, http::request<http::string_body>&& req)
  {
    size_t write_data = co_await http::async_write(stream, std::move(req));
    print("\nwrite data:", write_data, "\n");
  }


  net::awaitable<http::response<http::string_body>> 
  static read_response(auto & stream)
  {
    boost::beast::flat_buffer buffer{};
    http::response<http::string_body> resp;
    size_t read_data = co_await http::async_read(stream, buffer, resp);
    print("\nread data:", read_data, "\n");
    auto temp_body = std::move(resp).body();
    print(resp);
    resp.body() = std::move(temp_body);
    co_return resp;
  }


  net::awaitable<http::response<http::string_body>>
  static req_res
  (auto & stream, http::request<http::string_body>& req)
  {
    co_await write_request(stream, req);
    auto res = co_await read_response(stream);
    co_return res;
  }

  
  net::awaitable<http::response<http::string_body>>
  static req_res
  (auto & stream, http::request<http::string_body>&& req)
  {
    co_await write_request(stream, std::move(req));
    auto res = co_await read_response(stream);
    co_return res;
  }


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
  net::awaitable<http::response<http::string_body>>
  make_simple_get(auto& stream, json::string target, bool is_encode = true)
  {
    auto req = make_header(http::verb::get, host_, target, is_encode);
    auto res = co_await req_res(stream, std::move(req));
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
    json::string user_agent = "RavenFairy"
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
  (const tcp::resolver::results_type&) = 0;


  net::awaitable<void>
  virtual handshake () = 0;

  protected:

  int version_;
  json::string host_;
  json::string port_;
  boost::asio::any_io_executor ex_;
  tcp::resolver resolver_;
};

inline session_base::~session_base(){}



template<bool HTTPS>
class session_interface;


template<>
class session_interface<true> : public session_base
{
  protected: 

  json::string certif_;
  ssl::context ctx_;
  using stream_type = boost::asio::ssl::stream<beast::tcp_stream>;
  using stream_ptr = std::shared_ptr<stream_type>;
  stream_ptr stream_;

  public:

  net::awaitable<void> 
  write_request
  (http::request<http::string_body>&& req) override
  {
    co_await session_base::write_request(*stream_, std::move(req));
  }


  net::awaitable<void> 
  write_request
  (http::request<http::string_body>& req) override
  {
    co_await session_base::write_request(*stream_, req);
  }


  net::awaitable<http::response<http::string_body>> 
  read_response() override
  {
    auto res = co_await session_base::read_response(*stream_);
    co_return std::move(res);
  }


  net::awaitable<http::response<http::string_body>>
  req_res(http::request<http::string_body>& req) override
  {
    auto res = co_await session_base::req_res(*stream_, req);
    co_return std::move(res);
  }


  net::awaitable<http::response<http::string_body>>
  req_res(http::request<http::string_body>&& req) override
  {
    auto res = co_await session_base::req_res(*stream_, std::move(req));
    co_return std::move(res);
  }


  net::awaitable<void>
  virtual connect
  (const tcp::resolver::results_type& res) override
  {
    // Make the connection on the IP address we get from a lookup
    auto ep = co_await beast::get_lowest_layer(*stream_).async_connect(res);
  
    print("Connecting...\n\n", "connected endpoint:\n");
    print_endpoint(ep);
    print("\n\n");

    co_await handshake();
  }


  net::awaitable<void>
  virtual handshake() override
  {
    print("Handshake...\n"); 
    co_await stream_->async_handshake(ssl::stream_base::client);
  }


  void 
  virtual shutdown() override
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
  }

  public:

  [[nodiscard]]
  static ssl::context
  make_default_context()
  {
    ssl::context ctx{ssl::context::tlsv13_client};
    ctx.set_default_verify_paths();
    ctx.set_verify_mode(ssl::verify_peer);
    return ctx;
  }

  public:

  session_interface
  (
   int version,
   json::string host,
   json::string port, 
   net::any_io_executor executor,
   std::optional<ssl::context> ctx = {}
  )
  noexcept :
  session_base
  (
    version,
    host,
    port,
    executor
  ),
  ctx_(ctx ? std::move(ctx).value() : CRTF::load_default_client_ctx()),
  stream_(std::make_shared<stream_type>(ex_, ctx_))
  {}

  ~session_interface()
  {
    session_interface::shutdown();
  }

  public:

  template<typename Derived, typename...Args>
  static 
  net::awaitable<std::shared_ptr<Derived>> 
  make_session
  (
    int version,
    json::string host,
    json::string port,
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
    std::shared_ptr<Derived> session = std::make_shared<Derived>
    (version, host, port, std::move(ctx).value(), ex, std::forward<Args>(args)...);
    
    if(run)
      co_await session->run();

    co_return session;
  }


  net::awaitable<void>
  virtual run() override
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

};



template<>
class session_interface<false> : public session_base
{
  protected:

  using stream_type = boost::beast::tcp_stream;
  using stream_ptr  = std::shared_ptr<stream_type>;
  stream_ptr stream_;

  public:

  session_interface
  (
    int version,
    json::string host,
    json::string port,
    net::any_io_executor ex
  )
  noexcept :
  session_base
  (
    version,
    host,
    port,
    ex
  ),
  stream_(std::make_shared<stream_type>(ex_))
  {}

  ~session_interface()
  {
    session_interface::shutdown();
  }

  public:

  net::awaitable<void> 
  write_request
  (http::request<http::string_body>&& req) override
  {
    co_await session_base::write_request(*stream_, std::move(req));
  }


  net::awaitable<void> 
  write_request
  (http::request<http::string_body>& req) override
  {
    co_await session_base::write_request(*stream_, req);
  }


  net::awaitable<http::response<http::string_body>> 
  read_response() override
  {
    auto res = co_await session_base::read_response(*stream_);
    co_return std::move(res);
  }


  net::awaitable<http::response<http::string_body>>
  req_res(http::request<http::string_body>& req) override
  {
    auto res = co_await session_base::req_res(*stream_, req);
    co_return std::move(res);
  }


  net::awaitable<http::response<http::string_body>>
  req_res(http::request<http::string_body>&& req) override
  {
    auto res = co_await session_base::req_res(*stream_, std::move(req));
    co_return std::move(res);
  }


  template<typename Derived, typename...Args>
  static 
  net::awaitable<std::shared_ptr<Derived>> 
  make_session
  (
    int version,
    json::string host,
    json::string port,
    bool run = true,
    Args&&...args
  )
  {
    auto ex = co_await net::this_coro::executor;
    std::shared_ptr<Derived> session = std::make_shared<Derived>
    (version, host, port, ex, std::forward<Args>(args)...);
    
    if(run)
      co_await session->run();

    co_return session;
  }

  
  net::awaitable<void>
  run() override
  {
    co_await resolve();
  }

  protected:

  net::awaitable<void>
  virtual connect
  (const tcp::resolver::results_type& res) override
  {
    auto ep = co_await stream_->async_connect(res);

    print("Connecting...\n\n", "connected endpoint:\n");
    print_endpoint(ep);
    print("\n\n");
  }

  
  net::awaitable<void>
  virtual handshake() override
  {
    co_return;
  }


  void
  virtual shutdown() override
  {
    std::cout<<"\nShutdown...\n"<<std::endl;
    stream_->close();
  }

};
