#include "boost/asio/any_io_executor.hpp"
#include "boost/asio/detached.hpp"
#include "boost/asio/impl/co_spawn.hpp"
#include "boost/asio/io_context.hpp"
#include "boost/asio/ssl/context.hpp"
#include "boost/asio/this_coro.hpp"
#include "boost/beast.hpp"
#include "boost/asio/co_spawn.hpp"
#include "boost/asio/ssl.hpp"
#include "boost/asio/awaitable.hpp"
#include "boost/asio/use_awaitable.hpp"
#include "json_head.hpp"
#include "print.hpp"


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
    // Perform the SSL handshake
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
    for(auto && i : res)
    {
        print("\nendpoint:\n");
        print("service_name:",i.service_name(),"\n");
        print("host_name:", i.host_name(),"\n");
        print_endpoint(i.endpoint());
    }
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

  protected:

  session_interface
  (
   json::string host,
   json::string port, 
   int version,
   ssl::context ctx,
   net::any_io_executor& executor
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
    net::any_io_executor& ex,
    json::string host,
    json::string port,
    int version,
    std::optional<ssl::context> ctx,
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


  virtual ~session_interface(){}
};



class Searcher : public session_interface
{
  Searcher
  (
   json::string host,
   json::string port, 
   int version,
   ssl::context ctx,
   net::any_io_executor& executor
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

    public:

    static inline json::string host_{"google.com"};
};



int main()
{

  try
  {
    net::io_context ioc;
    auto session = session_interface::make_session<Searcher>(ioc, Searcher::host_, "5000",11,{});
    net::co_spawn(ioc, session->run(), net::detached);
  }
  catch(const std::exception& ex)
  {
    print("\n\n", ex.what(), "\n\n");
  }

  return 0;
}
