#include "head.hpp"


template<typename T>
concept is_http_message = requires(T&& mes)
{
  typename std::decay_t<T>::fields_type;
  typename std::decay_t<T>::header_type;
  typename std::decay_t<T>::body_type;
};


class Decoder final  
{

  public:

  enum class ENCODE
  {
    NONE = 0,
    gzip = 1
  };


  enum class CONTENT
  {
    NONE = 0,
    jpeg = 1,
    png  = 2
  };

  public:

  struct data_storage final
  {
    json::string data;
    CONTENT content_type = CONTENT::NONE;
    ENCODE encoding_type = ENCODE::NONE;
  
    public:
    
    data_storage() noexcept{}

    data_storage
    (
     json::string data,
     CONTENT content_type,
     ENCODE encoding_type
    ) 
    noexcept:
    data(std::move(data)),
    content_type(content_type),
    encoding_type(encoding_type)
    {}

    void 
    dump_data
    (json::string_view filename)
    {
      json::string name = filename;
      if(content_type == CONTENT::jpeg)
      {
        name+=".jpg";
      }
      else if(content_type == CONTENT::png)
      {
        name += ".png";
      }
      else
      {
        name+=".bin";
      }
      Pars::dump_data(name, data);
    }
    
  };

  public:


  [[nodiscard]]
  static json::string 
  encode_base64(json::string_view str)
  {
    json::string dest(str.size()*2,' ');
    boost::beast::detail::base64::encode(dest.data(), str.data(), str.size());
    return dest;
  }


  [[nodiscard]]
  static json::string
  decode_base64(json::string_view str)
  {
    json::string dest(str.size()*2, ' ');
    boost::beast::detail::base64::decode(dest.data(), str.data(), str.size());
    return dest;
  }


  [[nodiscard]]
  static json::string
  decode_gzip(json::string_view data)
  {
    boost::iostreams::array_source src{data.data(), data.size()};
    boost::iostreams::filtering_istream is;
    boost::iostreams::gzip_decompressor gz{};

    is.push(gz);
    is.push(src);

    std::string decode_str{};
    decode_str.reserve(data.size());

    boost::iostreams::back_insert_device ins {decode_str};
    boost::iostreams::copy(is, ins);

    return json::string{std::move(decode_str)}; 
  }


  template<is_http_message T>
  [[nodiscard]]
  static data_storage
  decode_data(T && mes)
  {
    data_storage data;

    auto it = mes.find(http::field::content_encoding);
    if(it == mes.end())
    {
      print("\n\nContent encoding field wasn't found\n\n");
      data.data = json::string{std::forward<T>(mes).body()};
      return data;
    }

    json::string encoding_type = it->value();
    print("\n~~~~~encoding_type~~~~~~~~~\n", encoding_type);
    if(encoding_type == "gzip")
    {
      data.encoding_type = ENCODE::gzip;
      data.data = decode_gzip(mes.body());
    }
    else
    {
      data.data = json::string{std::forward<T>(mes).body()};
    }

    
    json::string content_type;
    it = mes.find(http::field::content_type);
    if(it != mes.end())
    {
      content_type = it->value();
      print("\n~~~~~~~~content_type~~~~~~~~~~~\n", content_type);
    }

    if(content_type == "image/jpeg")
    {
      data.content_type = CONTENT::jpeg;
    }
    if(content_type == "image/png")
    {
      data.content_type = CONTENT::png;
    }

    return data;
  }


  [[nodiscard]]
  static json::string
  decode_url
  (json::string_view url)
  {
    boost::urls::pct_string_view vw{url};
    std::string buf;
    buf.resize(vw.decoded_size());
    vw.decode({}, boost::urls::string_token::assign_to(buf));
    return json::string{std::move(buf)};
  }

}; //class Decoder
