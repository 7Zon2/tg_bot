#include "session_interface.hpp"


class Searcher : public session_interface
{
  public:

  Searcher
  (
   json::string host,
   json::string port, 
   int version,
   ssl::context ctx,
   net::any_io_executor executor
  )
  noexcept :
  session_interface
  (
    host,
    port,
    version,
    std::move(ctx),
    executor
  )
  {
  }

  
  net::awaitable<void>
  run() override
  {
    co_await session_interface::run();
    co_await request_upload_image(boost::interprocess::file_mapping{}, "/home/zon/Downloads/Kiki.jpg");
  }


  net::awaitable<void>
  request_upload_image
  (boost::interprocess::file_mapping map, json::string filename)
  {
    map = boost::interprocess::file_mapping(filename.data(), boost::interprocess::read_only);
    boost::interprocess::mapped_region region(map, boost::interprocess::read_only);

    char * addr = static_cast<char*>(region.get_address());
    size_t size = region.get_size();

    json::string data{size,' '};
    std::copy(addr, addr+size, data.data());

    auto req = make_header
      (
        http::verb::post,
        "https://yandex.ru/images/search",
        R"(/?rpt=imageview&format=json&request={"blocks":[{"block":"b-page_type_search-by-image__link"}]})"
      );


    prepare_multipart
    (
      req,
      R"(image/jpeg)",
      R"("encoded_image")",
      R"("Kiki")",
      std::move(data),
      R"(gzip, deflate, br)"
    );

    auto res = co_await req_res(std::move(req));
    http::status status = res.result();
    if
    (
      (status != http::status::found) && 
      (status != http::status::see_other) &&
      (status != http::status::accepted)
    )
    {
      throw std::runtime_error{"not found"};
    }

    //json::string target = get_image_url(res);
    res = co_await make_simple_get("");
    co_return;
  }

  
  json::string
  get_image_url(http::response<http::string_body>& res)
  {

    auto find_decode_url = [this](const auto &url, size_t pos)
    {
      json::string dest{url.begin()+pos, url.end()};
      return encode_base64(dest);
    };

    auto find_url = [](const auto& url)
    {
      size_t beg = url.find("\"");
      size_t end = url.find_last_of("\"");
      if(beg == json::string::npos || end == json::string::npos)
      {
        throw std::runtime_error{"image url not found\n"};
      }
      beg+=1;
      return json::string(url.begin()+beg, url.begin()+end);
    };

    auto find_ref_tag = [&find_url](const auto& url)
    {  
      size_t beg = url.find("<A");
      size_t end = url.find_last_of("A>");
      if(beg == json::string::npos || end == json::string::npos)
      {
        throw std::runtime_error{"image url not found\n"};
      }
      auto url_ =  json::string{url.begin()+beg, url.begin() + end};
      return find_url(url_);
    };

    auto& body = res.body();
    json::string url = find_ref_tag(body);
    
    size_t pos = url.find(":", 7);
    if(pos == json::string::npos)
    {
      throw std::runtime_error{"image url not found\n"};
    }
    pos+=7;

    json::string encoded = find_decode_url(url, pos);
    url.erase(pos);
    url+=std::move(encoded);
    print("\nImage URL:", url,"\n");
    return url;
  }

  public:

  static inline json::string host_{"yandex.ru"};
};



int main()
{
  try
  {
    net::io_context ioc;
    net::co_spawn(ioc, session_interface::make_session<Searcher>(Searcher::host_, "443",11), net::detached); 
    ioc.run();
  }
  catch(const std::exception& ex)
  {
    print("\n\n", ex.what(), "\n\n");
  }

  return 0;
}
