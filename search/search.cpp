#include "head.hpp"
#include "session_interface.hpp"


class Searcher : public session_interface<PROTOCOL::HTTP>
{
  protected:

  using session_interface<PROTOCOL::HTTP>::req_res;
  using session_interface<PROTOCOL::HTTP>::write_request;
  using session_interface<PROTOCOL::HTTP>::read_response;

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
    json::string data = load_file(filename);

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
      default_encoding_
    );


    print("\n",req.target(),"\n");
    auto res = co_await session_base::req_res(*stream_, req);
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


  [[nodiscard]]
  net::awaitable<http::response<http::string_body>>
  make_oneshot_session(json::string_view url)
  {
    print("\nStart OneShot session\n");
    json::string host = make_host(url);
    json::string target = make_relative_url(url);
    auto req = make_header(http::verb::get, host, target); 
    req.set(http::field::accept_encoding, default_encoding_);
    req.set(http::field::accept, "*/*");
    req.set(http::field::connection,"keep-alive");

    print(req);
    auto ex = co_await net::this_coro::executor;
    session_interface<PROTOCOL::HTTPS> ses{11, host, "443", ex};
    co_await ses.run();
    auto res = co_await ses.redirect(req, target);
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
    res = co_await redirect(req, "/");
    auto status = res.result();
    if(status != http::status::ok)
    {
      throw std::runtime_error{"\nurl image wasn't found\n"};
    }
    
    json::string url = "/images/search?";
    url += parse_answer(std::move(res));

    req = make_header(http::verb::get, host_, url);
    res = co_await redirect(req, "/", http::status::found);
    status = res.result();
    if(status != http::status::ok)
    {
      throw std::runtime_error{"\nhtml answer wasn't found\n"};
    }

    auto refs = parse_html(res.body());
    refs = filter_by_extension(std::move(refs), "jpg");
    size_t img_counter = 0;
    for(auto&i:refs)
    {
      try
      {
        res = co_await make_oneshot_session(i);
        auto it = res.find(http::field::content_type);
        if(it == res.end() || it->value() != "image/jpeg")
          continue;

        auto& body = res.body();
        json::string filename{"/home/zon/Downloads/"};
        filename += std::to_string(img_counter++);
        filename += ".jpg";
        store_file(filename, body.data(), body.size());
      }
      catch(std::exception const & ex)
      {
        print("\n\nOneShot session exception:\n",ex.what(),"\n\n");
      }
    }
  }


  [[nodiscard]]
  json::string parse_answer(http::response<http::string_body> res)
  {
    print("\n\nStart parsing answer...\n\n");
    boost::iostreams::array_source src{res.body().data(), res.body().size()};
    boost::iostreams::filtering_istream is;
    boost::iostreams::gzip_decompressor gz{};

    is.push(gz);
    is.push(src);

    std::string encode_js{};
    encode_js.reserve(res.body().size());

    boost::iostreams::back_insert_device ins {encode_js};
    boost::iostreams::copy(is, ins);

    print(encode_js);
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

  json::string default_encoding_{"gzip, deflate, br"};
  static inline json::string host_{"yandex.ru"};
};



int main()
{
  try
  {
    net::io_context ioc;
    net::co_spawn(ioc, session_interface<PROTOCOL::HTTP>::make_session<Searcher>(11, Searcher::host_, "80"), net::detached); 
    ioc.run();
  }
  catch(const std::exception& ex)
  {
    print("\n\n", ex.what(), "\n\n");
  }

  return 0;
}
