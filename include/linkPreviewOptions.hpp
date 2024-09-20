#pragma once
#include "TelegramEntities.hpp"
#include "concept_entities.hpp"

namespace Pars
{
    namespace TG
    {
        struct LinkPreviewOptions : TelegramEntities<LinkPreviewOptions>
        {
            
            using TelegramEntities::operator=;

            optbool is_disabled;
            optstr url;
            optbool prefer_small_media;
            optbool prefer_large_media;
            optbool show_above_text;

            static const constexpr size_t req_fields = 0;
            static const constexpr size_t opt_fields = 5;

            public:

            LinkPreviewOptions(){}  

            LinkPreviewOptions
            (
                optbool is_disabled = {},
                optstrw url = {},
                optbool prefer_small_media = {},
                optbool prefer_large_media = {},
                optbool show_above_text = {}
            )
            :
             is_disabled(is_disabled),
             url(url),
             prefer_small_media(prefer_small_media),
             prefer_large_media(prefer_large_media),
             show_above_text(show_above_text)
            {

            }

            public:

            template<as_json_value T>
            [[nodiscard]]
            static
            opt_fields_map
            requested_fields(T&& val)
            {
                return fields_map{};
            }


            template<as_json_value T>
            [[nodiscard]]
            static
            fields_map
            optional_fields(T&& val)
            {
                return MainParser::mapped_pointers_validation
                (
                    std::forward<T>(val),
                    std::make_pair(JS_POINTER(LinkPreviewOptions, is_disabled), json::kind::bool),
                    std::make_pair(JS_POINTER(LinkPreviewOptions, url), json::kind::string),
                    std::make_pair(JS_POINTER(LinkPreviewOptions, prefer_small_media), json::kind::bool),
                    std::make_pair(JS_POINTER(LinkPreviewOptions, prefer_large_media), json::kind::bool),
                    std::make_pair(JS_POINTER(LinkPreviewOptions, show_above_text), json::kind::bool)
                );
            }


            template<is_opt_fields_map T>
            void 
            fields_from_map
            (T && map)
            {
                MainParser::fields_from_map
                <json::kind::bool>(std::forward<T>(map), MAKE_PAIR(is_disabled));

                MainParser::fields_from_map
                <json::kind::string>(std::forward<T>(map), MAKE_PAIR(url));

                MainParser::fields_from_map
                <json::kind::bool>(std::forward<T>(map), MAKE_PAIR(prefer_small_media));

                MainParser::fields_from_map
                <json::kind::bool>(std::forward<T>(map), MAKE_PAIR(prefer_large_media));

                MainParser::fields_from_map
                <json::kind::bool>(std::forward<T>(map), MAKE_PAIR(show_above_text));
            }

            [[nodiscard]]
            json::value
            fields_to_value()
            {
                return TelegramRequestes::linkPreviewOptions
                (
                    is_disabled,
                    url,
                    prefer_small_media,
                    prefer_large_media,
                    show_above_text
                );
            }
        };

    }//namespace TG

}//namespace Pars