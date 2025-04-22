#pragma once
#include "boost/beast/http/message.hpp"
#include "boost/beast/http/string_body.hpp"
#include "session_interface.hpp"
#include <entities/File.hpp>
#include <entities/entities.hpp>


template<typename F>
concept is_coro = std::is_same_v<std::decay_t<F>, boost::asio::awaitable<typename F::value_type>>;

namespace Commands
{
    using namespace Pars;


    class NothingMessage;
    class Echo;
    class Search;


    enum class
    CommandType
    {
      None,
      Echo,
      Search
    };


    class CommandInterface
    {
      protected:

        CommandType type_ = CommandType::None;
        static inline json::string command_prefix_{"/"};

        session_base* session_;
        size_t command_offset{};

      protected:

        template<typename Self>
        void set_offset
        (this Self&& self) noexcept
        {
          self.set_offset();
        }

      protected:

        CommandInterface( session_base* session)
        noexcept:
        session_(session)
        {}

        CommandInterface() noexcept {}

        void set_session
        (session_base* session) noexcept 
        {
          session_ = session;
        }

        public:

        void
        set_command_prefix
        (json::string_view prefix) noexcept
        {
          command_prefix_  = prefix;
        }

        virtual ~CommandInterface() = 0;

      public:

        template<typename Derived, typename Obj>
        static bool
        isCommand(json::string_view str) noexcept
        {
            return Derived::isCommand(str);
        }

    }; //CommandInterface

    inline CommandInterface::~CommandInterface(){}


    class NothingMessage : public CommandInterface
    {
      protected:

        static inline json::string command_{};

        void set_offset() noexcept
        {
          command_offset = 0;
        }

      public:

        NothingMessage(session_base* session)
        noexcept : CommandInterface(session)
        {}

        NothingMessage() noexcept {}

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



    class Echo : public NothingMessage
    {
      protected:

        static inline json::string command_{"echo"};

      protected:

        [[nodiscard]]
        static size_t 
        get_offset(json::string_view view) noexcept
        {
          json::string string_offset = command_prefix_;
          string_offset += command_;

          print("\nstring_offset:", string_offset,"\n");
          print("\nview:", view,"\n");
          if(view.size() <= string_offset.size())
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

      public:

        Echo(session_base* session) 
        noexcept : NothingMessage(session)
        {}

        Echo()noexcept{}

        ~Echo(){}

      public:

        [[nodiscard]]
        static 
        bool isCommand
        (std::string_view str)
        {
          size_t pos = get_offset(str); 
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


    [[nodiscard]]
    inline CommandType 
    get_command(json::string_view command) noexcept
    {

      if (Echo::isCommand(command))
      {
        return CommandType::Echo;
      }
          
      return CommandType::None;
    }

} //namespace Command
