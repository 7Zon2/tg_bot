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

        ~NothingMessage() = 0;
  
      public:

        [[nodiscard]]
        static bool 
        isCommand(json::string_view str) noexcept 
        {
          return str.empty();
        }


        net::awaitable<void> 
        static operator()
        (session_base & session, json::string_view url)
        {
          json::string host = session.get_host();
          http::request<http::string_body> req = 
          session.make_header
          (
            http::verb::get,
            host,
            url
          );
          req.body() = "There is Nothing. Where everything is gone?";
          co_await session.req_res(std::move(req));
        }

    };//NothingMessage

    inline NothingMessage::~NothingMessage(){}


    class Echo : public NothingMessage
    {
      protected:

        static inline json::string command_{"echo"};

      protected:

        [[nodiscard]]
        static size_t 
        get_offset() noexcept
        {
          size_t offset = command_prefix_.size();
          offset += command_.size();
          return offset;
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
          json::string command = command_prefix_;
          command += command_; 
          return str == command;
        }


        boost::asio::awaitable<void>
        static operator()
        (
        session_base& session, 
        http::request<http::string_body> req_,
        json::string data,
        const size_t limit = 2056
        )
        {
          size_t max_offset = 0;
          size_t offset = Echo::get_offset();
          json::string host = session.get_host();

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

            http::request<http::string_body> req = req_;
            req.body() = std::move(substr);
            co_await session.write_request(std::move(req));
            offset = offset + limit;
          }

          json::string substr{data.begin() + offset, data.end()};
          if (!substr.empty())
          {
            http::request<http::string_body> req = req_;
            req.body() = std::move(substr);
            co_await session.write_request(std::move(req));
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
