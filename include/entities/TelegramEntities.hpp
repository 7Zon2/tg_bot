#pragma once
#include "tg_pars.hpp"

namespace Pars
{
    namespace TG
    {
        template<typename Derived>
        struct TelegramEntities
        {
            public:

            [[nodiscard]]
            static
            std::optional<std::unordered_map<json::string, json::value>>  
            requested_fields(const json::value& val)
            {
                return Derived::requested_fields(val); 
            }

            [[nodiscard]]
            static
            std::unordered_map<json::string, json::value> 
            optional_fields(const json::value& val)
            {
                return Derived::optional_fields(val);
            }

            [[nodiscard]]
            Derived
            fields_from_map(const std::unordered_map<json::string, json::value>& map)
            {
                return static_cast<Derived&>(*this).fields_from_map(map);
            }

            public:

            template<as_json_value T>
            [[nodiscard]]
            static std::optional<Derived>
            get_request (T && val) noexcept
            {
                using namespace boost;

                system::error_code er;

                if( val.is_object() == false)
                {
                    return std::nullopt;
                }

                json::object obj = val.as_object();
                auto it = obj.find("user");

                if(it == obj.end())
                {
                    return std::nullopt;
                }

                return Derived{};
            }


            [[nodiscard]]
            json::value
            entity_to_value()
            {
                return static_cast<Derived&>(*this).fields_to_value();
            }


            template<typename T>
            requires std::is_same_v<std::decay_t<T>, Derived>
            [[nodiscard]]
            static 
            std::optional<Derived>
            verify_fields(const json::value& val, T&& obj)
            {

                auto req_map = Derived::requested_fields(val);
                if(! req_map.has_value())
                {
                    return std::nullopt;
                }

                auto opt_map = Derived::optional_fields(val);

                auto map = std::move(req_map.value());

                for(auto && i : opt_map)
                {
                    map.insert_or_assign(std::move(i.first), std::move(i.second));
                }

                obj.fields_from_map(map);
                return std::forward<T>(obj); 
            }


            [[nodiscard]]
            static 
            std::optional<std::unordered_map<json::string,json::value>>
            verify_fields(const json::value& val)
            {

                auto req_map = Derived::requested_fields(val);
                if(! req_map.has_value())
                {
                    return std::nullopt;
                }

                auto opt_map = Derived::optional_fields(val);

                auto map = std::move(req_map.value());

                for(auto && i : opt_map)
                {
                    map.insert_or_assign(std::move(i.first), std::move(i.second));
                }

                return  map;
            }

            virtual ~TelegramEntities() = 0;
        };

        template<typename Derived>
        TelegramEntities<Derived>::~TelegramEntities(){}

    }//namespace TG

}//namespace Pars
