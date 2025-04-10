#include "boost/asio/any_io_executor.hpp"
#include "boost/asio/detached.hpp"
#include "boost/asio/execution_context.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/ssl/context.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/asio/use_awaitable.hpp"
#include "boost/beast.hpp"
#include "boost/iostreams/device/mapped_file.hpp"
#include "boost/asio/co_spawn.hpp"
#include "boost/asio/awaitable.hpp"
#include "boost/beast/http/message.hpp"
#include "boost/beast/http/string_body.hpp"
#include "boost/beast/http/verb.hpp"
#include "json_head.hpp"
#include "print.hpp"
#include <cstring>
#include <fstream>


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>


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
    
    co_await stream_->async_handshake(ssl::stream_base::client);
    co_return;
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

  static void 
  print_endpoint
  (const tcp::endpoint& ep) noexcept
  {
    print("address:", ep.address().to_string(),"\n");
    print("port:", ep.port(),"\n");
    print("protocol:", ep.protocol().protocol(),"\n");
    print("protocol type:", ep.protocol().type(), "\n");
    print("protocol family:", ep.protocol().family(),"\n");
  }


  static void 
  print_result_type
  (const tcp::resolver::results_type& res) noexcept
  {
    print("\nserver endpoints:\n");
    print("number of endpoints:", res.size(),"\n");
    for(auto & i : res)
    {
        print("\nendpoint:\n");
        print("service_name:",i.service_name(),"\n");
        print("host_name:", i.host_name(),"\n");
        print_endpoint(i.endpoint());
    }
  }


  static void 
  print_response(auto&& res)
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
  static 
  http::request<http::string_body>
  make_header
  (
    http::verb verb,
    json::string host,
    json::string target,
    json::string user_agent = "Raven-Fairy"
  )
  {
    http::request<http::string_body> req;
    req.method(verb);
    req.set(http::field::host, host);
    req.set(http::field::user_agent, user_agent);
    req.target(target);
    return req;
  }


  static void 
  prepare_multipart
  (
    http::request<http::string_body>& req,
    json::string content_type,
    json::string data,
    Pars::optstr encoding = {},
    json::string boundaries = "Fairy"
  )
  {
    static const json::string  crlf{"\r\n"};

    json::string head_content{"multipart/form-data; boundary="}; head_content += boundaries;
    req.set(http::field::content_type, head_content);

    if(encoding.has_value())
    {
      req.set(http::field::accept_encoding, encoding.value());
    }

    json::string body{}; 
    body += "--"; body+=boundaries;                 body += crlf;
    
    body+="Content-Disposition: form-data;";        body += crlf;
    body+="Content-Type:"; body += content_type;    body += crlf; body += crlf;

    body+=std::move(data); body += crlf;

    body += "--"; body += boundaries; body += "--"; body += crlf;

    req.body() = std::move(body);
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
  std::shared_ptr<Derived> 
  make_session
  (
    net::any_io_executor ex,
    json::string host,
    json::string port,
    int version,
    std::optional<ssl::context> ctx = {},
    Args&&...args
  )
  {
    if(ctx.has_value() == false)
    {
      ctx = make_default_context();
    }

    std::shared_ptr<Derived> session = make_shared<Derived>(host, port, version, std::move(ctx).value(), ex, std::forward<Args>(args)...);
    return session;
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
  req_res(http::request<http::string_body> req)
  {
    size_t write_data = co_await http::async_write(*stream_, std::move(req), net::use_awaitable);
    print("\nwrite data:", write_data, "\n");

    boost::beast::flat_buffer buffer{};
    http::response<http::string_body> resp;
    size_t read_data = co_await http::async_read(*stream_, buffer, resp, net::use_awaitable);
    print("read data:", read_data,"\n");
    print_response(resp);
  }

  virtual ~session_interface(){}
};



class Searcher : public session_interface
{
  public:

  Searcher
  (
   json::string host,
   json::string port, 
   int version,
   ssl::context ctx,
   net::any_io_executor executor
  )
  noexcept :
  session_interface
  (
    host,
    port,
    version,
    std::move(ctx),
    executor
  )
  {
  }

  net::awaitable<void>
  request_upload_image
  (boost::iostreams::mapped_file map, json::string fileName)
  {
    map = boost::iostreams::mapped_file{fileName.data(), boost::iostreams::mapped_file::readonly};
    json::string data;
    data.reserve(map.size());

    for(auto cb = map.const_data(); cb != cb + map.size();cb++)
    {
      data.append(cb);
    }

    auto req = make_header
      (
        http::verb::post,
        host_,
        "/searchbyimage/upload"
      );

    prepare_multipart
    (
      req,
      "application/jpeg",
      std::move(data),
      "gzip, deflate, br, zstd"
    );

    co_await req_res(std::move(req));
  }

  public:

  static inline json::string host_{"google.com"};
};



int main()
{

  try
  {
    net::io_context ioc;
    auto session = session_interface::make_session<Searcher>(ioc.get_executor(), Searcher::host_, "443",11);
    net::co_spawn(ioc, session->run(), net::detached);
    net::co_spawn(ioc, session->request_upload_image(boost::iostreams::mapped_file{}, "/home/zon/Downloads/Kiki.jpg"), net::detached);
    ioc.run();
  }
  catch(const std::exception& ex)
  {
    print("\n\n", ex.what(), "\n\n");
  }

  return 0;
}
