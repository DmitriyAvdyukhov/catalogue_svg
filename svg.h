#pragma once

#include <optional>
#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <variant>

using namespace std::literals;

namespace svg
{
    struct Rgb
    {
        Rgb() : red(0), green(0), blue(0) {}
        Rgb(int r, int g, int b) : red(r), green(g), blue(b) {}

        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    template <class Ostream>
    Ostream& operator<<(Ostream& out, const Rgb& rgb)
    {
        out << "rgb(" << std::to_string(rgb.red) << ','
            << std::to_string(rgb.green) << ','
            << std::to_string(rgb.blue) << ')';
        return out;
    }

    struct Rgba
    {
        Rgba() : red(0), green(0), blue(0), opacity(1) { }
        Rgba(int r, int g, int b, double o) : red(r), green(g), blue(b), opacity(o) {}

        uint8_t red;
        uint8_t green;
        uint8_t blue;
        double opacity;
    };

    template <class Ostream>
    Ostream& operator<<(Ostream& out, const Rgba& rgba)
    {
        out << "rgba(" << std::to_string(rgba.red) << ','
            << std::to_string(rgba.green) << ','
            << std::to_string(rgba.blue) << ','
            << rgba.opacity << ')';
        return out;
    }

    struct Visiter
    {
        std::ostream& out;

        void operator()(std::monostate)const
        {
            out << "none"s;
        }

        void operator()(Rgb rgb)const
        {
            out << rgb;
        }
        void operator()(Rgba rgba)const
        {
            out << rgba;
        }
        void operator()(std::string color)const
        {
            if (color.length())
            {
                out << color;
            }
            else
            {
                out << "none"s;
            }
        }
    };

    using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;

    inline const Color NoneColor{ "none"s };

    enum class StrokeLineCap
    {
        BUTT,
        ROUND,
        SQUARE,
    };

    template <class Ostream>
    Ostream& operator<<(Ostream& out, const StrokeLineCap& line_cap)
    {
        if (line_cap == StrokeLineCap::BUTT)
        {
            out << "butt"sv;;
            return out;
        }
        if (line_cap == StrokeLineCap::ROUND)
        {
            out << "round"sv;;
            return out;
        }
        if (line_cap == StrokeLineCap::SQUARE)
        {
            out << "square"sv;;
            return out;
        }
        return out;
    }

    enum class StrokeLineJoin
    {
        ARCS,
        BEVEL,
        MITER,
        MITER_CLIP,
        ROUND,

    };

    template <class Ostream>
    Ostream& operator<<(Ostream& out, const StrokeLineJoin& line_join)
    {
        if (line_join == StrokeLineJoin::ARCS)
        {
            out << "arcs"sv;
            return out;
        }
        if (line_join == StrokeLineJoin::BEVEL)
        {
            out << "bevel"sv;
            return out;
        }
        if (line_join == StrokeLineJoin::MITER)
        {
            out << "miter"sv;
            return out;
        }
        if (line_join == StrokeLineJoin::MITER_CLIP)
        {
            out << "miter-clip"sv;
            return out;
        }
        if (line_join == StrokeLineJoin::ROUND)
        {
            out << "round"sv;
            return out;
        }
        return out;
    }

    struct Point
    {
        Point() = default;
        Point(double x, double y);
        double x = 0;
        double y = 0;
    };

    /*
     * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
     * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
     */
    struct RenderContext
    {
        RenderContext(std::ostream& out);

        RenderContext(std::ostream& out, int indent_step, int indent = 0);

        RenderContext Indented() const;

        void RenderIndent() const;

        std::ostream& out;
        int indent_step = 0;
        int indent = 0;
    };

    template <typename Owner>
    class PathProps
    {
    public:
        PathProps() = default;

        explicit PathProps(std::string fill_color, std::string stroke_color)
            : fill_color_(fill_color), stroke_color_(stroke_color) {}


        Owner& SetFillColor(Color color)
        {
            fill_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeColor(Color color)
        {
            stroke_color_ = std::move(color);
            return AsOwner();
        }

        Owner& SetStrokeWidth(double width)
        {
            width_ = width;
            return AsOwner();
        }

        Owner& SetStrokeLineCap(StrokeLineCap line_cap)
        {
            line_cap_ = std::move(line_cap);
            return AsOwner();
        }

        Owner& SetStrokeLineJoin(StrokeLineJoin line_join)
        {
            line_join_ = std::move(line_join);
            return AsOwner();
        }


    protected:

        ~PathProps() = default;

        void RenderAttrs(const RenderContext& context)const
        {
            auto& out = context.out;

            if (!std::holds_alternative<std::monostate>(fill_color_))
            {
                out << " fill=\""sv;
                std::visit(Visiter{ out }, fill_color_);
                out << "\""sv;
            }

            if (!std::holds_alternative<std::monostate>(stroke_color_))
            {
                out << " stroke=\""sv;
                std::visit(Visiter{ out }, stroke_color_);
                out << "\""sv;
            }

            if (width_)
            {
                out << " stroke-width=\""sv << width_ << "\""sv;
            }

            if (line_cap_ != std::nullopt)
            {
                out << " stroke-linecap=\""sv << *line_cap_ << "\""sv;
            }

            if (line_join_ != std::nullopt)
            {
                out << " stroke-linejoin=\""sv << *line_join_ << "\""sv;
            }
        }

        Color fill_color_;
        Color stroke_color_;
        double width_ = 0;
        std::optional<StrokeLineCap> line_cap_;
        std::optional<StrokeLineJoin> line_join_;

    private:
        Owner& AsOwner()
        {
            // static_cast безопасно преобразует *this к Owner&,
            // если класс Owner — наследник PathProps
            return static_cast<Owner&>(*this);
        }

    };

    /*
     * Абстрактный базовый класс Object служит для унифицированного хранения
     * конкретных тегов SVG-документа
     * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
     */
    class Object
    {
    public:
        void Render(const RenderContext& context) const;
        virtual ~Object() = default;

    private:
        virtual void RenderObject(const RenderContext& context) const = 0;
    };

    /*
     * Класс Circle моделирует элемент <circle> для отображения круга
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
     */
    class Circle final : public Object, public PathProps<Circle>
    {
    public:

        Circle& SetCenter(Point center);

        Circle& SetRadius(double radius);


    private:
        void RenderObject(const RenderContext& context) const override;

        Point center_;
        double radius_ = 1.0;
    };


    /*
     * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
     */
    class Polyline final : public Object, public svg::PathProps<Polyline>
    {
    public:
        Polyline& AddPoint(Point point);

    private:
        void RenderObject(const RenderContext& context) const override;

        std::vector<Point> polyline_;
    };

    /*
     * Класс Text моделирует элемент <text> для отображения текста
     * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
     */
    class Text final : public Object, public svg::PathProps<Text>
    {
    public:
        explicit Text();

        Text& SetPosition(Point pos);

        Text& SetOffset(Point offset);

        Text& SetFontSize(uint32_t size);

        Text& SetFontFamily(std::string font_family);

        Text& SetFontWeight(std::string font_weight);

        Text& SetData(std::string data);

    private:
        void RenderObject(const RenderContext& context) const override;
        Point pos_;
        Point offset_;
        std::string font_family_;
        std::string data_;
        uint32_t font_size_;
        std::string font_weight_;
    };

    class ObjectContainer
    {
    public:
        virtual ~ObjectContainer() = default;

        template <class Obj>
        void Add(Obj obj);

        virtual void AddPtr(std::unique_ptr<Object>&& obj) = 0;

    protected:
        std::vector < std::unique_ptr<Object>>doc_;
    };

    class Drawable
    {
    public:
        virtual ~Drawable() = default;

        virtual void Draw(ObjectContainer& container)const = 0;
    };

    template <class Obj>
    void ObjectContainer::Add(Obj obj)
    {
        std::unique_ptr<Obj> obj_ = std::make_unique<Obj>(obj);
        doc_.emplace_back(std::move(obj_));
    }

    class Document : public ObjectContainer
    {
    public:

        void AddPtr(std::unique_ptr<Object>&& obj)override;

        void Render(std::ostream& out) const;
    };

}  // namespace svg
