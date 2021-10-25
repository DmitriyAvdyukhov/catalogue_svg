#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>
#include <sstream>

namespace json 
{
    enum NodeVariant
    {
        IS_NULL,
        IS_ARREY,
        IS_MAP,
        IS_BOOL,
        IS_INT,
        IS_DOUBLE,
        IS_STRING
    };

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;
    using Variant = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
    using Number = std::variant<int, double>;

    // Эта ошибка должна выбрасываться при ошибках парсинга JSON
    class ParsingError : public std::runtime_error
    {
    public:
        using runtime_error::runtime_error;
    };

    class Node
    {
    public:
        Node() = default;

        template<typename T>
        Node(T value) : value_(std::move(value)) {}

        const Array& AsArray() const;
        const Dict& AsMap() const;
        int AsInt() const;
        const std::string& AsString() const;
        double AsDouble()const;
        bool AsBool()const;

        bool IsNull()const;
        bool IsInt()const;
        bool IsDouble()const;
        bool IsPureDouble()const;
        bool IsString()const;
        bool IsBool()const;
        bool IsArray()const;
        bool IsMap()const;

        const Variant  GetNode()const;

        bool operator==(const Node& rhs)const;
        bool operator!=(const Node& rhs)const;

    private:
        Variant value_;
    };


    class Document
    {
    public:

        Document();

        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& other);

        bool operator!=(const Document& other);

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    void PrintString(const std::string& value, std::ostream& out);

}  // namespace json


