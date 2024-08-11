#include "head.hpp"
#include "entities/entities.hpp"
#include "certif.hpp"
#include "tg_pars.hpp"
#include "print.hpp"
#include "coro_future.hpp"

// Performs an HTTP GET and prints the response
class session : public std::enable_shared_from_this<session>
{
    const json::string host_;
    const int version_;
    const json::string port_;
    const json::string token_; 

    json::string target_;
    tcp::resolver resolver_;

    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_; // (Must persist between reads)
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;


    public:

    explicit
    session
    (
        std::string_view host,
        const int version,
        std::string_view port,
        std::string_view token,
        net::any_io_executor ex,
        ssl::context& ctx
    )
    : 
      host_(host)
    , version_(version)
    , port_(port)
    , token_(token)
    , resolver_(ex) 
    , stream_(ex, ctx)
    {
        print
        (
            "host: ", host_, "\n"
            "port: ", port_, "\n"
            "version: ",  version_, "\n"
            "token: "   , token_, "\n"
        );
    }


    virtual ~session(){}

    public:

    void shutdown()
    {
        stream_.async_shutdown
        (
            beast::bind_front_handler
            (
                &session::on_shutdown,
                shared_from_this()
            )
        );
    }


    void resolve()
    {
        // Look up the domain name
        resolver_.async_resolve
        (
            host_.data(),
            port_.data(),
            beast::bind_front_handler
            (
                &session::on_resolve,
                shared_from_this()
            )
        );
    }


    void connect(const tcp::resolver::results_type& results)
    {
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(stream_).async_connect
        (
            results,
            beast::bind_front_handler
            (
                &session::on_connect,
                shared_from_this()
            )
        );
    }


    void handshake()
    {
        // Perform the SSL handshake
        stream_.async_handshake
        (
            ssl::stream_base::client,
            beast::bind_front_handler
            (
                &session::on_handshake,
                shared_from_this()
            )
        );
    }


    void write()
    {
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        http::async_write
        (
            stream_, 
            req_,
            beast::bind_front_handler
            (
                &session::on_write,
                shared_from_this()
            )
        );
    }


    void read()
    {
        http::async_read
        (
            stream_, 
            buffer_, 
            res_,
            beast::bind_front_handler
            (
                &session::on_read,
                shared_from_this()
            )
        );
    }

    public:

    json::string
    get_url_bot_target(std::string_view request)
    {
        json::string str;
        str.append("/bot");
        str.append(token_);
        str.append(request);
        return str;
    }


    void set_target
    (std::string_view target)
    {
        target_ = target;
    }


    protected:

    void simple_requset()
    {
        req_.version(version_);
        req_.method(http::verb::get);
        req_.set(http::field::host, host_);
        req_.set(http::field::user_agent, "lalala");
        req_.target("/");
    }

    void GetWebhookRequest()
    {
        req_.version(version_);
        req_.method(http::verb::get);
        req_.set(http::field::host, host_);
        req_.target(get_url_bot_target("getwebhookinfo"));
    }


    void SetWebhookRequest()
    {
        json::string certif = CRTF::load_cert("/home/zon/keys/certif/serv/host.crt");

        json::string url = get_url_bot_target("setwebhook");

        json::array arr{"Message", "Chat", "User", "ChatFullInfo"};

        json::value val = Pars::TG::TelegramRequestes::setWebhook
        (
            "",
            certif,
            std::nullopt,
            std::nullopt,
            arr,
            false
        );

        json::string data = Pars::MainParser::serialize_to_string(val);
        PostRequest(data, url, "multipart/form-data");
    }

    public:

    void start_get_webhook_request()
    { 
       auto play = [this]() -> std::future<void>
        {
            co_await std::async(&session::GetWebhookRequest, this);
            co_await std::async([this](){http::write(stream_, req_);});
            co_await std::async([this](){res_.clear(), http::read(stream_, buffer_, res_);});
            co_await std::async(&session::print_response, this);
            auto var = co_await std::async
            (
                [this]()
                {
                    return Pars::MainParser::parse_string_as_value(res_.body());
                }
            );
            auto map = co_await std::async
            (
                [&var, this]()
                {
                    auto map = Pars::TG::TelegramResponse::verify_fields(var);
                    if (map.has_value() == false)
                        throw std::runtime_error{"Failed verify_field in GetWebhookRequest\n"};
                    else
                        return std::move(map.value());
                }
            );

            // if(! map["ok"].as_bool() || !map["error_code"].as_int64())
            //     throw std::runtime_error{"Failed ResponseTelegram\n"};

            //print("description TelegramResponse:\n", map["description"].as_string());
            print("map:\n");
            for(auto &&i : map)
                print(i.first);
            shutdown();
        };

        try
        {
            auto fut = play();
            fut.get();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            throw e;
        }
        
    }

    public:

    void PostRequest
    (
        std::string_view data, 
        std::string_view target,
        std::string_view content_type 
    )
    {
        req_.method(http::verb::post);
        req_.set(http::field::host, host_);
        req_.set(http::field::content_type, content_type);
        req_.set(http::field::user_agent, "Raven-Fairy");
        req_.target(target);
        req_.set(http::field::content_length, boost::lexical_cast<std::string>(data.size()));
        req_.set(http::field::body, data);
        req_.body() = data;
        req_.prepare_payload();
    }


    void GetRequest
    (std::string_view target)
    {
        req_.method(http::verb::get);
        req_.set(http::field::host, host_);
        req_.set(http::field::user_agent, "lalala");
        req_.target(target);
    }

    public:

    // Start the asynchronous operation
    void
    run()
    {
        // Set SNI Hostname (many hosts need this to handshake successfully)
        if(! SSL_set_tlsext_host_name(stream_.native_handle(), host_.data()))
        {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            std::cerr << ec.message() << "\n";
            return;
        }

        resolve();
    }

    protected:

    void
    on_resolve
    (
        beast::error_code ec,
        tcp::resolver::results_type results
    )
    {
        if(ec)
            return fail(ec, "resolve");


        print_result_type(results);
        print("resolving...\n");

        connect(results);
    }


    void
    on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
    {
        if(ec)
            return fail(ec, "connect");

        print("Connecting...\n\n", "connected endpoint:\n");
        print_endpoint(ep);
        print("\n\n");

        handshake();
    }


    void
    on_handshake(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "handshake");

        print("Handshake...\n");

        try
        {
           start_get_webhook_request();
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            shutdown();
        }
        

        write();
    }


    void
    on_write
    (
        beast::error_code ec,
        std::size_t bytes_transferred
    )
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        print("writing...\n");


        read();
    }


    void
    on_read
    (
        beast::error_code ec,
        std::size_t bytes_transferred
    )
    {
        print("\nreading...\n","bytes_transferred:", bytes_transferred,"\n");

        if(ec)
            return fail(ec, "read");

        print_response();

        write();
    }


    void
    on_shutdown(beast::error_code ec)
    {
        print("Shutdown...\n",ec.what());

        if(ec == net::error::eof)
        {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec = {};
        }
        if(ec)
            return fail(ec, "shutdown");

        // If we get here then the connection is closed gracefully
    }

    public:

    void print_response()
    {
        for(auto&& i : res_)
            print(i.name_string());
        print(res_,"\n");
    }
};

//------------------------------------------------------------------------------

int main(int argc, char** argv)
{
  
    // if(argc <= 1)
    // {
    //     std::cout<<"Please, input your bot-telegram token\n";
    //     return 1;
    // }

    std::string host   = "api.telegram.org";
    std::string port   = "443";
    int version = 11;

    try
    {
         
        // The io_context is required for all I/O
        net::io_context ioc;

        // The SSL context is required, and holds certificates
        ssl::context ctx{ssl::context::tlsv13_client};
        
        // This holds the root certificate used for verification
        CRTF::load_cert(ctx,"/home/zon/keys/certif/serv/host.crt");

        ctx.set_default_verify_paths();

        // Verify the remote server's certificate
        ctx.set_verify_mode(ssl::verify_peer);

        // Launch the asynchronous operation
        // The session is constructed with a strand to
        // ensure that handlers do not execute concurrently.
        std::make_shared<session>
        (
            host,
            version,
            port,
            "7462084054:AAFMMUI_V8jEdzWSu-OPmuD-nKaLqdrzFOU",
            net::make_strand(ioc),
            ctx

        )->run();

        // Run the I/O service. The call will return when
        // the get operation is complete.
        ioc.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
  

    return EXIT_SUCCESS;
}
