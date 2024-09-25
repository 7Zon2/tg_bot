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

            template<typename Self>
            [[nodiscard]]
            json::value
            fields_to_value(this Self&& self)
            {
                return LinkPreviewOptions::fields_to_value
                (
                    self.is_disabled,
                    forward_like<Self>(self.url),
                    self.prefer_small_media,
                    self.prefer_large_media,
                    self.show_above_text
                );
            }


            [[nodiscard]]
            static json::value
            fields_to_value
            (
                optbool is_disabled = {},
                optstr url = {},
                optbool prefer_small_media = {},
                optbool prefer_large_media = {},
                optbool show_above_text = {}
            )
            {
                json::object ob{MainParser::get_storage_ptr()};
                ob = parse_OptPairs_as_obj
                    (
                        MAKE_OP(is_disabled),
                        MAKE_OP(std::move(url)),
                        MAKE_OP(prefer_small_media),
                        MAKE_OP(prefer_large_media),
                        MAKE_OP(show_above_text)
                    );

               json::object ob_2(MainParser::get_storage_ptr());
               ob_2["linkpreviewoptions"] = std::move(ob);
               return ob_2;
            }
        };

    }//namespace TG

}//namespace Pars