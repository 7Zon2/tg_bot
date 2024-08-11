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
            std::optional<std::unordered_map<json::string, json::value>>  
            requested_fields(const json::value& val)
            {
                return static_cast<Derived&>(*this).requested_fields(val); 
            }

            [[nodiscard]]
            std::unordered_map<json::string, json::value> 
            optional_fields(const json::value& val)
            {
                return static_cast<Derived&>(*this).optional_fields(val);
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


            [[nodiscard]]
            static
            std::optional<std::unordered_map<json::string, json::value>>
            verify_fields(const json::value& val)
            {
                Derived der{};

                auto req_map = der.requested_fields(val);
                if(! req_map.has_value())
                {
                    return std::nullopt;
                }

                auto opt_map = der.optional_fields(val);

                auto map = std::move(req_map.value());

                for(auto && i : opt_map)
                {
                    map.insert_or_assign(std::move(i.first), std::move(i.second));
                }

                return map; 
            }


            virtual ~TelegramEntities() = 0;
        };

        template<typename Derived>
        TelegramEntities<Derived>::~TelegramEntities(){}

    }//namespace TG

}//namespace Pars
