#pragma once
#include <iostream>
#include <boost/json.hpp>
#include <optional>
#include <concepts>
#include <iterator>
#include <ranges>
#include "print.hpp"


namespace json = boost::json;


namespace Pars
{   

    template<typename T>
    concept as_json_value = std::is_same_v<json::value, std::decay_t<T>>;

    template<typename T>
    concept as_json_array = std::is_same_v<json::array, std::decay_t<T>>;

    template<typename T>
    concept as_json_object  = std::is_same_v<json::object, std::decay_t<T>>;

    template<typename T>
    concept as_json_string =std::is_same_v<json::string, std::decay_t<T>>;

    template<typename T>
    concept as_json_view = requires 
    {
        requires std::is_convertible_v<std::decay_t<T>, json::string_view>; 
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
            std::is_same_v<decltype(t.first),json::string>
            &&
            std::is_constructible_v<json::value,decltype(t.second)>
        );
    };


    template<typename T>
    concept  is_optional_pair = requires(T&& t)
    {
        requires
        (
            std::is_same_v<decltype(t.first), json::string>
            &&
            std::is_convertible_v<decltype(t.second.value()), json::value>
        );
    };


    template<typename T>
    concept as_string_opt  = requires (T&& arg)
    {
        requires std::is_same_v<std::optional<std::decay_t<decltype(arg.value())>>, std::decay_t<T>>;
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


        [[nodiscard]]
        static json::value
        try_parse_value(const json::value& val)
        {
            return try_parse_message(val.as_string());
        }


        template<json::kind type>
        [[nodiscard]]
        static auto
        value_to_type
        (const json::value& val)
        {
            if constexpr( type == json::kind::bool_)
            {
                std::optional<bool> op;

                if (val.is_bool())
                {
                    op = val.as_bool();
                    return op;
                }

                return op;
            }
            else if constexpr( type == json::kind::double_)
            {
                std::optional<double> op;

                if(val.is_double())
                {
                    op = val.as_double();
                    return op;
                } 

                return op;
            }
            else if constexpr( type == json::kind::uint64)
            {
                std::optional<uint64_t> op;

                if(val.is_uint64())
                {
                    op = val.as_uint64();
                    return op;
                }

                return op;
            }
            else if constexpr( type == json::kind::int64)
            {
                std::optional<int64_t> op;

                if(val.is_int64())
                {
                    op = val.as_int64();
                    return op;
                }

                return op;
            }
            else if constexpr( type == json::kind::string)
            {
                std::optional<json::string> op;

                if(val.is_string())
                {
                    op = val.as_string();
                    return op;
                }

                return op;
            }
            else if constexpr( type == json::kind::object)
            {
                std::optional<json::object> op;

                if(val.is_object())
                {
                    op = val.as_object();
                    return op;
                }

                return op;
            }
            else if constexpr( type == json::kind::array)
            {
                std::optional<json::array> op;

                if(val.is_array())
                {
                    op = val.as_array();
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


        [[nodiscard]]
        static std::optional<json::value>
        check_pointer_validation(const json::value& val, std::pair<json::string_view, json::kind> pair)
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
                case json::kind::array   : {(t == pair.second)  ? opt =  it->as_array()   : opt = std::nullopt; break;}

                case json::kind::bool_   : {(t == pair.second)  ? opt =  it->as_bool()    : opt = std::nullopt; break;}

                case json::kind::double_ : {(t == pair.second)  ? opt =  it->as_double()  : opt = std::nullopt; break;}

                case json::kind::int64   : {(t == pair.second)  ? opt =  it->as_int64()   : opt = std::nullopt; break;}

                case json::kind::null    : {(t == pair.second)  ? opt =  nullptr          : opt = std::nullopt; break;}

                case json::kind::object  : {(t == pair.second)  ? opt =  it->as_object()  : opt = std::nullopt; break;}

                case json::kind::string  : {(t == pair.second)  ? opt =  it->as_string()  : opt = std::nullopt; break;}

                case json::kind::uint64  : {(t == pair.second)  ? opt =  it->as_uint64()  : opt = std::nullopt; break;}

                default:
                    break;
            }

            return opt;
        }

   

        template<json::kind Kind, typename T>
        static bool 
        field_from_map
        (
            const std::unordered_map<json::string, json::value>& map,
            std::pair<const char*, T>&& field
        )
        {
            auto it = map.find(field.first);
            if(it != map.end())
            {
                auto op = value_to_type<Kind>(it->second);
                if(op.has_value())
                {
                    field.second = std::move(op.value());
                    return true;
                }
            }

            return false;
        }


        template<as_json_view...Types>
        [[nodiscard]]
        static std::unordered_map<json::string, json::value>
        mapped_pointers_validation(const json::value& val, std::pair<Types, json::kind>&&...pointers)
        {

            std::unordered_map<json::string, json::value> map;

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
                check_pointer_validation(val, std::make_pair(json::string_view{pointers.first}, pointers.second))),...);

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
        (const std::unordered_map<json::string, json::value>& map)
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

            (((args.second.has_value()) ? ob.insert({{args.first, args.second.value()}}) : void()),...);

            return ob;
        }


        template<typename T>
        requires requires(T&& arg)
        {
            requires std::is_same_v<std::optional<std::remove_reference_t<decltype(arg.value())>>, std::remove_reference_t<T>>;
        }
        [[nodiscard]]
        static json::string
        parse_opt_as_string(const T& arg) 
        {   
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
