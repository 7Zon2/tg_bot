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
#include "LFS/LF_stack.hpp"


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
    beast::flat_buffer buffer_;

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
            return current_time.load(std::memory_order_release) - last_time.load(std::memory_order_release);
        }


        [[nodiscard]]
        static bool 
        update()
        {
            using namespace std::chrono;
            current_time.store(duration_cast<seconds>(high_resolution_clock::now().time_since_epoch()), std::memory_order_release);
            auto past_time = last_time.load(std::memory_order_release);
            return last_time.compare_exchange_strong(past_time, current_time, std::memory_order_acq_rel);
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

    protected:

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
            Pars::TG::TelegramResponse resp = co_await
                start_getUpdates<Pars::TG::TelegramResponse,true>(Pars::TG::getUpdates{});
            co_await parse_result(std::move(resp));
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


    void simple_requset
    ( http::request<http::string_body> &req )
    {   
        req.version(version_);
        req.method(http::verb::get);
        req.set(http::field::host, host_);
        req.set(http::field::user_agent, "lalala");
        req.target("/");
    }


    void GetWebhookRequest
    ( http::request<http::string_body>& req)
    {
        req.version(version_);
        req.method(http::verb::get);
        req.set(http::field::host, host_);
        req.target(bot_url);
    }


    [[nodiscard]]
    http::request<http::string_body>
    DeleteWebhookRequest(const bool del)
    {
        json::value val = Pars::TG::deletewebhook::fields_to_value
        (
            del
        );
        json::string data = Pars::MainParser::serialize_to_string(val);
        json::string target = bot_url;
        return PostRequest(std::move(data), std::move(target),"application/json",false);
    }


    [[nodiscard]]
    http::request<http::string_body>
    GetFileRequest(json::string file_id)
    {
        json::string url{"/getFile?file_id="};
        url+=std::move(file_id);
        return prepare_request<false>(std::move(url));
    }


    template<bool isPost>
    [[nodiscard]]
    http::request<http::string_body>
    prepare_request(json::string url, json::string head ="", json::string content_type = "text/html")
    {
        using namespace Pars;
        json::string url_ = std::move(head);
        url_ += bot_url;
        url_  = MainParser::prepare_url_text(std::move(url_));
        url = MainParser::prepare_url_text(std::move(url));
        url_ += std::move(url);
        print("\n\nsend_post_request\n\n",url_,"\n\n");
        if constexpr(isPost)
        {
            return PostRequest(" ", std::move(url_), content_type, false);
        }
        else
        {
            return GetRequest(std::move(url_));
        }
    }


    template<bool isPost, Pars::TG::is_TelegramBased T>
    [[nodiscard]]
    http::request<http::string_body>
    prepare_request(T&& obj, json::string  head = "", json::string content_type = "text/html")
    {
        using namespace Pars;
        json::string url = std::move(head);
        url+=bot_url;
        url = MainParser::prepare_url_text(std::move(url));
        json::string obj_url = std::forward<T>(obj).fields_to_url();
        url += std::move(obj_url);
        url = MainParser::prepare_url_text(std::move(url));
        print("\n\nsend_post_request\n\n",url,"\n\n");
        if constexpr (isPost)
        {
            return PostRequest(" ", std::move(url), content_type, false);
        }
        else
        {
            return GetRequest(std::move(url));
        }
    }

    void
    update_id(const Pars::TG::TelegramResponse& res)
    {
        if (res.update_id.has_value())
        {
            UpdateStorage_.update_stack.push(res.update_id.value() + 1);
        }
    }

    [[nodiscard]]
    http::request<http::string_body>
    SetWebhookRequest()
    {
        json::string certif = CRTF::load_cert(certif_);

        json::string target = bot_url;

        return PostRequest("", target, "multipart/form-data",true);
    }

    public:

    [[nodiscard]]
    boost::asio::awaitable<void>
    send_response
    (http::request<http::string_body> req)
    {
        co_await start_request_response<void, false, true>(std::move(req));
    }


    template<Pars::TG::is_TelegramBased U>
    boost::asio::awaitable<void>
    send_response
    (U&& obj)
    {
        http::request<http::string_body>   req = prepare_request<false>(std::forward<U>(obj));
        co_await send_response(std::move(req));
    }


    template<typename Res = Pars::TG::TelegramResponse>
    [[nodiscard]]
    boost::asio::awaitable<Res>
    read_response()
    {
        using namespace Pars;

        Res res{};
        res = co_await
            start_request_response<Res, true, false>({});
        co_return std::move(res);
    }

    public:

    template<typename Res = Pars::TG::TelegramResponse>
    [[nodiscard]]
    boost::asio::awaitable<Res>
    start_transaction(http::request<http::string_body> req)
    {
        co_await send_response(std::move(req));
        Res res = co_await read_response<Res>();
        co_return std::move(res);
    }


    template<Pars::TG::is_TelegramBased U, typename Res = Pars::TG::TelegramResponse>
    boost::asio::awaitable<Res>
    start_transaction(U&& obj)
    {
        co_await send_response(std::forward<U>(obj));
        Res res = co_await read_response<Res>();
        co_return std::move(res);
    }


    template<bool getUpdates>
    boost::asio::awaitable<Pars::TG::TelegramResponse>
    request_response(http::request<http::string_body> req)
    {
        using namespace Pars;
        using type = TG::TelegramResponse;

        type res{};
        co_await send_response(std::move(req));
        if constexpr (getUpdates)
        {
             res = co_await
             start_getUpdates<type, false, false>
            (Pars::TG::getUpdates{});
        }
        co_return std::move(res);
    }


    template<typename Res, bool getUpdates, Pars::TG::is_TelegramBased U>
    boost::asio::awaitable<Res>
    request_response(U&& obj)
    {
        http::request<http::string_body>   req = prepare_request<false>(std::forward<U>(obj));
        Res res = request_response<Res, getUpdates>(std::move(req));
        co_return std::move(res);
    }

    protected:

    boost::asio::awaitable<void>
    send_command
    (Pars::TG::message mes)
    {
        using namespace Commands;
        co_await CommandInterface<session>::exec(*this, std::move(mes));
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
            co_await send_command(std::move(mes.value()));
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
        update_id(res);

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
                TG::TelegramResponse resp = co_await
                start_getUpdates<TG::TelegramResponse, false>();
                std::jthread th
                {
                    [&]() -> boost::asio::awaitable<void>
                    {
                        co_await parse_result(std::move(resp));
                    }
                };
                th.detach();
            }
        }
        catch(const std::exception& e)
        {
            print(e.what());
            shutdown();
        }
    }

    protected:

    template<typename  Res, bool isRead = true, bool isWrite = true>
    [[nodiscard]]
    boost::asio::awaitable<Res>
    start_request_response(std::optional<http::request<http::string_body>> opt_req = {})
    {
        using namespace Pars;

        static std::mutex mut;

        bool isopen = beast::get_lowest_layer(stream_).socket().is_open();
        if (! isopen)
        {
            throw std::runtime_error{"\nSocket is not open'\n"};
        }

        if constexpr (isWrite)
        {
            if (opt_req.has_value() == false)
            {
                throw std::runtime_error{"\n request is empty\n"};
            }
            size_t writable{};
            auto& req = opt_req.value();
            {
                std::lock_guard<std::mutex> lk{mut};
                writable = co_await http::async_write(stream_ , req, boost::asio::use_awaitable);
            }
            print("\n==============================================================================\n", req,"\n");
            print("\nwritable:", writable,"\n");
            print("\n==============================================================================\n");
        }
        if constexpr(isRead)
        {
            http::response<http::string_body> res{};
            size_t readable{};
            {
                std::lock_guard<std::mutex> lk{mut};
                readable = co_await http::async_read(stream_, buffer_, res, boost::asio::use_awaitable);
            }
            print("\nreadable:", readable,"\n");
            print_response(res);

            if constexpr (TG::is_TelegramBased<Res>)
            {
                json::value var =  json::string{std::move(res).body()};
                var     =  Pars::MainParser::try_parse_value(var);
                Res obj{};
                obj= std::move(var);
                update_id(obj);
                co_return obj;
            }
            else if constexpr (TG::is_HeaderResult<Res>)
            {
                co_return std::move(res);
            }
            else if constexpr(std::is_same_v<Res, void>)
            {
                co_return;
            }
            else
            {
                static_assert(false, "\nCorrect result type wasn't specified\n");
            }
        }
    }

    protected:

    template<bool isForce = false, bool isLast = false>
    [[nodiscard]]
    boost::asio::awaitable<std::optional<http::request<http::string_body>>>
    prepare_getUpdates(Pars::TG::getUpdates upd)
    {
        upd.timeout = Timer::timeout.count();
        if (UpdateStorage_.update_stack.empty() || isLast)
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
                co_return prepare_request<false>(std::move(upd));
            }
        }

        co_return std::optional<http::request<http::string_body>>{};
    }

    template
    <
        typename Res = Pars::TG::TelegramResponse,
        bool isForce = false,
        bool isLast = false
    >
    [[nodiscard]]
    boost::asio::awaitable<Res>
    start_getUpdates(Pars::TG::getUpdates upd = {})
    {
        using namespace Pars::TG;
        auto opt_req = co_await
        prepare_getUpdates<true, isLast>(std::move(upd));

        try
        {
            Res obj;
            if (opt_req.has_value())
            {
                obj = co_await start_request_response<Res>(std::move(opt_req).value());
            }
            else
            {
                obj = co_await start_request_response<Res, true, false>();
            }
            co_return obj;
        }
        catch (const BadRequestException& e)
        {
            print(e.what());
            co_return Res{};
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            throw e;
        }
    }

    protected:

    void
    PrepareMultiPart
    (http::request<http::string_body>& req,  json::string_view data)
    {
        #define MULTI_PART_BOUNDARY "Fairy"
        #define CRLF "\r\n"

        req.set(http::field::content_type,"multipart/form-data; boundary=" MULTI_PART_BOUNDARY);

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
        req.body() = std::move(temp);
    }


    [[nodiscard]]
    http::request<http::string_body>
    GetRequest
    (
        json::string_view target
    )
    {
        http::request<http::string_body> req;
        req.method(http::verb::get);
        req.set(http::field::host, host_);
        req.set(http::field::keep_alive, "timeout=5, max=1000");
        req.set(http::field::user_agent, "Raven-Fairy");
        req.target(target);
        return req;
    }


    [[nodiscard]]
    http::request<http::string_body>
    PostRequest
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
        req.set(http::field::keep_alive, "timeout=5, max=1000");
        if(multipart)
        {
            PrepareMultiPart(req, data);
            print("\nPreparedMultiRequest:\n\n");
            std::cout<<req<<std::endl;
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
        return req;
    }


    void GetRequest
    ( http::request<http::string_body>& req, std::string_view target)
    {
        req.method(http::verb::get);
        req.set(http::field::host, host_);
        req.set(http::field::user_agent, "lalala");
        req.target(target);
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


    void print_response(auto&& res)
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
