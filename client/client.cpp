#include "head.hpp"
#include "certif.hpp"
#include "tg_pars.hpp"
#include "print.hpp"

// Performs an HTTP GET and prints the response
class session : public std::enable_shared_from_this<session>
{
    const std::string host_;
    const int version_;
    std::string target_;
    tcp::resolver resolver_;
    const std::string port_;

    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_; // (Must persist between reads)
    http::request<http::string_body> req_;
    http::response<http::string_body> res_;

 private:

    enum class Events : char
    {
        GET,
        POST,
        SHUTDOWN
    };

    Events event_ = Events::POST;

 public:


    explicit
    session
    (
        std::string_view host,
        const int version,
        std::string_view port,
        net::any_io_executor ex,
        ssl::context& ctx
    )
    : 
      host_(host)
    , version_(version)
    , resolver_(ex) 
    , port_(port)
    , stream_(ex, ctx)
    {
        print
        (
            "host: ", host, "\n"
            "port: ", port, "\n"
            "version: ", version, "\n"
        );
    }


    public:

    void SwitchEvent()
    {
        if(event_ == Events::GET)
        {
            event_ = Events::POST;
        }
        else if(event_ == Events::POST)
        {
            event_ = Events::GET;
        }

    }


    void PrepareRequest()
    {
        auto prepare_post = [&]()
        {
            auto val = Pars::TG::TelegramRequestes::get_user_request(10,true,"Raven");
            std::string str = Pars::MainParser::parse_all_json_as_string(std::move(val));
            PostRequest(str,target_);
        };


        auto prepare_get = [&]()
        {
            GetRequest(target_);
        };

        auto prepare_shutdown = [&]()
        {
            // Gracefully close the stream
            stream_.async_shutdown
            (
                beast::bind_front_handler
                (
                    &session::on_shutdown,
                    shared_from_this()
                )
            );
        };


        switch(event_)
        {
            case Events::GET  : prepare_post(); return;

            case Events::POST : prepare_get(); return;

            default: 
                prepare_shutdown(); return;
        }

    }


    void PostRequest(std::string_view data, std::string_view target)
    {
        req_.method(http::verb::post);
        req_.set(http::field::host, host_);
        req_.set(http::field::content_type,"application/json");
        req_.set(http::field::user_agent, "lalala");
        req_.target(target);
        req_.set(http::field::content_length, boost::lexical_cast<std::string>(data.size()));
        req_.set(http::field::body, data);
        req_.body() = data;
        req_.prepare_payload();
    }


    void GetRequest
    (std::string_view target)
    {
        req_.method(http::verb::get);
        req_.set(http::field::host, host_);
        req_.set(http::field::user_agent, "lalala");
        req_.target(target);
    }


    void set_target
    (std::string_view target)
    {
        target_ = target;
    }


    // Start the asynchronous operation
    void
    run()
    {
        // Set SNI Hostname (many hosts need this to handshake successfully)
        if(! SSL_set_tlsext_host_name(stream_.native_handle(), host_.data()))
        {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            std::cerr << ec.message() << "\n";
            return;
        }


        // Look up the domain name
        resolver_.async_resolve
        (
            host_.data(),
            port_.data(),
            beast::bind_front_handler
            (
                &session::on_resolve,
                shared_from_this()
            )
        );
    }


    void
    on_resolve(
        beast::error_code ec,
        tcp::resolver::results_type results)
    {
        if(ec)
            return fail(ec, "resolve");

        print("resolving...\n");

        // Set a timeout on the operation
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Make the connection on the IP address we get from a lookup
        beast::get_lowest_layer(stream_).async_connect
        (
            results,
            beast::bind_front_handler
            (
                &session::on_connect,
                shared_from_this()
            )
        );
    }


    void
    on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
    {
        if(ec)
            return fail(ec, "connect");

        print("Connecting...\n");

        // Perform the SSL handshake
        stream_.async_handshake
        (
            ssl::stream_base::client,
            beast::bind_front_handler
            (
                &session::on_handshake,
                shared_from_this()
            )
        );
    }


    void
    on_handshake(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "handshake");

        // Set a timeout on the operation
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        print("Handshake...\n");


        PrepareRequest();


        // Send the HTTP request to the remote host
        http::async_write
        (
            stream_, 
            req_,
            beast::bind_front_handler
            (
                &session::on_write,
                shared_from_this()
            )
        );
    }


    void
    on_write
    (
        beast::error_code ec,
        std::size_t bytes_transferred
    )
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        print("writing...\n");

        // Receive the HTTP response
        http::async_read
        (
            stream_, 
            buffer_, 
            res_,
            beast::bind_front_handler
            (
                &session::on_read,
                shared_from_this()
            )
        );
    }


    void
    on_read
    (
        beast::error_code ec,
        std::size_t bytes_transferred
    )
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "read");

        print("reading...\n");

        // Write the message to standard out
        std::cout << res_ << std::endl;

        // Set a timeout on the operation
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        SwitchEvent();

        PrepareRequest();

        http::async_write
        (
            stream_,
            req_,
            beast::bind_front_handler
            (
                &session::on_write,
                shared_from_this()
            )
        );
    }


    void
    on_shutdown(beast::error_code ec)
    {
        print("Shutdown...\n",ec.what());

        if(ec == net::error::eof)
        {
            // Rationale:
            // http://stackoverflow.com/questions/25587403/boost-asio-ssl-async-shutdown-always-finishes-with-an-error
            ec = {};
        }
        if(ec)
            return fail(ec, "shutdown");

        // If we get here then the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

int main(int argc, char** argv)
{
  
    if(argc <= 1)
    {
        std::cout<<"Please, input your bot-telegram token\n";
        return 1;
    }

    std::string host   = "localhost";
    std::string port   = "8080";
    int version = 11;

    try
    {
         
        // The io_context is required for all I/O
        net::io_context ioc;

        // The SSL context is required, and holds certificates
        ssl::context ctx{ssl::context::tlsv13_client};

        // This holds the root certificate used for verification
        CRTF::load_cert(ctx,"/home/zon/keys/certif/serv/host.crt");

        ctx.set_default_verify_paths();

        // Verify the remote server's certificate
        ctx.set_verify_mode(ssl::verify_peer);

        // Launch the asynchronous operation
        // The session is constructed with a strand to
        // ensure that handlers do not execute concurrently.
        std::make_shared<session>
        (
            host,
            version,
            port,
            net::make_strand(ioc),
            ctx

        )->run();

        // Run the I/O service. The call will return when
        // the get operation is complete.
        ioc.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }
  

    return EXIT_SUCCESS;
}
