#pragma once
#include "concept_entities.hpp"
#include "tg_requestes.hpp"

namespace Pars
{
    namespace TG
    {
        template<typename Derived>
        struct TelegramEntities
        {
            public:

            static inline size_t req_fields = Derived::req_fields;
            static inline size_t opt_fields = Derived::opt_fields;

            public:
            
            template<is_all_json_entities T>
            void operator=(T&& val)
            {
                json::value val_ = std::forward<T>(val);
                auto map = verify_fields(val_);
                if (map.has_value())
                {
                    fields_from_map(std::move(map.value()));
                }
            }
            

            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(json::value val)
            {
                return Derived::requested_fields(std::move(val)); 
            }


            [[nodiscard]]
            static
            fields_map
            optional_fields(json::value val)
            {
                return Derived::optional_fields(std::move(val));
            }


            template<is_fields_map T>
            void
            fields_from_map(T&& map)
            {
                return static_cast<Derived&>(*this).fields_from_map(std::forward<T>(map));
            }


            [[nodiscard]]
            json::string
            fields_to_url() 
            {
                return static_cast<Derived&>(*this).fields_to_url();
            }

            public:

            template<typename Self>
            [[nodiscard]]
            json::value
            entity_to_value(this Self&& self)
            {
                return self.fields_to_value();
            }


            [[nodiscard]]
            static 
            opt_fields_map
            verify_fields(json::value val)
            {
                auto req_map = Derived::requested_fields(std::move(val));
                if(! req_map.has_value())
                {
                    return std::nullopt;
                }

                auto opt_map = Derived::optional_fields(std::move(val));

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
