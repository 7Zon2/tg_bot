#pragma once
#include "boost/beast/http/string_body.hpp"
#include "boost/interprocess/file_mapping.hpp"
#include "head.hpp"
#include "session_interface.hpp"


template<PROTOCOL PR = PROTOCOL::HTTPS>
class Searcher : public session_interface<PR>
{
  protected:

  using session_base_t = session_interface<PR>;

  using session_base_t::req_res;
  using session_base_t::write_request;
  using session_base_t::read_response;
  using session_base_t::stream_;

  public:

  template<typename...Types>
  Searcher
  (
   int version,
   json::string host,
   json::string port, 
   net::any_io_executor executor,
   Types&&...args
  )
  noexcept :
  session_interface<PR>
  (
    version,
    host,
    port,
    executor,
    std::forward<Types>(args)...
  ){}

  public:

  net::awaitable<void>
  run() override
  {
    co_await session_interface<PR>::run();
  }

  
  template<typename F>
  net::awaitable<void>
  operator()
  (
    std::optional<json::string> data,
    std::optional<json::string> filename,
    F&& callback
  )
  {
    if(data)
    {
      co_await request_upload_image
      (
        std::move(data).value(), 
        std::forward<F>(callback)
      );
    }
    else if(filename)
    {
      co_await request_upload_image
      (
        boost::interprocess::file_mapping{}, 
        filename.value(), 
        std::forward<F>(callback)
      );
    }
  }

  protected:

  template<typename F>
  net::awaitable<void>
  request_upload_image
  (
    boost::interprocess::file_mapping map, 
    json::string_view filename, 
    F&& callback
  )
  {
    json::string data = load_file(filename); 
    co_await request_upload_image(std::move(data), std::forward<F>(callback));
  }


  template<typename F>
  net::awaitable<void>
  request_upload_image
  (json::string data, F&& callback)
  {

    auto req = session_base::make_header
      (
        http::verb::post,
        "yandex.ru",
        R"(/images/search/?rpt=imageview&format=json&request={"blocks":[{"block":"b-page_type_search-by-image__link"}]})"
      );


    session_base::prepare_multipart
    (
      req,
      R"(image/jpeg)",
      R"(encoded_image)",
      R"(kartinka)",
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
      (status != http::status::temporary_redirect) &&
      (status != http::status::ok)
    )
    {
      throw std::runtime_error{"\nImage searching request wasn't successful\n"};
    }

    if(status == http::status::ok)
    {
      co_await start_browsing(std::move(res), std::forward<F>(callback));
    }
    else if(status == http::status::temporary_redirect)
    {
      co_await start_redirection(std::move(req), std::forward<F>(callback));
    }
  }


  [[nodiscard]]
  net::awaitable<http::response<http::string_body>>
  make_oneshot_session(json::string_view url)
  {
    print("\nStart OneShot session\n");
    json::string host = make_host(url);
    json::string target = make_relative_url(url);
    auto req = session_base::make_header(http::verb::get, host, target); 
    req.set(http::field::accept_encoding, default_encoding_);
    req.set(http::field::accept, "*/*");
    req.set(http::field::connection,"keep-alive");
    
    print_response(req);

    auto ex = co_await net::this_coro::executor;
    session_interface<PROTOCOL::HTTPS> ses{11, host, "443", ex};
    co_await ses.run();

    auto res = co_await ses.redirect(req, target);
    co_return std::move(res);
  }


  template<typename F>
  net::awaitable<void>
  start_browsing
  (
    http::response<http::string_body> res,
    F&& callback
  )
  {
    print("\nSTART BROWSING...\n");

    json::string url = "/images/search?";
    url += parse_answer(std::move(res));

    auto req = session_base::make_header(http::verb::get, host_, url);
    res = co_await session_base::redirect(req, "/", http::status::found);
    auto status = res.result();
    if(status != http::status::ok)
    {
      throw std::runtime_error{"\nhtml answer wasn't found\n"};
    }

    auto refs = parse_html(res.body());
    refs = filter_by_extension(std::move(refs), "jpg");
    for(auto&&i:refs)
    {
      try
      {
        res = co_await make_oneshot_session(i);
        auto it = res.find(http::field::content_type);
        if(it == res.end() || it->value() != "image/jpeg")
          continue;

        co_await callback(std::move(res));
      }
      catch(std::exception const & ex)
      {
        print("\n\nOneShot session exception:\n",ex.what(),"\n\n");
      }
    }
  }

  template<typename F>
  net::awaitable<void> 
  start_redirection
  (
   http::request<http::string_body> req,
   F&& callback
  )
  {
    print("\nSTART REDIRECTION...\n");
    auto res = co_await session_base::redirect(req, "/");
    auto status = res.result();
    if(status != http::status::ok)
    {
      throw std::runtime_error{"\nredirection for finding an image wasn't successful\n"};
    }
   
   co_await start_browsing(std::move(res), std::forward<F>(callback)); 
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

    json::value var = json::string{std::move(encode_js)};
    var = Pars::MainParser::try_parse_value(std::move(var));
    Pars::MainParser::pretty_print(std::cout, var);

    boost::system::error_code er;
    auto * it = var.find_pointer("/blocks/0/params/url", er);
    if(er)
    {
      throw std::runtime_error{"\npointer wasn't found/n"};
    }

    json::string url = it->as_string();
    return url; 
  }

  public:

  json::string default_encoding_{"gzip, deflate, br"};
  static inline json::string host_{"yandex.ru"};
};
