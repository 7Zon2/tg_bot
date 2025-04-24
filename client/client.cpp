#include "boost/asio/awaitable.hpp"
#include "boost/asio/ssl/context.hpp"
#include "boost/beast/http/message.hpp"
#include "boost/beast/http/verb.hpp"
#include "entities/Document.hpp"
#include "entities/TelegramResponse.hpp"
#include "entities/concept_entities.hpp"
#include "head.hpp"
#include "entities/entities.hpp"
#include <stacktrace>
#include <boost/stacktrace/stacktrace.hpp>
#include "json_head.hpp"
#include "tg_exceptions.hpp"
#include "responses/interface.hpp"
#include "LFS/LF_stack.hpp"
#include "LFS/LF_set.hpp"
#include "session_interface.hpp"


template<typename T>
concept is_getUpdates = std::is_same_v<std::remove_reference_t<T>, Pars::TG::getUpdates>;


class tg_session : public session_interface<PROTOCOL::HTTPS>
{
  protected:

    const json::string token_; 
    json::string bot_url;
    json::string target_;

    using session_t = session_interface<PROTOCOL::HTTPS>; 

  protected:

    struct UpdateStorage
    {
        LF_set<size_t> updated_set;
        LF_stack<size_t>  update_stack;
    };

    UpdateStorage UpdateStorage_;

  private:

    struct Timer
    {
        Timer() = delete;

        static inline const std::chrono::seconds timeout{15};
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


  public:

    explicit
    tg_session
    (
      int version,
      json::string_view host,
      json::string_view port,
      net::any_io_executor ex,
      ssl::context ctx,
      json::string_view token
    )
    : 
      session_interface
      (
        version,
        host,
        port,
        ex,
        std::move(ctx)
      )
    {
        bot_url.append("/bot");
        bot_url.append(token);
        print
        (
            "host: ", host_, "\n"
            "port: ", port_, "\n"
            "version: ",  version_, "\n"
            "token: "   , token_, "\n"
            "bot url: ", bot_url, "\n"
        );
    }


    ~tg_session(){}

    protected:

    boost::asio::awaitable<void>
    handshake() override
    {
      co_await session_t::handshake();

      try
      {
        Pars::TG::TelegramResponse resp = co_await
        start_getUpdates<Pars::TG::TelegramResponse,true>(Pars::TG::getUpdates{});
        co_await parse_result(std::move(resp));
      }
      catch(const std::exception& e)
      {
          std::cerr << e.what() << '\n';
          std::terminate();
      }

      co_await start_waiting();
    }

    public:

    void set_target
    (std::string_view target)
    {
        target_ = target;
    }


    template<Pars::TG::is_UrlConvertible T>
    [[nodiscard]]
    json::string prepare_url(T&& obj)
    {
      print("\nprepare url\n");
      json::string url = bot_url;
      url += std::forward<T>(obj).fields_to_url();
      print(url,"\n");
      return url;
    }


    template<Pars::TG::is_TelegramBased T>
    [[nodiscard]]
    http::request<http::string_body>
    prepare_request(T&& obj, http::verb verb = http::verb::get)
    {
      print("\nprepare request\n");
      json::string url = prepare_url(std::forward<T>(obj));
      return make_header(verb, host_, std::move(url));
    }


    void
    update_id(const Pars::TG::TelegramResponse& res)
    {
      if (res.update_id.has_value())
      {
        size_t new_id = res.update_id.value() + 1;
        print("\nnew update id: ", new_id,"\n\n");
        UpdateStorage_.update_stack.push(new_id);
      }
    }


    public:


    template<Pars::TG::is_TelegramBased U>
    net::awaitable<void> 
    write_request
    (U&& obj)
    {
        http::request<http::string_body> req = prepare_request(std::forward<U>(obj));
        co_await session_t::write_request(std::move(req));
    }


    template<Pars::TG::is_TelegramBased Res = Pars::TG::TelegramResponse>
    boost::asio::awaitable<Res>
    read_response()
    {
        using namespace Pars;
        auto res =  co_await session_t::read_response();
        json::value var = json::string{std::move(res).body()};
        var = MainParser::try_parse_value(std::move(var));
        Res obj{};
        obj = std::move(var);
        co_return std::move(obj);
    }

    public:

    template<Pars::TG::is_TelegramBased Res = Pars::TG::TelegramResponse>
    boost::asio::awaitable<Res>
    req_res(http::request<http::string_body> req)
    {
      co_await session_t::write_request(std::move(req));
      Res res = co_await read_response<Res>();
      co_return std::move(res);
    }


    template<bool getUpdates>
    boost::asio::awaitable<Pars::TG::TelegramResponse>
    req_res(http::request<http::string_body> req)
    {
        using namespace Pars;

        TG::TelegramResponse res{};
        co_await session_t::write_request(std::move(req));
        if constexpr (getUpdates)
        {
          res = co_await
          start_getUpdates<TG::TelegramResponse, false, false>
          (Pars::TG::getUpdates{});
        }
        co_return std::move(res);
    }

    protected:

    net::awaitable<void>
    send_photo(Pars::TG::SendPhoto mes)
    {
      using namespace Pars;
      print("\n\nsend_photo\n\n");
      
      if ( ! mes.photo)
      {
        print("photo not found\n");
        co_return;
      }

      auto& vec = mes.photo.value();
      if(vec.empty())
      {
        print("vector of photo sizes is empty\n");
        co_return;
      }

      TG::PhotoSize phz = std::move(vec[0]);
      json::string url = bot_url;
      url += phz.getFile_url();

      http::request<http::string_body> req = make_header(http::verb::get, host_, url);
      TG::TelegramResponse res = co_await req_res<>(std::move(req));
      
      print("\n******PHOTO_SIZE resp******", res,"\n");
      if(!res.ok || !res.result)
      {
        print("response wasn't succcess\n");
        co_return;
      }
      
      phz = std::move(res.result).value();
      if(!phz.file_path)
      {
        print("\nFile path wasn't found\n");
        co_return;
      }

      json::string_view file_path = phz.file_path.value();
      url = "/file";
      url += bot_url;
      url += "/";
      url += file_path;
      req = make_header(http::verb::get, host_, std::move(url));
      auto res_b = co_await session_t::req_res(std::move(req));
    }


    net::awaitable<void>
    send_echo(Pars::TG::SendMessage mes)
    {
      using namespace Pars;

      auto callback = [this, chat_id = mes.chat_id](json::string mes) -> net::awaitable<void>
      {
        TG::SendMessage send_mes{chat_id, std::move(mes)};
        auto req = prepare_request(std::move(send_mes));
        co_await session_t::write_request(std::move(req));
      };

      
      json::string data{};
      if(mes.document)
      {
        TG::Document& doc = mes.document.value();
        json::string url = bot_url;
        url+=TG::File::getFile_url(doc.file_id);
         
        http::request<http::string_body> req = make_header(http::verb::get, host_, url);
        co_await session_t::write_request(std::move(req));

        TG::TelegramResponse res;
        TG::File file;
        for(;;)
        {
          res = co_await read_response<>();
          if(res.ok && res.result)
          {
            file = std::move(res.result).value();
            if(file.file_path)
            {
              break;
            }
          }
        }

        if (!file.file_path)
        {
          throw std::runtime_error{"\nfile path was not found\n"};
        }

        json::string_view file_path = file.file_path.value();
        url = "/file";
        url += bot_url;
        url += "/";
        url += file_path;
        req = make_header(http::verb::get, host_, url);
        auto res_b = co_await session_t::req_res(std::move(req));
        data = std::move(res_b).body();
      }
      else
      {
        data = std::move(mes.text).value();
      }

      co_await Commands::Echo{}(callback, std::move(data));
    }

    
    net::awaitable<void>
    send_search(Pars::TG::SendMessage mes)
    {
      using namespace Pars;
      co_return;
    }


    boost::asio::awaitable<void>
    send_command
    (Pars::TG::message mes)
    {
      json::string_view text;
      if(mes.text)
      {
        text = mes.text.value();
      }

      Commands::CommandType type = Commands::CommandType::None;
      if(mes.caption)
      {
        type = Commands::get_command(mes.caption.value());
      }
      else
      {
        type = Commands::get_command(text);
      }

      switch(type)
      {
        case Commands::CommandType::Echo : 
        {
          co_await send_echo(std::move(mes));
          break;
        }

        case Commands::CommandType::Search :
        {
          co_await send_search(std::move(mes));
          break;
        }

        case Commands::CommandType::None :
        {
          auto callback = [&](json::string data) -> net::awaitable<void>
          {
            Pars::TG::SendMessage mes_{std::move(mes)};
            mes_.text = std::move(data);
            auto req = prepare_request(std::move(mes_));
            co_await session_t::write_request(std::move(req));
          };

          print("\n\n_________________SEND_NONE_MESSAGE___________________\n\n");
          co_await Commands::NothingMessage{}(callback);
          co_return;
        }

        default:
          co_return;
      }
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
            co_await send_command(std::move(mes.value()));
        };

        if (!res.ok)
        {
            co_return;
        }

        if (!res.update_id.has_value())
        {
            co_return;
        }


        update_id(res);
        size_t id = UpdateStorage_.update_stack.top().value();
        bool ok = UpdateStorage_.updated_set.insert(id);
        if(!ok)
        {
            co_return;
        }

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

        bool reconnect = false;

        while(true)
        {
          if(reconnect)
          {
            reconnect = false;
            co_await run();
          }

          try
          {
            TG::TelegramResponse resp{};
            resp = co_await
            start_getUpdates<TG::TelegramResponse, false>();
            co_await parse_result(std::move(resp));
            /*std::jthread th
            {
                [&]() -> boost::asio::awaitable<void>
                {
                  co_await parse_result(std::move(resp));
                }
            };
            th.join();*/
          }
          catch(const std::exception& ex)
          {
            print(ex.what());
            session_interface<PROTOCOL::HTTPS>::shutdown();
            stream_ = std::make_unique<stream_type>(ex_, ctx_);
            reconnect = true;
          }
      }
    }

    protected:

    template<bool isForce = false, bool isLast = false>
    [[nodiscard]]
    net::awaitable<std::optional<http::request<http::string_body>>>
    prepare_getUpdates(Pars::TG::getUpdates upd)
    {
        upd.timeout = Timer::timeout.count();
        auto opt = UpdateStorage_.update_stack.top();
        if (!opt.has_value() || isLast)
        {
            upd.offset = -1;
        } 
        else
        {
            upd.offset = UpdateStorage_.update_stack.top();
            print("\n\nupdate offset:", upd.offset.value(),"\n\n");
        }

        print("dif Time:", Timer::get_dif(),"\n");
        if (Timer::get_dif() > Timer::timeout || isForce == true)
        {
            if(Timer::update())
            {
                co_return prepare_request(std::move(upd));
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
                obj = co_await req_res<Res>(std::move(opt_req).value());
            }
            else
            {
                obj = co_await read_response<Res>();
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

    public:

    net::awaitable<void>
    run() override
    {
      co_await session_interface<PROTOCOL::HTTPS>::run();
    }

};



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

    json::string bot_token = argv[1];
    json::string host   = "api.telegram.org";
    json::string port   = "443";
    int version = 11;

    try
    {         
        net::io_context ioc;
        ssl::context ctx{ssl::context::tlsv13_client};
        ctx.set_default_verify_paths();
        ctx.set_verify_mode(ssl::verify_peer);

        net::co_spawn
        (
          ioc, 
          session_interface<PROTOCOL::HTTPS>::make_session<tg_session>
          (version, host, port, true, std::move(ctx), bot_token), 
          boost::asio::detached
        );
        ioc.run();
    }
    catch(const std::exception& e)
    {
        std::cerr<<std::stacktrace::current()<<std::endl;
        std::cerr << e.what() << '\n';
    }
  

    return EXIT_SUCCESS;
}
