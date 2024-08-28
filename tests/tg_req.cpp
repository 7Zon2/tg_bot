#include <iostream>
#include "../include/certif.hpp"
#include "../include/entities/entities.hpp"

using namespace Pars;
using namespace TG;

void test0()
{
    json::value v = TG::TelegramRequestes::get_user_request(1, true, "Raven", "Fairy");

    Pars::MainParser::pretty_print(std::cout, v);

    print("\n\nJson from map:\n\n");
    auto op_map = TG::User::verify_fields(v);
    if(op_map.has_value())
    {
        for(auto && i : op_map.value())
        {
            std::cout<<i.first<<":"<<i.second<<std::endl;
        }
    }
    else
    {
        std::cout<<"map is empty"<<std::endl;
    }
    print("\n\n");

    User user{};

    user.fields_from_map(op_map.value());
    v = user.fields_to_value(); 
    print("Json from User:\n");
    MainParser::pretty_print(std::cout, v);
}

void test1()
{

    json::value val = Pars::MainParser::parse_json_from_file
    ("/home/zon/keys/certif/serv/host.crt");

    json::string certif = Pars::MainParser::serialize_to_string(val);

    json::array arr{"Message", "Chat", "User", "ChatFullInfo"};

    val = Pars::TG::TelegramRequestes::setWebhook
    (
        "127.0.0.1",
        certif,
        std::nullopt,
        std::nullopt,
        arr,
        false,
        "7462084054:AAFMMUI_V8jEdzWSu-OPmuD-nKaLqdrzFOU"
    );

    Pars::MainParser::pretty_print(std::cout, val);

}

void test_api()
{
       json::string certif = CRTF::load_cert("/home/zon/keys/certif/serv/host.crt");

        json::string url = "api.telegram.org";
        url.append("/bot");
        url.append("7462084054:AAFMMUI_V8jEdzWSu-OPmuD-nKaLqdrzFOU/");
        url.append("setWebhook");

        json::array arr{"Message", "Chat", "User", "ChatFullInfo"};

        json::value val = Pars::TG::TelegramRequestes::setWebhook
        (
            url,
            certif,
            std::nullopt,
            std::nullopt,
            arr,
            false
        );

        json::string data = Pars::MainParser::serialize_to_string(val);
        Pars::MainParser::pretty_print(std::cout, val);

}


void get_updates_test()
{
    std::vector<json::string> arr{"first", "second", "third"};
    getUpdates obj{1,2,3, std::move(arr)};
    json::string url = obj.fields_to_url();
    print(url,"\n");
}


void eq_test()
{
    Pars::TG::User us;
    json::object ob;
    ob.emplace("id", 10);
    ob.emplace("is_bot", true);
    us = ob;
    json::string str = us.fields_to_url();
    print(str,"\n");
}


int main()
{
    eq_test();
    return 0;
}