#include "json.h"

using namespace std;

namespace json
{
    //--------------LoadNode-------------

    Node LoadNode(istream& input);
    Node LoadString(istream& input);

    Node LoadNull(istream& input)
    {
        input.unget();
        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        std::string var;
        while (true)
        {
            if (it == end)
            {
                break;
            }

            const char ch = *it;

            if (ch == '\n' || ch == ',')
            {
                ++it;
                break;
            }
            else
            {
                var.push_back(ch);
            }
            ++it;
        }
        if (var == "null"s)
        {
            return Node{ nullptr };
        }
        else
        {
            std::string file = __FILE__;
            std::string line = std::to_string(__LINE__);
            std::string error = "Null parsing error in file: "s + file + " in line: "s + line;
            throw ParsingError(error);
        }
    }

    Node LoadArray(istream& input)
    {
        Array result;
        if (char ch; input >> ch)
        {
            input.unget();
            for (char c; input >> c && c != ']';)
            {
                if (c != ',')
                {
                    input.putback(c);
                }
                result.push_back(LoadNode(input));
            }
        }
        else
        {
            std::string file = __FILE__;
            std::string line = std::to_string(__LINE__);
            std::string error = "Array parsing error in file: "s + file + " in line: "s + line;
            throw ParsingError(error);
        }
        return Node(move(result));
    }

    Node LoadDict(istream& input)
    {
        Dict result;
        if (char ch; input >> ch)
        {
            input.unget();
            for (char c; input >> c && c != '}';)
            {
                if (c == ',')
                {
                    input >> c;
                }

                string key = LoadString(input).AsString();
                input >> c;
                result.insert({ move(key), LoadNode(input) });
            }
        }
        else
        {
            std::string file = __FILE__;
            std::string line = std::to_string(__LINE__);
            std::string error = "Dist parsing error in file: "s + file + " in line: "s + line;
            throw ParsingError(error);
        }
        return Node(move(result));
    }

    Node LoadBool(istream& input)
    {
        std::string line;
        getline(input, line, 'e');
        line.insert(0, "t");
        if (line.find("tru") != std::string::npos)
        {
            return Node(true);
        }
        else if (line.find("tals") != std::string::npos)
        {
            return Node(false);
        }
        else
        {
            std::string file = __FILE__;
            std::string line = std::to_string(__LINE__);
            std::string error = "Bool parsing error in file: "s + file + " in line: "s + line;
            throw ParsingError(error);
        }
    }

    Number LoadNumber(std::istream& input)
    {
        using namespace std::literals;

        std::string parsed_num;

        // Считывает в parsed_num очередной символ из input
        auto read_char = [&parsed_num, &input]
        {
            parsed_num += static_cast<char>(input.get());
            if (!input)
            {
                std::string file = __FILE__;
                std::string line = std::to_string(__LINE__);
                std::string error = "Failed to read number from stream in file: "s + file + " in line: "s + line;
                throw ParsingError(error);
            }
        };

        // Считывает одну или более цифр в parsed_num из input
        auto read_digits = [&input, read_char]
        {
            if (!std::isdigit(input.peek()))
            {
                std::string file = __FILE__;
                std::string line = std::to_string(__LINE__);
                std::string error = "A digit is expected in file: "s + file + " in line: "s + line;
                throw ParsingError(error);
            }
            while (std::isdigit(input.peek()))
            {
                read_char();
            }
        };

        if (input.peek() == '-')
        {
            read_char();
        }
        // Парсим целую часть числа
        if (input.peek() == '0')
        {
            read_char();
            // После 0 в JSON не могут идти другие цифры
        }
        else
        {
            read_digits();
        }

        bool is_int = true;
        // Парсим дробную часть числа
        if (input.peek() == '.')
        {
            read_char();
            read_digits();
            is_int = false;
        }

        // Парсим экспоненциальную часть числа
        if (int ch = input.peek(); ch == 'e' || ch == 'E')
        {
            read_char();
            if (ch = input.peek(); ch == '+' || ch == '-')
            {
                read_char();
            }
            read_digits();
            is_int = false;
        }

        try
        {
            if (is_int)
            {
                // Сначала пробуем преобразовать строку в int
                try
                {
                    return std::stoi(parsed_num);
                }
                catch (...)
                {
                    // В случае неудачи, например, при переполнении
                    // код ниже попробует преобразовать строку в double
                }
            }

            return std::stod(parsed_num);
        }
        catch (...)
        {
            std::string file = __FILE__;
            std::string line = std::to_string(__LINE__);
            std::string error = "Failed to convert "s + parsed_num + " to number in file: "s + file + " in line: "s + line;
            throw ParsingError(error);

        }
    }

    Node LoadString(std::istream& input)
    {
        auto it = std::istreambuf_iterator<char>(input);
        auto end = std::istreambuf_iterator<char>();
        std::string s;
        while (true)
        {
            if (it == end && s.back() != '"')
            {
                std::string file = __FILE__;
                std::string line = std::to_string(__LINE__);
                std::string error = "String parsing error in file: "s + file + " in line: "s + line;
                throw ParsingError(error);
            }
            if (it == end)
            {
                break;
            }
            const char ch = *it;
            if (ch == '"')
            {
                ++it;
                break;
            }
            else if (ch == '\\')
            {
                ++it;
                if (it == end)
                {
                    std::string file = __FILE__;
                    std::string line = std::to_string(__LINE__);
                    std::string error = "String parsing error in file: "s + file + " in line: "s + line;
                    throw ParsingError(error);
                }
                const char escaped_char = *it;
                switch (escaped_char)
                {
                case 'n':
                    s.push_back('\n');
                    break;
                case 't':
                    s.push_back('\t');
                    break;
                case 'r':
                    s.push_back('\r');
                    break;
                case '"':
                    s.push_back('"');
                    break;
                case '\\':
                    s.push_back('\\');
                    break;
                default:
                    std::string file = __FILE__;
                    std::string line = std::to_string(__LINE__);
                    std::string error = "Unrecognized escape sequence \\"s + escaped_char + " in file : "s + file + " in line : "s + line;
                    throw ParsingError(error);
                }
            }
            else if (ch == '\n' || ch == '\r')
            {
                std::string file = __FILE__;
                std::string line = std::to_string(__LINE__);
                std::string error = "Unexpected end of line in file: "s + file + " in line: "s + line;
                throw ParsingError(error);
            }
            else
            {
                s.push_back(ch);
            }
            ++it;
        }
        return Node(std::move(s));
    }

    Node LoadNode(istream& input)
    {
        char c;
        input >> c;
        if (c == '[')
        {
            return LoadArray(input);
        }
        else if (c == '{')
        {
            return LoadDict(input);
        }
        else if (c == '"')
        {
            return LoadString(input);
        }
        else if (c == 't' || c == 'f')
        {
            return  LoadBool(input);
        }
        else if (c == 'n')
        {
            return LoadNull(input);
        }
        else
        {
            input.unget();
            auto number = LoadNumber(input);
            if (number.index() == 0)
            {
                return Node(std::get<int>(number));
            }
            else
            {
                return Node(std::get<double>(number));
            }
        }
        std::string file = __FILE__;
        std::string line = std::to_string(__LINE__);
        std::string error = "Parsing error in file: "s + file + " in line: "s + line;
        throw ParsingError(error);
    }




    //--------Node As...------------------------------

    const Array& Node::AsArray() const
    {
        try
        {
            return std::get<Array>(value_);
        }
        catch (std::bad_variant_access&)
        {
            std::string file = __FILE__;
            std::string line = std::to_string(__LINE__);
            std::string error = "Logic error in file: "s + file + " in line: "s + line;
            throw std::logic_error(error);
        }
    }

    const Dict& Node::AsMap() const
    {
        try
        {
            return std::get<Dict>(value_);
        }
        catch (std::bad_variant_access&)
        {
            std::string file = __FILE__;
            std::string line = std::to_string(__LINE__);
            std::string error = "Logic error in file: "s + file + " in line: "s + line;
            throw std::logic_error(error);
        }
    }

    int Node::AsInt() const
    {
        try
        {
            return std::get<int>(value_);
        }
        catch (std::bad_variant_access&)
        {
            std::string file = __FILE__;
            std::string line = std::to_string(__LINE__);
            std::string error = "Logic error in file: "s + file + " in line: "s + line;
            throw std::logic_error(error);
        }
    }

    const std::string& Node::AsString() const
    {
        try
        {
            return std::get<std::string>(value_);
        }
        catch (std::bad_variant_access&)
        {
            std::string file = __FILE__;
            std::string line = std::to_string(__LINE__);
            std::string error = "Logic error in file: "s + file + " in line: "s + line;
            throw std::logic_error(error);
        }
    }

    double Node::AsDouble() const
    {
        try
        {
            if (IsPureDouble())
            {
                return std::get<double>(value_);
            }
            return AsInt() + 0.0;
        }
        catch (std::bad_variant_access&)
        {
            std::string file = __FILE__;
            std::string line = std::to_string(__LINE__);
            std::string error = "Logic error in file: "s + file + " in line: "s + line;
            throw std::logic_error(error);
        }
    }

    bool Node::AsBool() const
    {
        try
        {
            return std::get<bool>(value_);
        }
        catch (std::bad_variant_access&)
        {
            std::string file = __FILE__;
            std::string line = std::to_string(__LINE__);
            std::string error = "Logic error in file: "s + file + " in line: "s + line;
            throw std::logic_error(error);
        }
    }

    //---------------Node Is...----------------------

    bool Node::IsNull() const
    {
        if (value_.index() == 0)
        {
            return true;
        }
        return false;
    }

    bool Node::IsInt() const
    {
        if (value_.index() == 4)
        {
            return true;
        }
        return false;
    }

    bool Node::IsDouble() const
    {
        if (value_.index() == 5)
        {
            return true;
        }
        if (value_.index() == 4)
        {
            return true;
        }
        return false;
    }

    bool Node::IsPureDouble() const
    {
        if (value_.index() == 5)
        {
            return true;
        }
        return false;
    }

    bool Node::IsString() const
    {
        if (value_.index() == 6)
        {
            return true;
        }
        return false;
    }

    bool Node::IsBool() const
    {
        if (value_.index() == 3)
        {
            return true;
        }
        return false;
    }

    bool Node::IsArray() const
    {
        if (value_.index() == 1)
        {
            return true;
        }
        return false;
    }

    bool Node::IsMap() const
    {
        if (value_.index() == 2)
        {
            return true;
        }
        return false;
    }

    const Variant Node::GetNode() const
    {
        return value_;
    }

    bool Node::operator==(const Node& rhs) const
    {
        return this->GetNode() == rhs.GetNode();
    }

    bool Node::operator!=(const Node& rhs) const
    {
        return this->GetNode() != rhs.GetNode();
    }

    //--------------Dokument---------------------


    Document::Document() :root_(Node{})
    {
    }

    Document::Document(Node root)
        : root_(move(root)) {
    }

    const Node& Document::GetRoot() const
    {
        return root_;
    }

    bool Document::operator==(const Document& other)
    {
        return this->GetRoot() == other.GetRoot();
    }

    bool Document::operator!=(const Document& other)
    {
        return this->GetRoot() != other.GetRoot();
    }

    // -------------------Load----------------

    Document Load(istream& input)
    {
        return Document{ LoadNode(input) };
    }

    //--------------------Print-----------------------

    void PrintString(const std::string& value, std::ostream& out)
    {
        out << '"';
        for (const char c : value)
        {
            switch (c)
            {
            case '\r':
                out << "\\r"sv;
                break;
            case '\n':
                out << "\\n"sv;
                break;
            case '"':
                // Символы " и \ выводятся как \" или \\, соответственно
                [[fallthrough]]; // Break пропущен намеренно
            case '\\':
                out << '\\';
                [[fallthrough]]; // Break пропущен намеренно
            default:
                out << c;
                break;
            }
        }
        out << '"';
    }

   
    void Print(const Document& doc, std::ostream& output)
    {
        auto var = doc.GetRoot().GetNode().index();
        Node node = doc.GetRoot();
        if (var == NodeVariant::IS_NULL) 
        {
            output << "null"s;
        }
        else if (var == NodeVariant::IS_ARRAY)
        {
            bool is_first = true;
            static bool first = true;
            output << "[" << std::endl << "   ";

            for (const auto& arr : node.AsArray())
            {
                if (!is_first)
                {
                    output << "," << std::endl << "    ";
                }

                Print(Document(arr), output);

                is_first = false;
            }
            if (first)
            {
                output << std::endl << "    ]";
                first = false;
            }
            else
            {
                output << std::endl << "]";
            }

        }
        else if (var == NodeVariant::IS_MAP)  
        {
            bool is_first = true;

            output << "{" << std::endl;
            for (const auto& pair : node.AsMap())
            {
                if (!is_first)
                {
                    output << ",\n";
                }
                output << "        " << '"' << pair.first << "\": "s;

                Print(Document(pair.second), output);

                is_first = false;
            }
            output << "\n    }";
        }
        else if (var == NodeVariant::IS_BOOL) 
        {
            if (node.AsBool())
            {
                output << "true"s;
            }
            else
            {
                output << "false"s;
            }
        }
        else if (var == NodeVariant::IS_INT) 
        {
            output << std::to_string(node.AsInt());
        }
        else if (var == NodeVariant::IS_DOUBLE) 
        {
            ostringstream ost;
            ost << node.AsDouble();
            std::string result = ost.str();
            output << result;
        }
        else if (var == NodeVariant::IS_STRING)  
        {
            PrintString(doc.GetRoot().AsString(), output);
        }
    }

}  // namespace json