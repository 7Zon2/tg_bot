#pragma once 
#include "print.hpp"
#include "json_head.hpp"
#include <functional>
#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <entities/entities.hpp>
#include <variant>

template<typename F>
concept is_coro = std::is_same_v<std::decay_t<F>, boost::asio::awaitable<typename F::value_type>>;

namespace Commands
{
    using namespace Pars;

    class CommandInterface
    {
        protected:

        json::string data_{};
        size_t command_offset{};

        protected:

        CommandInterface(json::string data):
        data_(std::move(data)){}

        public:

        template<typename Derived, typename Obj>
        static bool
        isCommand(Obj&& obj)
        {
            return Derived::isCommand(std::forward<Obj>(obj));
        }


        template<typename Self, typename Obj>
        auto prepare(this Self&& self, Obj&& obj)
        {
            return std::forward<Self>(self).prepare(std::forward<Obj>(obj));
        }
    };


    class NothingMessage : public CommandInterface
    {
        protected:

        TG::SendMessage mes_;

        NothingMessage(json::string data):
        CommandInterface(std::move(data)){}

        public:

        NothingMessage():
        CommandInterface("There is nothing. Where everything is gone?")
        {

        }


        template<TG::is_message Obj>
        void prepare
        (Obj&& obj)
        {
            mes_ = TG::SendMessage(obj.chat.id, data_);
        }


        template<typename F, typename...Args>
        boost::asio::awaitable<void> operator()(F&& sender, Args&&...args)
        {
            co_await std::invoke(std::forward<F>(sender), std::forward<Args>(args)..., std::move(mes_));
        }
    };


    class Echo : public NothingMessage
    {
        public:

        Echo()
            {
                command_offset = 5;
            }

        Echo(json::string data):
            NothingMessage(std::move(data))
            {
                command_offset = 5;
            }


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


            template<typename F, typename...Args>
            boost::asio::awaitable<void>
            operator()(F&& sender, Args&&...args)
            {
                size_t max_offset = 0;
                size_t offset = command_offset;
                const size_t limit = 2056;

                while(offset + limit < data_.size())
                {
                    if (data_.size() - offset > limit)
                    {
                        max_offset = limit;
                    }
                    else
                    {
                        max_offset =  data_.size() - offset;
                    }

                    json::string substr
                        {data_.begin() + offset,
                         data_.begin() + offset + max_offset};

                    TG::SendMessage mes;
                    mes.chat_id = mes_.chat_id;
                    mes.text = std::move(substr);
                    co_await std::invoke(std::forward<F>(sender), std::forward<Args>(args)..., std::move(mes));
                    offset = offset + limit;
                }

                json::string substr{data_.begin() + offset, data_.end()};
                if (!substr.empty())
                {
                    mes_.text = std::move(substr);
                    co_await std::invoke(std::forward<F>(sender), std::forward<Args>(args)..., std::move(mes_));
                }
            }


        template<TG::is_message Obj>
        void prepare
        (Obj&& mes)
        {
            mes_.chat_id = mes.chat.id;
            data_ = Utils::forward_like<Obj>(mes.text.value());
        }
    };

    class Gadget
    {
        using variant = std::variant<NothingMessage, Echo>;

        variant var_;

        public:

        Gadget(variant var):
                var_(std::move(var)){}

        template<typename F, typename...Args>
        boost::asio::awaitable<void>
        get_await ( F&& sender, Args&&...args)
        {
            auto coro_lambda = [&]<typename Obj>
            (Obj&& obj) -> boost::asio::awaitable<void>
            {
                     co_await std::forward<Obj>(obj)(std::forward<F>(sender), std::forward<Args>(args)...);
            };

            boost::asio::awaitable<void> aw;

           std::visit([&]<typename T>(T&& obj)
            {
                using Ret_type = std::decay_t<decltype(std::forward<T>(obj)(std::forward<F>(sender), std::forward<Args>(args)...))>;

                if constexpr(is_coro<Ret_type>)
                {
                    aw = coro_lambda(std::forward<T>(obj));
                }
            }, std::move(var_));

            co_await std::move(aw);
        }
    };


    template<Pars::TG::is_message T>
    inline Gadget
    prepare_command( T&& mes)
    {
        using namespace Pars;

        bool res = Echo::isCommand(mes);    
        if (res == false)
        {
            NothingMessage m{};
            m.prepare(std::forward<T>(mes));
            return  Gadget{std::move(m)};
        }

        Echo m{};
        m.prepare(std::forward<T>(mes));
        return Gadget{std::move(m)};
    }

} //namespace Command
