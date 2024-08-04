#include "head.hpp"
#include "certif.hpp"
#include "print.hpp"

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session>
{
    beast::ssl_stream<beast::tcp_stream> stream_;
    beast::flat_buffer buffer_;
    std::shared_ptr<std::string const> doc_root_;
    http::request<http::string_body> req_;

 public:

    // Take ownership of the socket
    explicit
    session
    (
        tcp::socket&& socket,
        ssl::context& ctx,
        std::shared_ptr<std::string const> const& doc_root
    )
        : stream_(std::move(socket), ctx)
        , doc_root_(doc_root)
    {
        
    }


    // Start the asynchronous operation
    void
    run()
    {   
        // We need to be executing within a strand to perform async operations
        // on the I/O objects in this session. Although not strictly necessary
        // for single-threaded contexts, this example code is written to be
        // thread-safe by default.
        net::dispatch
        (
            stream_.get_executor(),
            beast::bind_front_handler
            (
                &session::on_run,
                shared_from_this()
            )
        );
    }


    void
    on_run()
    {
        // Set the timeout.
        beast::get_lowest_layer(stream_).expires_after(
            std::chrono::seconds(30));

        // Perform the SSL handshake
        stream_.async_handshake
        (
            ssl::stream_base::server,
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

        do_read();
    }


    void
    do_read()
    {
        // Make the request empty before reading,
        // otherwise the operation behavior is undefined.
        req_ = {};

        // Set the timeout.
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Read a request
        http::async_read(stream_, buffer_, req_,
            beast::bind_front_handler(
                &session::on_read,
                shared_from_this()));
    }


    void
    on_read
    (
        beast::error_code ec,
        std::size_t bytes_transferred
    )
    {
        boost::ignore_unused(bytes_transferred);

        // This means they closed the connection
        if(ec == http::error::end_of_stream)
            return do_close();

        if(ec)
            return fail(ec, "read");

        print("Beind read the received request...\n");
        std::cout<<req_<<std::endl;

        // Send the response
        send_response(
            handle_request(*doc_root_, std::move(req_)));
    }


    void
    send_response(http::message_generator&& msg)
    {
        beast::error_code er;

        bool keep_alive = msg.keep_alive();

        print("Being sent message...\n");

        // Write the response
        beast::async_write
        (
            stream_,
            std::move(msg),
            beast::bind_front_handler
            (
                &session::on_write,
                this->shared_from_this(),
                keep_alive
            )
        );
    }


    void
    on_write(
        bool keep_alive,
        beast::error_code ec,
        std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec)
            return fail(ec, "write");

        print("Writing...\n");

        if(! keep_alive)
        {
            // This means we should close the connection, usually because
            // the response indicated the "Connection: close" semantic.
            return do_close();
        }

        // Read another request
        do_read();
    }


    void
    do_close()
    {
        // Set the timeout.
        beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

        // Perform the SSL shutdown
        stream_.async_shutdown
        (
            beast::bind_front_handler
            (
                &session::on_shutdown,
                shared_from_this()
            )
        );
    }


    void
    on_shutdown(beast::error_code ec)
    {
        if(ec)
            return fail(ec, "shutdown");

        // At this point the connection is closed gracefully
    }
};

//------------------------------------------------------------------------------

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
    net::io_context& ioc_;
    ssl::context& ctx_;
    tcp::acceptor acceptor_;
    std::shared_ptr<std::string const> doc_root_;

 public:

    template<typename T>
    void print_endpoint(const T& endpoint)
    {
        print("endpoint:\n");
        print
        (
            "Address: ", endpoint.address().to_string(), "\n",
            "Port: "   , endpoint.port(),"\n",
            "Protocol: ", endpoint.protocol().protocol(), "\n",
            "Protocol-family: ", endpoint.protocol().family(), "\n",
            "Protocol-type: ", endpoint.protocol().type(), "\n"
        );
    }

 public:
 
    listener
    (
        net::io_context& ioc,
        ssl::context& ctx,
        tcp::endpoint endpoint,
        std::shared_ptr<std::string const> const& doc_root
    )
        : ioc_(ioc)
        , ctx_(ctx)
        , acceptor_(ioc)
        , doc_root_(doc_root)
    {
        print("Server:\n");
        print_endpoint(endpoint);

        beast::error_code ec;

        // Open the acceptor
        acceptor_.open(endpoint.protocol(), ec);
        if(ec)
        {
            fail(ec, "open");
            return;
        }

        // Allow address reuse
        acceptor_.set_option(net::socket_base::reuse_address(true), ec);
        if(ec)
        {
            fail(ec, "set_option");
            return;
        }
        
        // Bind to the server address
        acceptor_.bind(endpoint, ec);
        if(ec)
        {
            fail(ec, "bind");
            return;
        }

        // Start listening for connections
        acceptor_.listen(
            net::socket_base::max_listen_connections, ec);
        if(ec)
        {
            fail(ec, "listen");
            return;
        }

    }

    // Start accepting incoming connections
    void
    run()
    {
        do_accept();
    }

 private:


    void
    do_accept()
    {
        // The new connection gets its own strand
        acceptor_.async_accept(
            net::make_strand(ioc_),
            beast::bind_front_handler(
                &listener::on_accept,
                shared_from_this()));
    }


    void
    on_accept(beast::error_code ec, tcp::socket socket)
    {
        print("\nAccepted connection:\n");
        print_endpoint(socket.remote_endpoint());
        if(ec)
        {
            fail(ec, "accept");
            return; // To avoid infinite loop
        }
        else
        {
            // Create the session and run it
            std::make_shared<session>(
                std::move(socket),
                ctx_,
                doc_root_)->run();
        }

        // Accept another connection
        do_accept();
    }
};

//------------------------------------------------------------------------------

int main()
{ 
    try
    {
        const auto  address  = net::ip::make_address("127.0.0.1");
        const unsigned short port   = 8080;
        const auto doc_root   = std::make_shared<std::string>("/home/zon/vs_code/http_serv/http/res/");
        const int threads     = 4;

        // The io_context is required for all I/O
        net::io_context ioc{threads};

        // The SSL context is required, and holds certificates
        ssl::context ctx{ssl::context::tlsv13_server};

        CRTF::set_cert
        (
            ctx,
            "/home/zon/keys/certif/serv/host.key",
            "/home/zon/keys/certif/serv/host.crt",
            ""
        );

        // Create and launch a listening port
        std::make_shared<listener>(
            ioc,
            ctx,
            tcp::endpoint{address, port},
            doc_root)->run();

        // Run the I/O service on the requested number of threads
        std::vector<std::thread> v;
        v.reserve(threads - 1);
        for(auto i = threads - 1; i > 0; --i)
            v.emplace_back(
            [&ioc]
            {
                ioc.run();
            });
        ioc.run();

        return EXIT_SUCCESS;

    }
    catch(const std::exception& ex)
    {
        std::cerr<<ex.what()<<std::endl;
    }
    catch(...)
    {   
        std::cerr<<"Something got wrong\n";
    }
  
}