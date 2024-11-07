#include "head.hpp"
#include "entities/entities.hpp"
#include "certif.hpp"
#include "print.hpp"
#include "coro_future.hpp"
#include <stacktrace>
#include <boost/stacktrace/stacktrace.hpp>

template<typename T>
concept is_getUpdates = std::is_same_v<std::remove_reference_t<T>, Pars::TG::getUpdates>;


// Performs an HTTP GET and prints the response
class session : public std::enable_shared_from_this<session>
{
    const json::string host_;
    const int version_;
    const json::string port_;
    const json::string token_; 
    const json::string certif_;
    json::string bot_url;

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
        bot_url.append("/bot");
        bot_url.append(token_);
        print
        (
            "host: ", host_, "\n"
            "port: ", port_, "\n"
            "version: ",  version_, "\n"
            "token: "   , token_, "\n"
            "bot url: ", bot_url, "\n"
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
        req_.target(bot_url);
    }


    void DeleteWebhookRequest(const bool del)
    {
        json::value val = Pars::TG::deletewebhook::fields_to_value
        (
            del
        );
        json::string data = Pars::MainParser::serialize_to_string(val);
        json::string target = bot_url;
        PostRequest(std::move(data), std::move(target),"application/json",false);
    }


    template<Pars::TG::is_TelegramBased T>
    void prepare_post_request(T&& obj, json::string_view content_type = "text/html")
    {
        json::string url = bot_url;
        url += std::forward<T>(obj).fields_to_url();
        print("\n\nsend_post_request\n\n",url,"\n\n");
        PostRequest("", std::move(url), content_type, false);
    }


    void SetWebhookRequest()
    {
        json::string certif = CRTF::load_cert(certif_);

        json::string target = bot_url;

        PostRequest("", target, "multipart/form-data",true);
    }

    protected:

    template<Pars::TG::is_message T>
    [[nodiscard]]
    bool prepare_response
    (T&& mes)
    {
        static json::string command{"/echo"};

        if (!mes.text.has_value())
            return false;

        json::string txt = Utils::forward_like<T>(mes.text.value());
        size_t pos = txt.find_first_of(command);
        if (pos == txt.npos)
            return false;

        const size_t chat_id = mes.chat.id;

        try
        {
            txt = json::string{std::make_move_iterator(txt.begin()) + command.size(), std::make_move_iterator(txt.end())};
            if (txt.empty())
            {
                txt = "There is nothing. Where everything is gone?";    
            }
            Pars::TG::SendMessage send{chat_id, std::move(txt)};
            prepare_post_request(std::move(send));
            return true;
        }
        catch(const std::exception& ex)
        {
            print(ex.what());
            return false;
        }
    }


    void parse_result
    (Pars::TG::TelegramResponse response)
    {
        using namespace Pars;

        auto find_message = [](auto& b, auto& e) -> std::optional<TG::message>
        {
            for(; b < e; ++b)
            {
                json::value val = b->at_pointer("/message"); 
                if(val != nullptr)
                {
                    Pars::TG::message msg{};
                    msg = std::move(b->as_object());
                    return msg;
                }
            }
            return {};
        };

        if (! response.ok)
        {
            return;
        }

        TG::TelegramResponse res = std::move(response);

        try
        {
            if (res.description.has_value())
            {
                json::string_view vw = res.description.value();
                print("description:\n", vw);
            }

            if (! res.result.has_value())
            {
                print("result array is empty\n");
                return;
            }

            json::array& arr = res.result.value();
            auto b = arr.begin();
            auto e = arr.end();

            auto mes = find_message(b, e);
            if(mes.has_value())
            {
                print("Message reply:\n");
                MainParser::pretty_print(std::cout, mes.value().fields_to_value());
                bool is_sent = prepare_response(std::move(mes.value()));
                if(is_sent)
                {
                    TG::TelegramResponse obj = start_request_response<TG::TelegramResponse>().get();
                    req_.clear();
                    print("Message after sent response:\n");
                    MainParser::pretty_print(std::cout, obj.fields_to_value());
                }
            }
            else
            {
                print("Message reply is empty\n");
            }
        }
        catch(const std::exception& ex)
        {
            print(ex.what(),"\n"); 
            return;
        }
    }

    
    template<Pars::TG::is_TelegramBased Answer = Pars::TG::TelegramResponse>
    void start_waiting()
    {   
        using namespace Pars;

        try
        {
            boost::asio::post(resolver_.get_executor(), [self = shared_from_this()]
            {
                TG::TelegramResponse resp = start_getUpdates();
                if (!resp.ok)
                {
                    return;
                }

                http::read(self->stream_, self->buffer_, self->res_);
                json::value val = Pars::MainParser::parse_string_as_value(std::move(self->res_).body());
                val = Pars::MainParser::try_parse_value(std::move(val));
                auto opt_map = Answer::verify_fields(std::move(val));
                if (opt_map.has_value())
                {
                    Answer obj{};
                    obj.fields_from_map(std::move(opt_map.value()));
                    self->parse_result(std::move(obj));
                } 
                self->start_waiting();
            });
        }
        catch(const std::exception& e)
        {
            print(e.what());
            shutdown();
        }
    }


    template<Pars::TG::is_TelegramBased Res, bool Only_read = false>
    std::future<Res> start_request_response()
    {
        if constexpr (Only_read == false)
        {
            co_await std::async(std::launch::async, [this](){http::write(stream_, req_); req_.clear();});
        }
        co_await std::async(std::launch::async, [this](){res_.clear(), http::read(stream_, buffer_, res_);});
        co_await std::async(std::launch::async, &session::print_response, this);
        json::value var = co_await std::async
        (
            std::launch::async,
            [this]()
            {
                return Pars::MainParser::parse_string_as_value(std::move(res_).body());
            }
        );
        var      = co_await std::async(std::launch::async, [&var]{ return Pars::MainParser::try_parse_value(var);});
        auto obj = co_await std::async
        (
            std::launch::async,
            [&var]()
            {
                auto opt_map = Res::verify_fields(std::move(var));
                if (! opt_map.has_value())
                    throw std::runtime_error{"Failed verify_fields\n"};
                else
                {
                    Res obj{};
                    obj.fields_from_map(std::move(opt_map.value()));
                    return obj;
                }
            }
        );
        co_return obj;
    }

    public:

    template<typename T>
    requires (std::is_same_v<std::remove_reference_t<T>, Pars::TG::getUpdates>)
    [[nodiscard]]
    bool
    send_getUpdates(T&& upd = Pars::TG::getUpdates{})
    {
        using namespace std::chrono;
        static const seconds timeout_ = 25;
        static std::atomic<seconds> last_time = duration_cast<seconds>(high_resolution_clock::now());

        upd.timeout = timeout_;
        seconds current_time = duration_cast<seconds>(high_resolution_clock::now());
        seconds dif = current_time - last_time;
        
        if (dif > timeout_)
        {
            print("dif Time:", dif);
            if(! last_time.compare_exchange_weak(last_time, current_time))
                return false;
        }

        prepare_post_request(std::forward<T>(upd));
        http::write(stream_, req_); 
        req_.clear();
        return true;
    }


    template<typename T>
    requires (std::is_same_v<std::remove_reference_t<T>, Pars::TG::getUpdates>)
    [[nodiscard]]
    Pars::TG::TelegramResponse 
    start_getUpdates(T&& upd = Pars::TG::getUpdates{})
    {
        using namespace Pars::TG;
        bool is = send_getUpdates(std::forward<T>(upd));
        if(!is)
        {
            return {};
        }

        try
        {    
            TelegramResponse obj = start_request_response<TelegramResponse, true>().get();
            return obj;
        }
        catch(const std::exception& e)
        {
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
    json::string 
    PrepareMultiPart
    (json::string_view data)
    {
        #define MULTI_PART_BOUNDARY "Fairy"
        #define CRLF "\r\n"

        req_.set(http::field::content_type,"multipart/form-data; boundary=" MULTI_PART_BOUNDARY);

        json::string temp
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
        json::string_view data, 
        json::string_view target,
        json::string_view content_type, 
        bool multipart
    )
    {  
        req_.clear();
        req_.method(http::verb::post);
        req_.set(http::field::host, host_);
        if(multipart)
        {
            json::string file = PrepareMultiPart(data);
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
    on_connect
    (
        beast::error_code ec,
        tcp::resolver::results_type::endpoint_type ep
    )
    {
        if(ec)
            return fail(ec, "connect");

        print("Connecting...\n\n", "connected endpoint:\n");
        print_endpoint(ep);
        print("\n\n");
        
        handshake();
    }


    void
    on_handshake
    (beast::error_code ec)
    {
        if(ec)
            return fail(ec, "handshake");

        print("Handshake...\n");
        
        try
        {
           auto resp = start_getUpdates(Pars::TG::getUpdates{});
           parse_result(std::move(resp));
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            shutdown();
            return;
        }
        
        start_waiting();
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
    on_shutdown
    (beast::error_code ec)
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

    std::set_terminate([]()
    {
        try
        {
            std::cerr<<std::stacktrace::current()<<std::endl;
        }
        catch(...)
        {}
        std::abort();
    });

    if(argc != 2)
    {
      print
      (
        "Please, input your bot-telegram token\n"
      );
      return 1;
    }

    std::string bot_token = argv[1];

    std::string host   = "api.telegram.org";
    std::string port   = "443";
    int version = 11;

    try
    {         
        net::io_context ioc;

        ssl::context ctx{ssl::context::tlsv13_client};
        
        ctx.set_default_verify_paths();

        ctx.set_verify_mode(ssl::verify_peer);


        std::make_shared<session>
        (
            host,
            version,
            port,
            bot_token,
            net::make_strand(ioc),
            ctx

        )->run();

        ioc.run();
    }
    catch(const std::exception& e)
    {
        std::cerr<<std::stacktrace::current()<<std::endl;
        //std::cerr<<boost::stacktrace::from_current_exception()<<std::endl;
        std::cerr << e.what() << '\n';
    }
  

    return EXIT_SUCCESS;
}
