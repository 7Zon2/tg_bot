#include "head.hpp"
#include "entities/entities.hpp"
#include "certif.hpp"
#include "tg_pars.hpp"
#include "print.hpp"
#include "coro_future.hpp"


template<typename T>
concept is_getUpdates = std::is_same_v<std::remove_reference_t<T>, Pars::TG::getUpdates>;


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
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(1000));

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
    get_url_bot_target()
    {
        json::string str;
        str.append("/bot");
        str.append(token_);
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
        req_.target(get_url_bot_target());
    }


    void DeleteWebhookRequest(const bool del)
    {
        json::value val = Pars::TG::TelegramRequestes::deletewebhook
        (
            del
        );
        json::string data = Pars::MainParser::serialize_to_string(val);
        json::string target = get_url_bot_target();
        PostRequest(data, target,"application/json",false);
    }



    void GetUpdatesRequest(const Pars::TG::getUpdates& obj)
    {
        json::string url = get_url_bot_target();
        url += obj.fields_to_url();
        PostRequest("", url, "application/json", false);
    }


    void SetWebhookRequest()
    {
        json::string certif = CRTF::load_cert("/home/zon/keys/certif/serv/host.crt");

        json::string target = get_url_bot_target();

        PostRequest("", target, "multipart/form-data",true);
    }

    public:

    template<typename T>
    requires (std::is_base_of_v<Pars::TG::TelegramEntities<T>,T>)
    std::future<T> start_request_response()
    {
        co_await std::async(std::launch::async, [this](){http::write(stream_, req_);});
        co_await std::async(std::launch::async, [this](){res_.clear(), http::read(stream_, buffer_, res_);});
        co_await std::async(std::launch::async, &session::print_response, this);
        json::value var = co_await std::async
        (
            std::launch::async,
            [this]()
            {
                return Pars::MainParser::parse_string_as_value(res_.body());
            }
        );
        var      = co_await std::async(std::launch::async, [&var]{ return Pars::MainParser::try_parse_value(var);});
        auto obj = co_await std::async
        (
            std::launch::async,
            [&var, this]()
            {
                auto obj = T::verify_fields(var,T{});
                if (obj.has_value() == false)
                    throw std::runtime_error{"Failed verify_fields\n"};
                else
                    return std::move(obj.value());
            }
        );
        co_return obj;
    }

    public:

    [[nodiscard]]
    Pars::TG:: getUpdates 
    start_getUpdates(const Pars::TG::getUpdates& upd)
    {
        using namespace Pars::TG;
        try
        {
            GetUpdatesRequest(upd);
            getUpdates obj = start_request_response<getUpdates>().get();
            req_.clear();
            return obj;
        }
        catch(const std::exception& e)
        {
            req_.clear();
            std::cerr << e.what() << '\n';
            throw e;
        }
    }

    [[nodiscard]]
    Pars::TG::deletewebhook 
    start_delete_web_hook(const bool del)
    {
        try
        {
            using namespace Pars::TG;
            DeleteWebhookRequest(del);
            deletewebhook obj = start_request_response<deletewebhook>().get();
            req_.clear();
            return obj;
        }
        catch(const std::exception& e)
        {
            req_.clear();
            std::cerr << e.what() << '\n';
            throw e;
        }
    }


    [[nodiscard]]
    Pars::TG::TelegramResponse 
    start_get_webhook_request()
    { 
        using namespace Pars::TG;
        try
        {
            GetWebhookRequest();
            TelegramResponse obj = start_request_response<TelegramResponse>().get();
            req_.clear();
            if (obj.ok == false)
                throw std::runtime_error{"GetWebhook response failed\n"};
            return obj;
        }
        catch(const std::exception& e)
        {
            req_.clear();
            std::cerr << e.what() << '\n';
            throw e;
        }
    }


    [[nodiscard]]
    Pars::TG::TelegramResponse 
    start_set_webhook_request()
    {
        using namespace Pars::TG;
        try
        {
            SetWebhookRequest();
            TelegramResponse obj = start_request_response<TelegramResponse>().get();
            req_.clear();
            if (obj.ok == false)
                throw std::runtime_error{"SetWebhook response failed\n"};
            return obj;
        }
        catch(const std::exception& e)
        {
            req_.clear();
            std::cerr << e.what() << '\n';
            throw e;
        }
    }

    public:

    [[nodiscard]]
    std::string PrepareMultiPart(std::string_view data)
    {
        #define MULTI_PART_BOUNDARY "Fairy"
        #define CRLF "\r\n"

        req_.set(http::field::content_type,"multipart/form-data; boundary=" MULTI_PART_BOUNDARY);

        std::string temp
        {
            "--" MULTI_PART_BOUNDARY CRLF
            R"(Content-Disposition: form-data; name="file"; filename=somefile)" CRLF
            "Content-Type: application/octet-stream" CRLF CRLF
        };
        temp.append(data);
        temp+=CRLF;
        temp+="--" MULTI_PART_BOUNDARY "--" CRLF;

        #undef MULTI_PART_BOUNDARY
        #undef CRLF
        return temp;
    }

    void PostRequest
    (
        std::string_view data, 
        std::string_view target,
        std::string_view content_type, 
        bool multipart
    )
    {  
        req_.method(http::verb::post);
        req_.set(http::field::host, host_);
        if(multipart)
        {
            std::string file = PrepareMultiPart(data);
            req_.body() = file;
            print("PreparedMultiRequest:\n\n");
            std::cout<<req_<<std::endl;
        }
        else
        {
            req_.set(http::field::content_type, content_type);
            req_.set(http::field::content_length, boost::lexical_cast<std::string>(data.size()));
            req_.set(http::field::body, data);
            req_.body() = data;
        }
        req_.set(http::field::user_agent, "Raven-Fairy");
        req_.target(target);

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
           auto resp = start_getUpdates(Pars::TG::getUpdates{});
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            shutdown();
            return;
        }
        
        read();
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
        print("Response:\n\n====================================================================================\n");
        for(auto&& i : res_)
        {
            print
            (
                "field name: ",  i.name_string(),"\t",
                "field value: ", i.value(),"\n"
            );
        }
        print(res_,"\n=========================================================================================\n");
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
