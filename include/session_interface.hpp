#pragma once
#include "boost/asio/co_spawn.hpp"
#include "boost/asio/detached.hpp"
#include "boost/beast/core/stream_traits.hpp"
#include "json_head.hpp"
#include "head.hpp"
#include <chrono>

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


  net::awaitable<void>
  virtual shutdown() = 0;

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

  public:

  [[nodiscard]]
  json::string 
  get_host() const noexcept 
  {
    return host_;
  }


  [[nodiscard]]
  json::string
  get_port() const noexcept
  {
    return port_;
  }


  [[nodiscard]]
  int get_version() const noexcept
  {
    return version_;
  }

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
  redirect
  (
    auto&& req, 
    json::string relative_url,
    size_t attempt_limit = 5,
    auto...statuses
  )
  {
    print("\n\nRedirect...\n\n");
    
    auto temp_req{std::move(req)};
    http::response<http::string_body> res;
    auto status = http::status::temporary_redirect;
    while
    (
      (status == http::status::temporary_redirect)||
      (status == http::status::permanent_redirect)||
      ((status == statuses) || ...)
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
        print("\nConnection was closed. Try to reconnect...\n");
        co_await run();
      }
      
      if(--attempt_limit == 0)
      {
        break;
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
    print_response(resp);
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
 

  template<typename...Types> //tuples
  static void 
  prepare_multipart
  (
    http::request<http::string_body>& req,
    Types&&...fields
  )
  {

    static_assert(sizeof...(fields) > 0);

    static const json::string  crlf{"\r\n"};
    static const json::string boundaries = R"(magicboundaries)";
    
    auto fix_quotes = [](std::string_view vw)
    {
      std::string str;
      str.reserve(vw.size());
      
      if (*vw.data() != '\"')
      {
        str.push_back('\"');
      }
      
      str += vw;
      
      if(vw.back() != '\"')
      {
        str.push_back('\"'); 
      }
      
      return str;
    };


    auto add_field = [fix_quotes]<typename D>
    requires 
    (
      std::is_convertible_v<std::decay_t<D>, std::string_view>
    )
    (
      std::string_view name, 
      std::string_view filename, 
      std::string_view content_type, 
      D && data
    )
    {
      std::string str{};
      
      str += R"(--)"; str += boundaries; str += crlf;
      str += R"(Content-Disposition: form-data; )";

      str += R"(name=)";
      str += fix_quotes(name);

      if( ! filename.empty())
      {
        str += "; ";
        str += R"(filename=)";
        str += fix_quotes(filename);
        str += ";";
      }

      if ( ! content_type.empty())
      {
        str += crlf;
        str += R"(Content-Type: )";
        str += content_type;
      }
      
      str += crlf; str += crlf;
      print(str);
      str += std::move(data);
      str += crlf;
      return str;
    };
    

    req.set(http::field::accept, "*/*");
    req.set(http::field::accept_encoding, "gzip, deflate, br");
    req.set(http::field::connection, "keep-alive");

    json::string content_type = R"(multipart/form-data; boundary=)";
    content_type += boundaries;
    req.set(http::field::content_type, std::move(content_type));

    print_response(req);

    json::string body{};

    body += 
    (
      add_field
      (
        Utils::forward_like<Types>(std::get<0>(fields)), 
        Utils::forward_like<Types>(std::get<1>(fields)),
        Utils::forward_like<Types>(std::get<2>(fields)),
        Utils::forward_like<Types>(std::get<3>(fields))
      ) 
      + ... + ""
    );

    body += R"(--)"; body += boundaries; body += R"(--)"; body += crlf;

    Pars::dump_data("dump.txt", body);
    req.set(http::field::content_length, std::to_string(body.size()));
    req.body() = std::move(body);
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


enum class PROTOCOL
{
  HTTP,
  HTTPS
};


template<PROTOCOL T>
class session_interface;


template<>
class session_interface<PROTOCOL::HTTPS> : public session_base
{
  protected: 

  std::optional<size_t> timeout_;
  json::string certif_;
  ssl::context ctx_;
  using stream_type = boost::asio::ssl::stream<beast::tcp_stream>;
  using stream_ptr = std::shared_ptr<stream_type>;
  stream_ptr stream_;

  public:

  void set_timeout
  (size_t timeout) noexcept
  {
    timeout_ = timeout;
  }


  void set_timeout() noexcept
  {
    timeout_.reset();
  }

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

    if(timeout_)
    {
      beast::get_lowest_layer(*stream_).expires_after(std::chrono::milliseconds(timeout_.value())); 
    }
    co_await stream_->async_handshake(ssl::stream_base::client);
  }


  net::awaitable<void> 
  virtual shutdown() override
  {
    std::cout<<"\nShutdown...\n"<<std::endl;

    beast::get_lowest_layer(*stream_).expires_after(std::chrono::milliseconds(200));
    co_await stream_->async_shutdown();
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
    net::co_spawn(stream_->get_executor(), shutdown(), net::detached);
  }

  public:

  net::awaitable<void>
  run() override
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
class session_interface<PROTOCOL::HTTP> : public session_base
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
    co_spawn(stream_->get_executor(), session_interface::shutdown(), net::detached);
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
  }

  
  net::awaitable<void>
  virtual handshake() override
  {
    co_return;
  }


  net::awaitable<void>
  virtual shutdown() override
  {
    print("\nShutdown...\n");
    stream_->close();
    co_return;
  }

};
