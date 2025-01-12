#pragma once
#include "head.hpp"
#include "print.hpp"
#include "json_head.hpp"
#include <entities/File.hpp>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <entities/entities.hpp>


template<typename F>
concept is_coro = std::is_same_v<std::decay_t<F>, boost::asio::awaitable<typename F::value_type>>;

namespace Commands
{
    using namespace Pars;

    enum class
    CommandType
    {
        None,
        Message,
        File
    };

    template<typename T>
    class NothingMessage;

    template<typename T>
    class Echo;

    template<typename T>
    class CommandInterface
    {
        public:

        CommandType type_ = CommandType::None;

        template<typename Self>
        void set_commandType(this Self&& self) noexcept
        {
                self.set_CommandType();
        }

        protected:

        size_t command_offset{};
        T& session_;

        template<typename Self>
        void  set_offset(this Self&& self) noexcept
        {
            self.set_offset();
        }

        protected:

        CommandInterface(T& session, CommandType type):
                session_(session), type_(type){}

        public:

        CommandInterface(T& session):
                session_(session){}

        virtual ~CommandInterface() = 0;


        template<TG::is_message Obj>
        [[nodiscard]]
        boost::asio::awaitable<void>
        static exec(T& session, Obj&& mes)
        {
            using namespace Pars;

            if (Echo<T>::isCommand(mes))
            {
                Echo<T> com(session, std::forward<Obj>(mes));
                co_await com();
                co_return;
            }

            NothingMessage<T> com(session, std::forward<Obj>(mes));
            co_await com();
            co_return;
        }

        public:

        template<typename Derived, typename Obj>
        static bool
        isCommand(Obj&& obj) noexcept
        {
            return Derived::isCommand(std::forward<Obj>(obj));
        }

    };

    template<typename T>
    CommandInterface<T>::~CommandInterface(){}


    template<typename T>
    class NothingMessage : public CommandInterface<T>
    {
        protected:

        TG::SendMessage mes_;

        void  set_offset() noexcept
        {
            NothingMessage::command_offset = 0;
        }

        public:

        void set_commandType() noexcept
        {
            this->type_ = CommandType::Message;
        }


        NothingMessage(T& session, TG::SendMessage mes):
            CommandInterface<T>(session), mes_(std::move(mes))
        {
        }


        NothingMessage(T& session):
        CommandInterface<T>(session, CommandType::Message)
        {
        }

        ~NothingMessage(){}

        public:

        boost::asio::awaitable<void> operator()()
        {
            set_offset();
            mes_.text  = "There is Nothing. Where everything is gone?";
            co_await this->session_.send_response(std::move(mes_));
            co_await this->session_.template read_response();
        }
    };


    template<typename T>
    class Echo : public NothingMessage<T>
    {
        protected:

        boost::asio::awaitable<void>
        onFile()
        {
            TG::Document& doc = Echo::mes_.document.value();
            http::request<http::string_body> req = Echo::session_.GetFileRequest(doc.file_id);
            co_await Echo::session_.send_response(std::move(req));

            TG::TelegramResponse res;
            TG::File file;
            for(;;)
            {
                res = co_await
                Echo::session_.template read_response();
                if (res.ok && res.result.has_value())
                {
                    file = std::move(res.result).value();
                    if (file.file_path.has_value())
                    {
                        break;
                    }
                }
            }


            req = Echo::session_.template prepare_request
                  <false>(std::move(file.file_path).value(), "file");

            co_await Echo::session_.send_response(std::move(req));
            using type = http::response<http::string_body>;
            type res_b = co_await Echo::session_.template
                  read_response<type>();

            Echo::mes_.text = std::move(res_b).body();
        }

        void set_offset() noexcept
        {
            Echo::command_offset = 5;
        }

        public:

            Echo(T& session) :
            NothingMessage<T>(session)
            {
            }


            Echo(T& session, TG::SendMessage mes):
                NothingMessage<T>(session, std::move(mes))
            {
            }

            ~Echo(){}


            void set_commandType() noexcept
            {
                if (this->mes_.document.has_value())
                {
                    this->type_ = CommandType::File;
                }
                else if (this->mes_.text.has_value())
                {
                    this->type_ = CommandType::Message;
                }
                else
                {
                    this->type_ = CommandType::None;
                }
            }

            template<TG::is_message Obj>
            [[nodiscard]]
            static bool isCommand
            (Obj&& mes)
            {
                const static json::string command{"/echo"};
                if (mes.document.has_value())
                {
                    return true;
                }

                json::string& ref = mes.text.value();
                if(ref.empty())
                {
                    return false;
                }

                json::string substr{ref.begin(), ref.begin()+command.size()};
                return substr == command;
            }


            boost::asio::awaitable<void>
            operator()()
            {
                set_commandType();
                switch (Echo::type_)
                {
                    case CommandType::File : co_await onFile(); break;

                    case CommandType::Message : set_offset(); break;

                    default: co_return;
                }

                size_t max_offset = 0;
                size_t offset = Echo::command_offset;
                const size_t limit = 2056;
                json::string data = std::move(Echo::mes_).text.value();


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

                    TG::SendMessage mes;
                    mes.chat_id = Echo::mes_.chat_id;
                    mes.text = std::move(substr);
                    co_await Echo::session_.send_response(std::move(mes));
                    co_await Echo::session_.template read_response();
                    offset = offset + limit;
                }

                json::string substr{data.begin() + offset, data.end()};
                if (!substr.empty())
                {
                    Echo::mes_.text = std::move(substr);
                    co_await Echo::session_.send_response(std::move(Echo::mes_));
                    co_await Echo::session_.template read_response();
                }
            }
    };
} //namespace Command
