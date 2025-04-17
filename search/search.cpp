#include "boost/beast/http/message.hpp"
#include "boost/beast/http/status.hpp"
#include "boost/beast/http/string_body.hpp"
#include "boost/beast/http/verb.hpp"
#include "boost/iostreams/filter/gzip.hpp"
#include "boost/iostreams/device/array.hpp"
#include "boost/iostreams/filtering_stream.hpp"
#include "boost/iostreams/copy.hpp"
#include "boost/iostreams/device/back_inserter.hpp"
#include "boost/system/detail/error_code.hpp"
#include "head.hpp"
#include "session_interface.hpp"
#include <stdexcept>


class Searcher : public session_interface<false>
{
  public:

  Searcher
  (
   int version,
   json::string host,
   json::string port, 
   net::any_io_executor executor
  )
  noexcept :
  session_interface
  (
    version,
    host,
    port,
    executor
  )
  {}

  
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
        "yandex.ru",
        R"(/images/search/?rpt=imageview&format=json&request={"blocks":[{"block":"b-page_type_search-by-image__link"}]})"
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


    print("\n",req.target(),"\n");
    auto res = co_await req_res(*stream_, req);
    http::status status = res.result();
    if
    (
      (status != http::status::found) && 
      (status != http::status::see_other) &&
      (status != http::status::accepted) &&
      (status != http::status::temporary_redirect)
    )
    {
      throw std::runtime_error{"not found"};
    }

    if(status == http::status::temporary_redirect)
    {
      co_await start_redirection(std::move(req), std::move(res));
    }
  }


  net::awaitable<http::response<http::string_body>>
  redirect(auto& req, json::string relative_url, auto...statuses)
  {
    http::response<http::string_body> res;
    auto status = http::status::temporary_redirect;
    while
    (
      (status == http::status::temporary_redirect)||
      ((status == statuses) && ...)
    )
    {
      res = co_await req_res(*stream_, req);
      status = res.result();
      auto it = res.find(http::field::location);
      if (it == res.end())
        break;

      json::string url = it->value();
      url = make_relative_url(std::move(url), relative_url);
      req.target(std::move(url));
    }

    co_return std::move(res);
  }


  net::awaitable<void> 
  start_redirection
  (
   http::request<http::string_body> req,
   http::response<http::string_body> res
  )
  {
    print("\nstart redirection...\n");
    res = co_await redirect(req, "/images");
    auto status = res.result();
    if(status != http::status::ok)
    {
      throw std::runtime_error{"\nurl image wasn't found\n"};
    }
    
    json::string url = "/images/search?";
    url += parse_answer(std::move(res));

    req = make_header(http::verb::get, host_, url);
    res = co_await redirect(req, "/images", http::status::found);
    status = res.result();
    if(status != http::status::ok)
    {
      throw std::runtime_error{"\nhtml answer wasn't found\n"};
    }

    auto doc = parse_html(res.body());
  }


  [[nodiscard]]
  json::string parse_answer(http::response<http::string_body> res)
  {
    boost::iostreams::array_source src{res.body().data(), res.body().size()};
    boost::iostreams::filtering_istream is;
    boost::iostreams::gzip_decompressor gz{};

    is.push(gz);
    is.push(src);

    std::string encode_js{};
    encode_js.reserve(res.body().size());

    boost::iostreams::back_insert_device ins {encode_js};
    boost::iostreams::copy(is, ins);

    json::value var = json::string{std::move(encode_js)};
    var = Pars::MainParser::try_parse_value(std::move(var));
    Pars::MainParser::pretty_print(std::cout, var);

    boost::system::error_code er;
    auto * it = var.find_pointer("/blocks/0/params/url", er);
    if(er)
      throw std::runtime_error{"\npointer wasn't found/n"};

    json::string url = it->as_string();
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
    net::co_spawn(ioc, session_interface<false>::make_session<Searcher>(11, Searcher::host_, "80"), net::detached); 
    ioc.run();
  }
  catch(const std::exception& ex)
  {
    print("\n\n", ex.what(), "\n\n");
  }

  return 0;
}
