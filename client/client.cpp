#include "head.hpp"
#include "entities/entities.hpp"
#include "certif.hpp"
#include "print.hpp"
#include "coro_future.hpp"
#include <stacktrace>
#include <unordered_set>
#include <stack>
#include <boost/stacktrace/stacktrace.hpp>
#include "tg_exceptions.hpp"

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

    struct UpdateStorage
    {
        std::unordered_set<size_t> updated_set;
        std::stack<size_t>  update_stack;
    };

    UpdateStorage UpdateStorage_;

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
        PostRequest(" ", std::move(url), content_type, false);
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

        using namespace Pars;

        auto NothingMessage = [&]
        {
            static json::string txt{"There is nothing. Where everything is gone?"}; 
            prepare_post_request(TG::SendMessage(mes.chat.id, txt));
        };


        static json::string command{"/echo"};
        
        if (!mes.text.has_value())
        {
            NothingMessage();
            return false;
        }
        if (mes.text.value().size() <= command.size())
        {
            NothingMessage();
            return false;
        }


        const size_t chat_id = mes.chat.id;
        json::string& ref = mes.text.value();
        json::string substr{ref.begin(), ref.begin()+command.size()};
        if(substr != command)
        {
            NothingMessage();
            return false;
        }

        try
        {
            size_t max_offset = 0;
            size_t offset = command.size();
            const size_t limit = 2048;
            json::string txt = Utils::forward_like<T>(ref);

            while(offset < txt.size())
            {
                if (txt.size() - offset > limit)
                {
                    max_offset = limit;  
                }
                else
                {
                    max_offset =  txt.size() - offset;
                }   

                substr = json::string
                {std::make_move_iterator(txt.begin()) + offset,
                std::make_move_iterator(txt.begin()) + offset + max_offset};

                TG::SendMessage send{chat_id, std::move(substr)};
                prepare_post_request(std::move(send));
                offset = offset + limit;
            }

            return true;
        }
        catch(const std::exception& ex)
        {
            print(ex.what());
            return false;
        }
    }


    void parse_result
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

        auto SendReply = [&]()
        {
            auto mes = find_message(std::move(res.result.value()));
            if(!mes.has_value())
            {
                print("Message reply is empty\n");
            }

            print("Message reply:\n");
            MainParser::pretty_print(std::cout, mes.value().fields_to_value());
            bool is_sent = prepare_response(std::move(mes.value()));
            if(true)
            {
                TG::TelegramResponse obj = start_request_response<TG::TelegramResponse>().get();
                if(obj.update_id.has_value())
                {
                    UpdateStorage_.update_stack.push(obj.update_id.value() + 1);
                }
                print("Message after sent response:\n");
                MainParser::pretty_print(std::cout, obj.fields_to_value());
            }
        };

        if (! res.ok)
        {
            return;
        }

        if (!res.update_id.has_value())
        {
            return;
        }

        auto it =  UpdateStorage_.updated_set.insert(res.update_id.value());
        if (!it.second)
        {
            return;
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
                return;
            }

            SendReply();
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
            while(true)
            {
                TG::TelegramResponse resp = start_getUpdates<true>(TG::getUpdates{});
                parse_result(std::move(resp));
            }
        }
        catch(const std::exception& e)
        {
            print(e.what());
            shutdown();
        }
    }


    template<Pars::TG::is_TelegramBased Res, bool OnlyRead = false>
    std::future<Res> start_request_response()
    {
        if constexpr (OnlyRead == false)
        {
            co_await std::async(std::launch::async, [this]()
            {
                http::write(stream_, req_); 
                req_.clear();
            });
        }
        co_await std::async(std::launch::async, [this]()
        {
            http::response_parser<http::string_body> parser_;
            auto& res = parser_.get();

            boost::system::error_code er;
            http::read_header(stream_, buffer_, parser_, er);

            if(er && er!=http::error::need_buffer)
            {
                print("\n\n",er.what(),"\n\n");
                buffer_.clear();
                throw boost::system::system_error(er);
            }
            assert(parser_.is_header_done());
            print("\nResponse Header:\n\n", res.base(),"\n");

            while(!parser_.is_done())
            {
                http::read(stream_, buffer_, parser_, er);
                if(er && er != http::error::need_buffer)
                {
                    buffer_.clear();
                    throw boost::system::system_error(er);
                }
            }

            res_ = std::move(res);
            print("\nBody lenght: ", res_.body().size(),"\n");
            print("\nResponse Body:\n\n", res_.body());

            if(res_.result() != http::status::ok)
            {
                buffer_.clear();
                throw BadRequestException{};
            }
        });
        co_await std::async(std::launch::async, &session::print_response, this);
        json::value var = co_await std::async
        (
            std::launch::async,
            [this]()
            {
                json::string str{std::move(res_).body()};
                return str;
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

    template<bool isForce = false, typename T>
    requires (std::is_same_v<std::remove_reference_t<T>, Pars::TG::getUpdates>)
    void
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
                http::write(stream_, req_); 
                req_.clear();
            }
        }
    }


    template<bool isForce = false, typename T>
    requires (std::is_same_v<std::remove_reference_t<T>, Pars::TG::getUpdates>)
    [[nodiscard]]
    Pars::TG::TelegramResponse 
    start_getUpdates(T&& upd)
    {
        using namespace Pars::TG;
        send_getUpdates<isForce>(std::forward<T>(upd));

        try
        {    
            TelegramResponse obj = start_request_response<TelegramResponse, true>().get();
            return obj;
        }
        catch (const BadRequestException& e)
        {
            print(e.what());
            return {};
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
           Pars::TG::TelegramResponse resp = start_getUpdates<true>(Pars::TG::getUpdates{});
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
        std::cerr << e.what() << '\n';
    }
  

    return EXIT_SUCCESS;
}
