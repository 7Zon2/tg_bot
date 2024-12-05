#include "head.hpp"
#include "entities/entities.hpp"
#include "certif.hpp"
#include "print.hpp"
#include <boost/asio/use_future.hpp>
#include "coro_future.hpp"
#include <stacktrace>
#include <unordered_set>
#include <stack>
#include <boost/stacktrace/stacktrace.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include "tg_exceptions.hpp"
#include "responses/interface.hpp"

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

    ssl::context ctx_;
    boost::asio::any_io_executor ex_;
    tcp::resolver resolver_;
    boost::asio::ssl::stream<beast::tcp_stream> stream_;

    beast::flat_buffer buffer_; // (Must persist between reads)
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;

    private:

    struct UpdateStorage
    {
        std::unordered_set<size_t> updated_set;
        std::stack<size_t>  update_stack;
    };

    UpdateStorage UpdateStorage_;

    private:

    struct Timer
    {
        Timer() = delete;

        static inline const std::chrono::seconds timeout{25};
        static inline std::atomic<std::chrono::seconds> last_time
        {std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch())};

        static inline std::atomic<std::chrono::seconds> current_time
        {std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now().time_since_epoch())};

        [[nodiscard]]
        static 
        std::chrono::seconds 
        get_dif()
        {
            using namespace std::chrono;
            current_time.store(duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()), std::memory_order_release);
            return current_time.load(std::memory_order_relaxed) - last_time.load(std::memory_order_relaxed);
        }


        [[nodiscard]]
        static bool 
        update()
        {
            using namespace std::chrono;
            current_time.store(duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()), std::memory_order_release);
            auto past_time = last_time.load(std::memory_order_relaxed); 
            return last_time.compare_exchange_strong(past_time, current_time, std::memory_order_relaxed, std::memory_order_relaxed);
        }
    };


    protected:

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
     ,ctx_(std::move(ctx))
     , ex_(ex)
    , resolver_(ex_)
    ,stream_(ex_,ctx_)
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

    static
    boost::asio::awaitable<void>
    create_and_run
    (
        std::string_view host,
        const int version,
        std::string_view port,
        std::string_view token,
        ssl::context& ctx
    )
    {
        auto executor = co_await boost::asio::this_coro::executor;
        session ses_(host, version, port, token, executor, ctx);
        co_await ses_.run();
    }

    public:

    void shutdown()
    {
        std::cout<<"\nShutdown...\n"<<std::endl;

        boost::system::error_code er;
        stream_.shutdown(er);
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


    boost::asio::awaitable<void>
    resolve()
    {
        // Look up the domain name
        auto res = co_await resolver_.async_resolve(host_, port_);
        co_await connect(res);
    }


    boost::asio::awaitable<void>
    connect(const tcp::resolver::results_type& results)
    {
        // Make the connection on the IP address we get from a lookup
        auto ep = co_await beast::get_lowest_layer(stream_).async_connect(results);

        print("Connecting...\n\n", "connected endpoint:\n");
        print_endpoint(ep);
        print("\n\n");

       co_await handshake();
    }

    boost::asio::awaitable<void>
    handshake()
    {
        print("Handshake...\n");
        // Perform the SSL handshake
        co_await stream_.async_handshake(ssl::stream_base::client);

        try
        {
            Pars::TG::TelegramResponse resp = co_await start_getUpdates<true>(Pars::TG::getUpdates{});
            parse_result(std::move(resp));
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            shutdown();
        }

        co_await start_waiting();
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
        PostRequest(" ", std::move(url), content_type, false);
    }


    void SetWebhookRequest()
    {
        json::string certif = CRTF::load_cert(certif_);

        json::string target = bot_url;

        PostRequest("", target, "multipart/form-data",true);
    }

    protected:

    template< Pars::TG::is_TelegramBased U, bool OnlyRead, bool OnlyWrite>
    boost::asio::awaitable<bool>
    send_response
    (U&& obj)
    {
        using namespace Pars;
        prepare_post_request(std::forward<U>(obj));
        TG::TelegramResponse response = co_await start_request_response<TG::TelegramResponse, OnlyRead, OnlyWrite>();
        if(response.update_id.has_value())
        {
            UpdateStorage_.update_stack.push(response.update_id.value() + 1);
        }
        print("Message after sent response:\n");
        MainParser::pretty_print(std::cout, response.fields_to_value());
        co_return response.ok;
    }

    template<Pars::TG::is_message T>
    boost::asio::awaitable<void>
    send_response
    (T&& mes)
    {
        using namespace Pars;
        auto funcMessage = &session::send_response<TG::SendMessage, false, true>;

        auto command = Commands::prepare_command(std::forward<T>(mes));
        co_await command.get_await(funcMessage, *this);
    }


    boost::asio::awaitable<void>
    parse_result
    (Pars::TG::TelegramResponse res)
    {
        using namespace Pars;

        auto find_message = [](json::value && value) -> std::optional<TG::message>
        {
            boost::system::error_code er;
            json::value* val = value.find_pointer("/message", er);
            if(er)
            {
                return {};
            }

            Pars::TG::message msg{};
            msg = std::move(val->as_object());
            return msg;
        };

        auto SendReply = [&]() -> boost::asio::awaitable<void>
        {
            auto mes = find_message(std::move(res.result.value()));
            if(!mes.has_value())
            {
                print("Message reply is empty\n");
            }

            print("Message reply:\n");
            MainParser::pretty_print(std::cout, mes.value().fields_to_value());
            co_await send_response(std::move(mes.value()));
        };

        if (! res.ok)
        {
            co_return;
        }

        if (!res.update_id.has_value())
        {
            co_return;
        }

        auto it =  UpdateStorage_.updated_set.insert(res.update_id.value());
        if (!it.second)
        {
            co_return;
        }
        UpdateStorage_.update_stack.push(res.update_id.value() + 1);


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
                co_return;
            }

            co_await SendReply();
        }
        catch(const std::exception& ex)
        {
            print(ex.what(),"\n"); 
            co_return;
        }
    }

    
    template<Pars::TG::is_TelegramBased Answer = Pars::TG::TelegramResponse>
    [[nodiscard]]
    boost::asio::awaitable<void>
    start_waiting()
    {   
        using namespace Pars;

        try
        {
            while(true)
            {
                TG::TelegramResponse resp = co_await start_getUpdates<true>(TG::getUpdates{});
                co_await parse_result(std::move(resp));
            }
        }
        catch(const std::exception& e)
        {
            print(e.what());
            shutdown();
        }
    }


    template<Pars::TG::is_TelegramBased Res, bool OnlyRead = false, bool OnlyWrite = false>
    [[nodiscard]]
    boost::asio::awaitable<Res>
    start_request_response()
    {
        bool isopen = beast::get_lowest_layer(stream_).socket().is_open();
        if (! isopen)
        {
            throw std::runtime_error{"\nSocket is not open'\n"};
        }

        size_t writable = 0;
        if constexpr (OnlyRead == false)
        {
            writable = co_await http::async_write(stream_ , req_, boost::asio::use_awaitable);
            print("\nwritable:", writable,"\n");
        }
        if constexpr(OnlyWrite == true)
        {
            co_return Res{};
        }
        else
        {
            http::response<http::string_body> res;
            size_t readable = co_await http::async_read(stream_, buffer_, res, boost::asio::use_awaitable);
            buffer_.consume(-1);
            res_ = std::move(res);
            print("\nreadable:", readable,"\n");
            print_response();

            json::value var =  json::string{std::move(res_).body()};
            var     =  Pars::MainParser::try_parse_value(var);
            auto opt_map = Res::verify_fields(std::move(var));
            if (! opt_map.has_value())
            {
                throw std::runtime_error{"Failed verify_fields\n"};
            }
            Res obj{};
            obj.fields_from_map(std::move(opt_map.value()));
            co_return obj;
        }
    }

    public:

    template<bool isForce = false, typename T>
    requires (std::is_same_v<std::remove_reference_t<T>, Pars::TG::getUpdates>)
    [[nodiscard]]
    boost::asio::awaitable<void>
    send_getUpdates(T&& upd)
    {
        upd.timeout = Timer::timeout.count();
        if (UpdateStorage_.update_stack.empty())
        {
            upd.offset = -1;
        } 
        else
        {
            upd.offset = UpdateStorage_.update_stack.top();
        }

        print("dif Time:", Timer::get_dif(),"\n");
        if (Timer::get_dif() > Timer::timeout || isForce == true)
        {
            if(Timer::update())
            {
                req_.clear();
                prepare_post_request(std::forward<T>(upd));
                co_await http::async_write(stream_, req_, boost::asio::use_awaitable);
                req_.clear();
            }
        }
    }


    template<bool isForce = false, typename T>
    requires (std::is_same_v<std::remove_reference_t<T>, Pars::TG::getUpdates>)
    [[nodiscard]]
    boost::asio::awaitable<Pars::TG::TelegramResponse>
    start_getUpdates(T&& upd)
    {
        using namespace Pars::TG;
        co_await send_getUpdates<isForce>(std::forward<T>(upd));

        try
        {    
            TelegramResponse obj = co_await start_request_response<TelegramResponse, true>();
            co_return obj;
        }
        catch (const BadRequestException& e)
        {
            print(e.what());
            co_return TelegramResponse{};
        }
        catch(const std::exception& e)
        {
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
        http::request<http::string_body> req;
        req.method(http::verb::post);
        req.set(http::field::host, host_);
        req_.set(http::field::keep_alive, "timeout=5, max=1000");
        if(multipart)
        {
            json::string file = PrepareMultiPart(data);
            req.body() = file;
            print("PreparedMultiRequest:\n\n");
            std::cout<<req_<<std::endl;
        }
        else
        {
            req.set(http::field::content_type, content_type);
            req.set(http::field::content_length, boost::lexical_cast<std::string>(data.size()));
            req.set(http::field::body, data);
            req.body() = data;
        }
        req.set(http::field::user_agent, "Raven-Fairy");
        req.target(target);
        req.prepare_payload();
        req_ = std::move(req);
    }


    void GetRequest
    (std::string_view target)
    {
        req_.method(http::verb::get);
        req_.set(http::field::host, host_);
        req_.set(http::field::user_agent, "lalala");
        req_.target(target);
    }

    protected:

    // Start the asynchronous operation
    boost::asio::awaitable<void>
    run()
    {
        // Set SNI Hostname (many hosts need this to handshake successfully)
        if(! SSL_set_tlsext_host_name(stream_.native_handle(), host_.data()))
        {
                throw boost::system::system_error
                (
                    static_cast<int>(::ERR_get_error()),
                    net::error::get_ssl_category()
                );
        }

        co_await resolve();
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

        boost::asio::co_spawn(ioc, session::create_and_run(host, version, port, bot_token, ctx), boost::asio::detached);
        ioc.run();
    }
    catch(const std::exception& e)
    {
        std::cerr<<std::stacktrace::current()<<std::endl;
        std::cerr << e.what() << '\n';
    }
  

    return EXIT_SUCCESS;
}
