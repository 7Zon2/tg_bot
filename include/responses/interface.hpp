#pragma once 
#include "print.hpp"
#include "json_head.hpp"
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

        protected:

        json::string data_{};
        size_t command_offset{};
        T& session_;

        protected:

        CommandInterface(T& session, json::string data):
        session_(session), data_(std::move(data)){}


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
                Echo<T> com(session);
                com.prepare(std::forward<Obj>(mes));
                co_await com();
                co_return;
            }

            NothingMessage<T> com(session);
            com.prepare(std::forward<Obj>(mes));
            co_await com();
            co_return;
        }

        public:

        template<typename Derived, typename Obj>
        static bool
        isCommand(Obj&& obj)
        {
            return Derived::isCommand(std::forward<Obj>(obj));
        }

        protected:

        template<typename Self, typename Obj>
        auto prepare(this Self&& self, Obj&& obj)
        {
            return std::forward<Self>(self).prepare(std::forward<Obj>(obj));
        }
    };

    template<typename T>
    CommandInterface<T>::~CommandInterface(){}


    template<typename T>
    class NothingMessage : public CommandInterface<T>
    {
        protected:

        TG::SendMessage mes_;

        NothingMessage(T& session, json::string data):
        CommandInterface<T>(session, std::move(data)){}


        public:

        NothingMessage(T& session):
        CommandInterface<T>(session, "There is nothing. Where everything is gone?")
        {

        }

        ~NothingMessage(){}

        template<TG::is_message Obj>
        void prepare
        (Obj&& obj)
        {
            mes_ = TG::SendMessage(obj.chat.id, this->data_);
            this->type_ = CommandType::Message;
        }


        boost::asio::awaitable<void> operator()()
        {
            co_await this->session_.template send_response<false,true>(std::move(mes_));
        }
    };


    template<typename T>
    class Echo : public NothingMessage<T>
    {
        public:

            Echo(T& session) :
            NothingMessage<T>(session)
            {
                Echo::command_offset = 5;
            }

            Echo(T& session, json::string data):
            NothingMessage<T>(std::move(data))
            {
                Echo::command_offset = 5;
            }

            ~Echo(){}

            template<TG::is_message Obj>
            [[nodiscard]]
            static bool isCommand
            (Obj&& mes)
            {
                const static json::string command{"/echo"};
                json::string& ref = mes.text.value();

                if(ref.empty())
                    return false;

                json::string substr{ref.begin(), ref.begin()+command.size()};
                if(substr != command)
                    return false;
                else
                    return true;
            }


            boost::asio::awaitable<void>
            operator()()
            {
                size_t max_offset = 0;
                size_t offset = Echo::command_offset;
                const size_t limit = 2056;
                json::string data = std::move(Echo::data_);

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
                    co_await Echo::session_.template send_response<false,true>(std::move(mes));
                    offset = offset + limit;
                }

                json::string substr{data.begin() + offset, data.end()};
                if (!substr.empty())
                {
                    Echo::mes_.text = std::move(substr);
                    co_await Echo::session_.template send_response<false,true>(std::move(Echo::mes_));
                }
            }


        template<TG::is_message Obj>
        void prepare
        (Obj&& mes)
        {
            if (mes.document.has_value())
            {
                Echo::type_ = CommandType::File;
                return;
            }

            Echo::mes_.chat_id = mes.chat.id;
            Echo::data_ = Utils::forward_like<Obj>(mes.text.value());
            Echo::type_ = CommandType::Message;
        }
    };


    template<typename S, Pars::TG::is_message T>
    [[nodiscard]]
    inline std::unique_ptr<CommandInterface<S>>
    find_command(S& session, T&& mes)
    {
        using namespace Pars;

        if (Echo<S>::isCommand(mes))
        {
            auto ptr =  std::make_unique<Echo<S>>(session);
            ptr->prepare(std::forward<T>(mes));
            return ptr;
        }

        auto ptr = std::make_unique<NothingMessage<S>>(session);
        ptr->prepare(std::forward<T>(mes));
        return std::make_unique<NothingMessage<S>>(session);
    }

} //namespace Command
