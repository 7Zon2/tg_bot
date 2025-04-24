#pragma once
#include "boost/beast/http/message.hpp"
#include "boost/beast/http/string_body.hpp"
#include "session_interface.hpp"
#include <concepts>
#include <type_traits>
#include "searcher/searcher.hpp"


namespace Commands
{

    template<typename F>
    concept is_coro = std::is_same_v<std::decay_t<F>, boost::asio::awaitable<typename F::value_type>>;


    template<typename Derived>
    class CommandInterface;


    using namespace Pars;

    enum class
    CommandType
    {
      None,
      Echo,
      Search
    };


    template<typename  Derived>
    class CommandInterface
    {
      protected:

        static inline json::string command_prefix_{"/"};
        session_base* session_;

      protected:

        [[nodiscard]]
        static size_t 
        get_offset(json::string_view view) noexcept
        {
          json::string string_offset = command_prefix_;
          string_offset += Derived::command_;

          if(view.size() < string_offset.size())
          {
            return 0;
          }

          json::string substr = view.substr(0, string_offset.size());
          if (substr == string_offset)
          {
            return substr.size();
          }
          return 0;
        }

      protected:

        CommandInterface(session_base* session)
        noexcept:
        session_(session)
        {}


        CommandInterface() 
        noexcept {}

      public:

        void set_session
        (session_base* session) noexcept 
        {
          session_ = session;
        }


        static void
        set_command_prefix
        (json::string_view prefix) noexcept
        {
          command_prefix_  = prefix;
        }

        virtual ~CommandInterface() = 0;

      public:

        static bool
        isCommand(json::string_view str) noexcept
        {
            return Derived::isCommand(str);
        }

    }; //CommandInterface

    template<typename Derived>
    inline CommandInterface<Derived>::~CommandInterface(){}


    class NothingMessage : public CommandInterface<NothingMessage>
    {
      public:

        const static inline json::string command_{};

      public:

        NothingMessage(session_base* session)
        noexcept: 
        CommandInterface<NothingMessage>(session)
        {}


        NothingMessage() 
        noexcept {}


        ~NothingMessage(){}
  
      public:

        [[nodiscard]]
        static bool 
        isCommand(json::string_view str) noexcept 
        {
          return str.empty();
        }


        net::awaitable<void> 
        static operator()
        (auto&& callback)
        {
          static const json::string mes{"There is Nothing. Where everything is gone?"};
          co_await callback(mes);
        }

    };//NothingMessage



    class Echo : public CommandInterface<Echo>
    {
      public:

        const static inline json::string command_{"echo"};

        using base_t = CommandInterface<Echo>;

      public:

        Echo(session_base* session) 
        noexcept : 
        base_t(session)
        {}


        Echo()
        noexcept{}


        ~Echo(){}

      public:

        [[nodiscard]]
        static bool 
        isCommand
        (std::string_view str) noexcept 
        {
          size_t pos = base_t::get_offset(str); 
          return pos;
        }


        boost::asio::awaitable<void>
        static operator()
        (
          auto&& callback,
          json::string data,
          const size_t limit = 2056
        )
        {
          size_t max_offset = 0;
          size_t offset = Echo::get_offset(data);

          while(offset + limit < data.size())
          {
            if (data.size() - offset > limit)
            {
              max_offset = limit;
            }
            else
            {
              max_offset =  data.size() - offset;
            }

            json::string substr
            {data.begin() + offset,
            data.begin() + offset + max_offset};

            co_await callback(std::move(substr));
            offset = offset + limit;
          }

          json::string substr{data.begin() + offset, data.end()};
          if (!substr.empty())
          {
            co_await callback(std::move(substr));
          }

      }//Echo
    };


    class Search : public CommandInterface<Search>
    {
      public:

        const static inline json::string command_{"search"};
      
      protected:

        static inline std::shared_ptr<Searcher> searcher_;

        using base_t = CommandInterface<Search>;

      public:

        Search()
        noexcept{}


        Search(session_base* session)
        noexcept : 
        base_t(session)
        {}

        
        net::awaitable<void>
        static start()
        {
          searcher_ = co_await session_interface<PROTOCOL::HTTP>::
          make_session<Searcher>(11, "yandex.ru","80");
        }

      public:

        [[nodiscard]]
        static bool 
        isCommand
        (json::string_view view) noexcept 
        {
          size_t pos = base_t::get_offset(view);
          return pos;
        }
    
        
        template<typename F>
        net::awaitable<void>
        static operator ()
        (F&& callback, json::string data)
        {
          auto& searcher = *searcher_;
          co_await searcher(std::forward<F>(callback), std::move(data));
        }

      };//Search
    

    [[nodiscard]]
    inline CommandType 
    get_command(json::string_view command) noexcept
    {

      if (Echo::isCommand(command))
      {
        return CommandType::Echo;
      }
      if (Search::isCommand(command))
      {
        return CommandType::Search;
      }
          
      return CommandType::None;
    }

} //namespace Command
