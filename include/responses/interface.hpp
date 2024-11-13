#pragma once 
#include "print.hpp"
#include "json_head.hpp"
#include <functional>



class Interface
{
    protected:

    using sender = std::move_only_function<void(json::string)>;

    json::string data_{};
    sender sender_{};

    protected:

    Interface(json::string data, sender fun):
    data_(std::move(data)), sender_(std::move(fun)){}

    public:

    template<typename Self>
    void send(this Self&& self)
    {
        self.send();
    } 
}; 



class NothingMessage : public Interface
{
    protected:

    NothingMessage(json::string data, sender fun):
    Interface(std::move(data), std::move(fun)){}

    public:

    NothingMessage(sender fun):
    Interface("There is nothing. Where everything is gone?")
    {

    }

    void send()
    {
        sender_(data_);
    }
};


class Echo : public NothingMessage
{
    public:

    Echo(json::string data, sender fun):
        NothingMessage(std::move(data), std::move(fun)){}

    
    void send()
    {
        if(data_.empty())
        {
            NothingMessage::send();
        }

        
    }
};




