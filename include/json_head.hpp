#pragma once
#include <iostream>
#include <boost/json.hpp>
#include <boost/algorithm/string.hpp>
#include <optional>
#include <concepts>
#include <iterator>
#include <ranges>
#include "print.hpp"


namespace json = boost::json;


namespace Utils
{
    template<typename T, typename U>
    constexpr auto&& forward_like(U&& u) noexcept
    {
        constexpr bool is_const = std::is_const_v<std::remove_reference_t<T>>;
        if constexpr (std::is_lvalue_reference_v<T&&>)
        {
            if constexpr (is_const)
                return std::as_const(u);
            else
                return static_cast<U&>(u);
        }
        else
        {
            if constexpr (is_const)
                return std::move(std::as_const(u));
            else
                return std::move(u);
        }
    }
}

namespace Pars
{           
    using optarray  = std::optional<json::array>;
    using optobj    = std::optional<json::object>;
    using optstrw   = std::optional<json::string_view>;
    using optbool   = std::optional<bool>;
    using optstr    = std::optional<json::string>;
    using optint    = std::optional<int64_t>;
    using optuint   = std::optional<uint64_t>;
    using optdouble = std::optional<double>; 
    using op        = std::pair<json::string,std::optional<json::value>>;
    using p         = std::pair<json::string,json::value>;


    #define FIELD_NAME(field)  #field

    #define FIELD_TO_LOWER(field) boost::algorithm::to_lower_copy(json::string{FIELD_NAME(field)})

    #define FIELD_EQUAL(field) FIELD_NAME(field) "="

    #define JS_POINTER(method, field) boost::algorithm::to_lower_copy(json::string{"/"#method"/"#field})

    #define MAKE_PAIR(name, field) std::make_pair(FIELD_NAME(name), std::ref(field))

    #define MAKE_OP(name, field)  op{FIELD_NAME(name), field}

    #define PAIR(name, field)     p{FIELD_NAME(name), field}

    #define URL_USER_INFO(field, value) "@"#field":" #value

    #define URL_FIELD(field, value)     #field"=" #value

    #define URL_REQUEST(field) "/"#field"?"

 
    template<typename T>
    concept as_pointer = 
    (
        std::is_pointer_v<std::remove_cvref_t<T>> ||
        std::is_pointer_v<std::remove_cvref_t<decltype(std::declval<T>().get())>>
    );

    template<typename T>
    concept as_json_value = std::is_same_v<json::value, std::remove_reference_t<T>>;

    template<typename T>
    concept as_json_array = std::is_same_v<json::array, std::remove_reference_t<T>>;

    template<typename T>
    concept as_json_object  = std::is_same_v<json::object, std::remove_reference_t<T>>;

    template<typename T>
    concept as_json_string =std::is_same_v<json::string, std::remove_reference_t<T>>;

    template<typename T>
    concept as_json_view = requires 
    {
        requires std::is_convertible_v<std::remove_reference_t<T>, json::string_view>; 
    };

    template<typename T>
    concept as_json_kind = std::is_same_v<std::remove_reference_t<T>, json::kind>;


    template<typename T>
    concept is_obj = requires 
    {
        requires 
        (
            std::is_integral_v<T> 
            ||
            as_json_view<T>
        );
    };


    template<typename...Types>
    concept is_all_json_entities = 
    (
        (as_json_value<Types>  && ...)
        ||
        (as_json_object<Types> && ...)
        ||
        (as_json_string<Types> && ...)
        ||
        (as_json_array<Types>  && ...)
    );


    template<typename T>
    concept is_obj_pair = requires(T&& t) 
    {
        requires
        (
            std::is_same_v<std::remove_reference_t<decltype(t.first)>,json::string>
            &&
            std::is_constructible_v<json::value,std::remove_reference_t<decltype(t.second)>>
        );
    };


    template<typename T>
    concept  is_optional_pair = requires(T&& t)
    {
        requires
        (
            std::is_same_v<std::remove_reference_t<decltype(t.first)>, json::string>
            &&
            std::is_convertible_v<std::remove_reference_t<decltype(t.second.value())>, json::value>
        );
    };


    template<typename T>
    concept as_string_opt  = requires (T&& arg)
    {
        requires std::is_same_v<std::optional<std::remove_reference_t<decltype(arg.value())>>, std::remove_reference_t<T>>;
        std::to_string(arg.value());
    };


    template<typename T>
    concept is_iterator = requires()
    {
        requires std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<T>::iterator_category>;
    };


    template<typename T>
    concept is_back_inserter = requires (T&& t)
    {
        std::back_inserter(t);
    };


    template<typename T>
    concept is_opt = requires(T&& t)
    {
        t.has_value();
        t.value();
    };


    template<typename T>
    concept is_wrapper = requires(T&& t)
    {
        requires std::is_same_v<std::reference_wrapper<typename T::type>, std::remove_reference_t<T>>;
    };


    using fields_map = std::unordered_map<json::string, json::value>;
    using opt_fields_map = std::optional<fields_map>;

    template<typename T>
    concept is_fields_map = std::is_same_v<std::remove_reference_t<T>, fields_map>;

    template<typename T>
    concept is_opt_fields_map = std::is_same_v<std::remove_reference_t<T>, opt_fields_map>;


    struct MainParser
    {

        protected:

        static inline unsigned char buf_[8096]{};
        static inline json::monotonic_resource res_{buf_};
        static inline json::storage_ptr ptr_{&res_};
        static inline json::stream_parser pars_{ptr_};
        static inline json::serializer ser_{ptr_};


        protected:

        static void
        find_and_print_type
        (json::string_view name, const json::value& type)
        {
            if(type.is_array())
                std::cout<<name<<" :is_array\n";
            if(type.is_bool())
                std::cout<<name<<" :is_bool\n";
            if(type.is_double())
                std::cout<<name<<" :is_double\n";
            if(type.is_int64())
                std::cout<<name<<" :is_int64\n";
            if(type.is_null())
                std::cout<<name<<" :is_null\n";
            if(type.is_object())
                std::cout<<name<<" :is_object\n";
            if(type.is_string())
                std::cout<<name<<" :is_string\n";
            if(type.is_uint64())
                std::cout<<name<<" :is_uint64\n";
        }

        public:

        [[nodiscard]]
        static 
        json::storage_ptr& 
        get_storage_ptr()
        {
            return ptr_;
        }

        [[nodiscard]]
        static json::string
        make_json_pointer
        (json::string_view method, json::string_view field)
        {
            json::string str;
            str.reserve(method.size() + field.size() + 8);
            str.append("/");
            str.append(method);
            str.append("/");
            str.append(field);
            return str;
        }


        template<typename...T>
        requires (std::is_convertible_v<T, json::string>&&...&& true)
        [[nodiscard]]
        static json::string
        concat_string(const unsigned char del, T&&...str)
        {
            json::string temp;

            auto handle = [del, &temp]<typename U>(U&& str)
            {
                temp += std::forward<U>(str);
                temp += del;
            };

            (handle(std::forward<T>(str)),...);
            return temp;
        }


        [[nodiscard]]
        static json::value
        try_parse_message(json::string_view vw)
        {
            try
            {
                pars_.reset();
                pars_.write(vw);
                if(pars_.done() == false)  
                    throw std::runtime_error{"json parse message error\n"};

                pars_.finish();
                json::value v = pars_.release();
                pars_.reset();
                return v;
            }
            catch(const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                pars_.reset();
                throw e;
            }
        }


        template<as_json_value T>
        [[nodiscard]]
        static json::value
        try_parse_value(T&& val)
        {
            return try_parse_message(std::forward<T>(val).as_string());
        }


        template<json::kind type, as_json_value T>
        [[nodiscard]]
        static auto
        value_to_type
        (T&&  val)
        {
            if constexpr( type == json::kind::bool_)
            {
                std::optional<bool> op;

                if (val.is_bool())
                {
                    op = std::forward<T>(val).as_bool();
                    return op;
                }

                return op;
            }
            else if constexpr( type == json::kind::double_)
            {
                std::optional<double> op;

                if(val.is_double())
                {
                    op = std::forward<T>(val).as_double();
                    return op;
                } 

                return op;
            }
            else if constexpr( type == json::kind::uint64)
            {
                std::optional<uint64_t> op;

                if(val.is_uint64())
                {
                    op = std::forward<T>(val).as_uint64();
                    return op;
                }

                return op;
            }
            else if constexpr( type == json::kind::int64)
            {
                std::optional<int64_t> op;

                if(val.is_int64())
                {
                    op = std::forward<T>(val).as_int64();
                    return op;
                }

                return op;
            }
            else if constexpr( type == json::kind::string)
            {
                std::optional<json::string> op;

                if(val.is_string())
                {
                    op = std::forward<T>(val).as_string();
                    return op;
                }

                return op;
            }
            else if constexpr( type == json::kind::object)
            {
                std::optional<json::object> op;

                if(val.is_object())
                {
                    op = std::forward<T>(val).as_object();
                    return op;
                }

                return op;
            }
            else if constexpr( type == json::kind::array)
            {
                std::optional<json::array> op;

                if(val.is_array())
                {
                    op = std::forward<T>(val).as_array();
                    return op;
                }

                return op;
            }
            else 
            {
                std::optional<std::nullptr_t> op{};

                if(val.is_null())
                {
                    op = nullptr;
                    return op;
                }

                return op;
            }

        }


        template<std::ranges::viewable_range T>
        [[nodiscard]]
        static json::array
        parse_range_to_jsonArray(T&& ran)
        {
            std::ranges::ref_view r(ran);

            if constexpr ( ! std::is_reference_v<T> && std::is_rvalue_reference_v<T>)
                return json::array{std::make_move_iterator(r.begin()), std::make_move_iterator(r.end()), ptr_};
            else
                return json::array{r.begin(), r.end(), ptr_};
        }


        template<is_back_inserter T>
        static void
        parse_jsonArray_to_container(T&& obj, const json::array& arr)
        {
            auto back = std::back_inserter(obj);
            for(auto&& i : arr)
            {
                *back = {json::serialize(i)};
            }
        }


        template<as_json_value T>
        [[nodiscard]]
        static std::optional<json::value>
        check_pointer_validation(T&& val, std::pair<json::string_view, json::kind> pair)
        {
            boost::system::error_code er;

            auto it = val.find_pointer(pair.first, er);
            if(er)
            {
                return std::nullopt;
            }

            std::optional<json::value> opt{};

            json::kind t = it->kind();
            switch(t)
            {
                case json::kind::array   : {(t == pair.second)  ? opt =  std::forward<T>(*it).as_array()   : opt = std::nullopt; break;}

                case json::kind::bool_   : {(t == pair.second)  ? opt =  std::forward<T>(*it).as_bool()    : opt = std::nullopt; break;}

                case json::kind::double_ : {(t == pair.second)  ? opt =  std::forward<T>(*it).as_double()  : opt = std::nullopt; break;}

                case json::kind::int64   : {(t == pair.second)  ? opt =  std::forward<T>(*it).as_int64()   : opt = std::nullopt; break;}

                case json::kind::null    : {(t == pair.second)  ? opt =  nullptr          : opt = std::nullopt; break;}

                case json::kind::object  : {(t == pair.second)  ? opt =  std::forward<T>(*it).as_object()  : opt = std::nullopt; break;}

                case json::kind::string  : {(t == pair.second)  ? opt =  std::forward<T>(*it).as_string()  : opt = std::nullopt; break;}

                case json::kind::uint64  : {(t == pair.second)  ? opt =  std::forward<T>(*it).as_uint64()  : opt = std::nullopt; break;}

                default:
                    break;
            }

            return opt;
        }


        template<typename T, typename U>
        requires requires(T&& f, U&& t)
        {
            f.begin(); f.end();
            t.begin(); t.end(); 
        }
        static void
        container_move(T&& from, U& to)
        {
            auto b = std::make_move_iterator(from.begin());
            auto e = std::make_move_iterator(from.end());
            to.insert(b,e);
        }


        template<is_fields_map MAP>
        [[nodiscard]]
        static  
        std::optional<json::value>
        field_from_map
        (
            MAP&& map, 
            json::string_view field
        )
        {
            auto it = map.find(field);
            if (it!=map.end())
                return it->second();
            else
                return std::nullopt;
        }


        template
        <
            json::kind Kind, 
            typename T, 
            is_fields_map MAP, 
            typename U = json::string_view
        >
        static bool 
        field_from_map
        (
            MAP&& map,
            std::pair<U, T>&& field
        )
        {

            auto field_for_field = []
            <typename F>(F& field, auto value) 
            {
                if constexpr (as_pointer<F>) 
                {
                    if (field == nullptr)
                    {
                        F ptr{new std::remove_cvref_t<decltype(*field)>()};
                        field = std::move(ptr);
                    }
                
                    *field = std::move(value);
                }
                else 
                {
                    field = std::move(value);
                }
            };


            auto it = map.find(field.first);
            if(it != map.end())
            {
                using ret = std::remove_reference_t<decltype(value_to_type<Kind>(it->second))>;
                ret op;

                if constexpr (std::is_lvalue_reference_v<MAP>)
                {
                    op = value_to_type<Kind>(it->second);
                    if ( ! op.has_value())
                    {
                        return false;
                    }
                }
                else
                {
                    auto move_it = std::make_move_iterator(it);
                    op = value_to_type<Kind>(std::move(move_it->second));
                    if ( ! op.has_value())
                    {
                        return false;
                    }
                }

                if constexpr (is_opt<T>)
                    field_for_field(field.second.value(), std::move(op.value()));
                else
                    field_for_field(field.second, std::move(op.value()));
                return true;
            }

            return false;
        }


        template<as_json_value VAL, as_json_view...Types>
        [[nodiscard]]
        static fields_map
        mapped_pointers_validation(VAL && val, std::pair<Types, json::kind>&&...pointers)
        {

            fields_map map;

            auto cleanUp_pointer = [](json::string_view vw) -> json::string
            {
                size_t pos = vw.find_last_of("/");
                if(pos != vw.npos)
                {
                    json::string_view temp{vw.data() + pos + 1, vw.data() + vw.size()};
                    json::string str;

                    for(size_t i = 0; i < temp.size(); i++)
                    {
                        if(temp[i] == '~')
                        {
                            if(temp[i + 1] == '0')
                            {
                                str.append("~");
                                ++i;
                                continue;
                            }

                            if(temp[i + 1] == '1')
                            {
                                str.append("/");
                                ++i;
                                continue;
                            }
                        }

                        str += temp[i];
                    }       

                    return str;
                }
                else
                {
                    return vw;
                }
            };


            auto adapter = [&map, &cleanUp_pointer](json::string_view pointer, std::optional<json::value>&& opt)
            {
                if (opt.has_value())
                {
                    map.insert_or_assign(cleanUp_pointer(pointer), std::move(opt.value()));
                }
            }; 

            ((adapter)(json::string_view{pointers.first}, 
                check_pointer_validation(std::forward<VAL>(val), std::make_pair(json::string_view{pointers.first}, pointers.second))),...);

            return map;
        }


        static void
        find_and_print_json_type
        (const json::value& val, json::string_view name = {})
        {
            find_and_print_type(name, val);
        }


        static void 
        find_and_print_json_type
        (const fields_map& map)
        {
            for(auto&& i : map)
                find_and_print_type(i.first, i.second);
        }


        [[nodiscard]]
        static json::string
        serialize_to_string(const json::value& val)
        {
            json::string str;
            ser_.reset(&val);

            while(! ser_.done())
            {
                char buf[4096]{};
                json::string_view vw = ser_.read(buf);
                str.append(vw);
            }

            return str;
        }

        public:


        template<is_all_json_entities...Types>
        [[nodiscard]]
        static json::value 
        parse_all_json_as_value(Types&&...args)
        {
            return {std::forward<Types>(args)...};
        }


        template<is_all_json_entities...Types>
        [[nodiscard]]
        static json::string
        parse_all_json_as_string(Types&&...args)
        {
            json::value val;
            val = parse_all_json_as_value(std::forward<Types>(args)...);
            return serialize_to_string(val);
        }


        public:

        [[nodiscard]]
        static json::value
        parse_string_as_value(std::string_view vw)
        {
            return json::string{vw};
        }


        template<typename T>
        requires (std::is_integral_v<T>)
        [[nodiscard]]
        static json::value
        parse_number_as_value(T number)
        {
            return number;
        }


        template<typename T,typename U>
        requires (std::is_convertible_v<T,json::string_view> && is_obj<U>)
        [[nodiscard]]
        static json::value
        parse_obj_as_value(T&& key,U&& obj)
        {
            size_t sz = sizeof(T) + sizeof(U);

            json::object obj_(sz, ptr_);

            obj_.emplace(key, obj);

            return obj_;
        }


        template<is_obj_pair...T>
        [[nodiscard]]
        static json::object
        parse_ObjPairs_as_obj(T&&...pairs)
        {
            return {{pairs.first, pairs.second}...};
        }


        template<is_all_json_entities...Types>
        [[nodiscard]]
        static json::array
        parse_all_json_as_array(Types&&...args)
        {
            return {std::forward<Types>(args)...};
        }


        template<typename It>
        requires (is_all_json_entities<typename It::value_type>)
        [[nodiscard]]
        static json::array
        parse_all_json_as_array(It b, It e)
        {
            json::array arr;

            for(;b!=e;b++)
            {
                arr.emplace_back(*b);
            }

            return arr;
        }


        template<is_all_json_entities...Types>
        [[nodiscard]]
        static bool
        parse_all_json_to_file(std::string_view filename, Types&&...args)
        {
            FILE* f = std::fopen(filename.data(),"w+");
            if(! f)
                return false;

            json::value val = parse_all_json_as_value(std::forward<Types>(args)...);
            
            ser_.reset(&val);

            while( ! ser_.done())
            {
                char buf[4096]{};
                json::string_view vw = ser_.read(buf);
                std::fwrite(buf, sizeof(char), vw.size(), f);
            }

            return !std::fclose(f);
        }


        [[nodiscard]]
        static json::value
        parse_json_from_file(std::string_view filename )
        {
            std::FILE* f = std::fopen(filename.data(),"r");
            if(f == nullptr)
                throw std::runtime_error{"file is not opened\n"};

            try
            {
                do
                {
                    char buf[4096]{};
                    auto const nread = std::fread(buf,sizeof(char),sizeof(buf),f);
                    pars_.write( buf, nread);
                }
                while( ! std::feof(f));

                pars_.finish();

                std::fclose(f);

                return pars_.release();
            }
            catch(const std::exception& e)
            {
                std::fclose(f);
                std::cerr << e.what() << '\n';
                throw e;
            }
        }


        template<is_optional_pair...Types>
        [[nodiscard]]
        static json::object
        parse_OptPairs_as_obj(Types&&...args)
        {
            json::object ob(ptr_);

            (((args.second.has_value()) ? ob.insert({{Utils::forward_like<Types>(args.first), Utils::forward_like<Types>(args.second.value())}}) : void()),...);

            return ob;
        }


        template<typename T>
        requires requires(T&& arg)
        {
            requires std::is_same_v<std::optional<std::remove_reference_t<decltype(arg.value())>>, std::remove_reference_t<T>>;
        }
        [[nodiscard]]
        static json::string
        parse_opt_as_string(T&& arg) 
        {   
            if constexpr (std::is_same_v<std::remove_reference_t<decltype(std::declval<T>().value())>, json::string>)
                return json::string{Utils::forward_like<T>(arg.value())};
            else
                return (arg.has_value()) ? json::string{std::to_string(arg.value())} : json::string{};
        }


        template<typename It>
        requires (std::is_same_v<typename It::value_type, json::object>)
        [[nodiscard]]
        static json::object
        merge_json_obj(It beg, It end) 
        {
            json::object obj{ptr_};

            for(;beg!=end;beg++)
            {
                obj.construct(beg,end);
            }

            return obj;
        }


        public:

        static void
        pretty_print( std::ostream& os, json::value const& jv, std::string* indent = nullptr )
        {
            std::string indent_;
            if(! indent)
                indent = &indent_;
            switch(jv.kind())
            {
            case json::kind::object:
            {
                os << "{\n";
                indent->append(4, ' ');
                auto const& obj = jv.get_object();
                if(! obj.empty())
                {
                    auto it = obj.begin();
                    for(;;)
                    {
                        os << *indent << json::serialize(it->key()) << " : ";
                        pretty_print(os, it->value(), indent);
                        if(++it == obj.end())
                            break;
                        os << ",\n";
                    }
                }
                os << "\n";
                indent->resize(indent->size() - 4);
                os << *indent << "}";
                break;
            }

            case json::kind::array:
            {
                os << "[\n";
                indent->append(4, ' ');
                auto const& arr = jv.get_array();
                if(! arr.empty())
                {
                    auto it = arr.begin();
                    for(;;)
                    {
                        os << *indent;
                        pretty_print( os, *it, indent);
                        if(++it == arr.end())
                            break;
                        os << ",\n";
                    }
                }
                os << "\n";
                indent->resize(indent->size() - 4);
                os << *indent << "]";
                break;
            }

            case json::kind::string:
            {
                os << json::serialize(jv.get_string());
                break;
            }

            case json::kind::uint64:
            case json::kind::int64:
            case json::kind::double_:
                os << jv;
                break;

            case json::kind::bool_:
                if(jv.get_bool())
                    os << "true";
                else
                    os << "false";
                break;

            case json::kind::null:
                os << "null";
                break;
            }

            if(indent->empty())
                os << "\n";
        }

    };

};// namespace Pars
