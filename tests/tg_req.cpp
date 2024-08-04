#include "../include/tg_pars.hpp"


using namespace Pars;
using namespace TG;

int main()
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

    return 0;
}