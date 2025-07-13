#include "TelegramEntities.hpp"
#include "boost/json/kind.hpp"
#include "boost/system/detail/error_code.hpp"
#include "json_head.hpp"


namespace YandexEntities
{
  using namespace Pars;

  class Thumb : public TG::TelegramEntities<Thumb>
  {
    public:

    using TG::TelegramEntities<Thumb>::operator=;

    json::string imageUrl;
    size_t width = 0;
    size_t height = 0;
    json::string title;
    json::string linkUrl;

    public:

    static const inline json::string entity_name{FIELD_NAME(Thumb)};
    static const constexpr size_t req_fields = 5;
    static const constexpr size_t opt_fields = 0;

    json::string 
    get_entity_name() noexcept override
    {
      return Thumb::entity_name;
    }

    public:

    Thumb() noexcept {}


    template<is_all_json_entities T>
    Thumb(T&& obj)
    {
      create(std::forward<T>(obj));
    }


    Thumb
    (
      json::string imageUrl,
      size_t width,
      size_t height,
      json::string title,
      json::string linkUrl
    )
    noexcept:
    imageUrl(std::move(imageUrl)),
    width(width),
    height(height),
    title(title),
    linkUrl(std::move(linkUrl))
    {

    }

    public:

    template<typename T>
    [[nodiscard]]
    static fields_map 
    optional_fields(T&& val)
    {
      return {};
    }


    template<typename T>
    [[nodiscard]]
    static opt_fields_map 
    requested_fields(T&& val)
    {
      auto map = MainParser::mapped_pointers_validation
        (
          std::forward<T>(val),
          std::make_pair(JSP(imageUrl), json::kind::string),
          std::make_pair(JSP(width), json::kind::int64),
          std::make_pair(JSP(height), json::kind::int64),
          std::make_pair(JSP(title), json::kind::string),
          std::make_pair(JSP(linkUrl), json::kind::string)
        );

      if(req_fields != map.size())
      {
        return {};
      }

      return map;
    }


    template<is_fields_map T>
    void 
    fields_from_map(T && map)
    {
      MainParser::field_from_map
      <json::kind::string>(std::forward<T>(map), RFP(imageUrl, imageUrl));
      
      MainParser::field_from_map
      <json::kind::int64>(std::forward<T>(map), RFP(width, width));

      MainParser::field_from_map
      <json::kind::int64>(std::forward<T>(map), RFP(height, height));

      MainParser::field_from_map
      <json::kind::string>(std::forward<T>(map), RFP(title, title));

      MainParser::field_from_map
      <json::kind::string>(std::forward<T>(map), RFP(linkUrl, linkUrl));
    }


    template<typename Self>
    [[nodiscard]]
    json::value
    fields_to_value(this Self&& self)
    {
      return Thumb::fields_to_value
        (
          Utils::forward_like<Self>(self.imageUrl),
          self.width,
          self.height,
          Utils::forward_like<Self>(self.title),
          Utils::forward_like<Self>(self.linkUrl)
        );
    }


    [[nodiscard]]
    static json::value
    fields_to_value
    (
      json::string imageUrl,
      size_t width,
      size_t height,
      json::string title,
      json::string linkUrl
    )
    {
      json::object ob{MainParser::get_storage_ptr()};
      ob = MainParser::parse_ObjPairs_as_obj
        (
          PAIR(imageUrl, std::move(imageUrl)),
          PAIR(width, width),
          PAIR(height, height),
          PAIR(title, std::move(title)),
          PAIR(linkUrl, linkUrl)
        );

      json::object res{MainParser::get_storage_ptr()};
      res[Thumb::entity_name] = std::move(ob);
      return res;
    }

  };


  //main json object that stores all other entities
  template<as_json_value T>
  json::value *
  find_InitialState
  (T && val)
  {
    boost::system::error_code er;
    json::value * pv = val.find_pointer("/initialState", er);
    return pv;
  }


  template<as_json_value T>
  auto find_cbirSimilar_Thumbs
  (T && val) -> std::optional<std::pmr::vector<YandexEntities::Thumb>>
  {
    std::pmr::vector<YandexEntities::Thumb> vec_thumbs;
    boost::system::error_code er;
    json::value * pv = val.find_pointer("/cbirSimilar/thumbs", er);
    if(!pv)
    {
      print("\nCbirSibilar thumbs wasn't found\n");
      return {};
    }

    json::array& arr = pv->as_array();
    vec_thumbs.reserve(arr.size());
    MainParser::container_move(arr, vec_thumbs);
    return vec_thumbs;
  };



  class YandexRoot : TG::TelegramEntities<YandexRoot>
  {

  };


} //YandexEntities
