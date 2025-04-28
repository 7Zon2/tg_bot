#pragma once
#include "entities/tg_message.hpp"
#include "json_head.hpp"
#include "sendMessage.hpp"
#include "session_interface.hpp"
#include <type_traits>



namespace Pars
{
  namespace TG
  {

    struct SendPhoto : public SendMessage
    {
        using SendMessage::operator =;
        using message::photo;

        static inline const json::string entity_name{"sendphoto"};
        static constexpr size_t req_fields = 2;
        static constexpr size_t opt_fields = 13; //this struct is not full 

        [[nodiscard]] 
        json::string 
        get_entity_name() 
        noexcept override 
        {
          return entity_name;
        }
      
      public:

        optstr photo_data{};
        optstr photo_url{};

      public:

        SendPhoto()
        noexcept{}

        SendPhoto(const message& mes):
        SendMessage(mes){}


        SendPhoto(message&& mes)
        noexcept: 
        SendMessage(std::move(mes))
        {}


        SendPhoto& operator = 
        (const message& mes)
        {
          SendPhoto temp{mes};
          *this = std::move(temp);
          return *this;
        }


        SendPhoto& operator =
        (message&& mes) noexcept 
        {
          SendPhoto temp{std::move(mes)};
          *this = std::move(temp);
          return *this;
        }


        SendPhoto
        (
          const size_t chat_id,
          optstr photo_data,
          optstr photo_url
        )
        noexcept :
        SendMessage(chat_id,""),
        photo_data(std::move(photo_data)),
        photo_url(std::move(photo_url))
        {}


        template<is_all_json_entities T>
        SendPhoto(T&& val)
        {
          create(std::forward<T>(val));
        }

      public:

        template<typename Self> 
        [[nodiscard]]
        http::request<http::string_body>
        fields_to_url(this Self&& self)
        noexcept (std::is_rvalue_reference_v<Self>)
        {
          assert(self.photo_url || self.photo_data);

          json::string id{FIELD_EQUAL(chat_id)};
          id += std::to_string(self.chat_id);

          if( ! self.photo_url)
          {
            auto req = session_base::make_header(http::verb::post, "", "");
            session_base::prepare_multipart
            (
              req, 
              "image/jpeg",
              R"(encoded_image)",
              R"(kartinka)",
              Utils::forward_like<Self>(self.photo_data).value(),
              "gzip, deflate, br"
            );

            
            json::string photo{FIELD_EQUAL(photo)};
            //photo += std::move(req).body();

            json::string url{URL_REQUEST(sendPhoto)};
            URL_BIND(url, id);
            URL_BIND(url, photo);
            url.pop_back();

            req.target(std::move(url));
            return req;
          }

          json::string url{FIELD_EQUAL(photo)};
          url += Utils::forward_like<Self>(self.photo_url).value();

          json::string req{URL_REQUEST(sendPhoto)};
          URL_BIND(req, id);
          URL_BIND(req, url);
          req.pop_back();

          return session_base::make_header(http::verb::get,"",std::move(req));
        }


        template<as_json_value T>
        [[nodiscard]]
        static opt_fields_map
        requested_fields(T&& val)
        {
          auto map = MainParser::mapped_pointers_validation
          (
            std::forward<T>(val),
            std::make_pair(JS_POINTER(sendphoto, chat_id), json::kind::int64),
            std::make_pair(JS_POINTER(sendphoto, photo_url), json::kind::string),
            std::make_pair(JS_POINTER(sendphoto, photo_data), json::kind::string)
          );

          if(map.size() != req_fields)
          {
            return std::nullopt;
          }

          return map;
        }


        template<as_json_value T>
        [[nodiscard]]
        static opt_fields_map 
        optional_fields(T&& val)
        {
          return {};
        }
  

        template<is_fields_map T>
        void 
        fields_from_map
        (T && map)
        {
          MainParser::field_from_map
          <json::kind::int64>(std::forward<T>(map), MAKE_PAIR(chat_id, chat_id));

          MainParser::field_from_map
          <json::kind::string>(std::forward<T>(map), MAKE_PAIR(photo_url, photo_url));

          MainParser::field_from_map
          <json::kind::string>(std::forward<T>(map), MAKE_PAIR(photo_data, photo_data));
        }



        template<typename Self>
        [[nodiscard]]
        json::value 
        fields_to_value(this Self&& self)
        noexcept (std::is_rvalue_reference_v<Self>)
        {
          return SendPhoto::fields_to_value
          (
            self.chat_id,
            Utils::forward_like<Self>(self.photo_url),
            Utils::forward_like<Self>(self.photo_data)
          );
        }


        [[nodiscard]]
        static json::value
        fields_to_value
        (
          size_t chat_id,
          optstr photo_url,
          optstr photo_data
        )
        {
          json::object req_ob(MainParser::get_storage_ptr());
          req_ob = MainParser::parse_ObjPairs_as_obj
            (
              PAIR(chat_id, chat_id)
            );


          json::object opt_ob(MainParser::get_storage_ptr());
          opt_ob = MainParser::parse_OptPairs_as_obj
            (
              MAKE_OP(photo_url, std::move(photo_url)),
              MAKE_OP(photo_data, std::move(photo_data))
            );

          Pars::MainParser::container_move(std::move(opt_ob), req_ob);
          return req_ob;
        }

    };//SendPhoto

  }//namespace TG

}//namespace Pars
