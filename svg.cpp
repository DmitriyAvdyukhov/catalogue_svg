#define _USE_MATH_DEFINES
#include "svg.h"

#include <cmath>

namespace svg {

    using namespace std::literals;

    void Object::Render(const RenderContext& context) const
    {
        context.RenderIndent();

        // Делегируем вывод тега своим подклассам
        RenderObject(context);
    }

    //---------------Point------------------

    Point::Point(double x, double y)
        : x(x)
        , y(y)
    {}

    // ---------- Circle -------------------------------------------------------

    Circle& Circle::SetCenter(Point center)
    {
        center_ = center;
        return *this;
    }

    Circle& Circle::SetRadius(double radius)
    {
        radius_ = radius;
        return *this;
    }

    void Circle::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << " <circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
        out << "r=\""sv << radius_ << "\""sv;

        PathProps<Circle>::RenderAttrs(context.out);

        out << "/>\n "sv;
    }

    //------------Text--------------------------------------------------------------------------

    Text::Text()
    {
        pos_ = { 0, 0 };
        offset_ = { 0, 0 };
        font_size_ = 1;
        data_ = {};
    }

    Text& Text::SetPosition(Point pos)
    {
        pos_ = pos;
        return *this;
    }

    Text& Text::SetOffset(Point offset)
    {
        offset_ = offset;
        return *this;
    }

    Text& Text::SetFontSize(uint32_t size)
    {
        font_size_ = size;
        return *this;
    }

    Text& Text::SetFontFamily(std::string font_family)
    {
        font_family_ = font_family;
        return *this;
    }

    Text& Text::SetFontWeight(std::string font_weight)
    {
        font_weight_ = font_weight;
        return *this;
    }

    Text& Text::SetData(std::string data)
    {
        data_ = data;
        return *this;
    }

    void Text::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;

        out << "<text";
        PathProps<Text>::RenderAttrs(context.out);
        out << " x=\""sv << pos_.x << "\" y=\""sv << pos_.y << "\" "sv;
        out << "dx=\""sv << offset_.x << "\" dy=\""sv << offset_.y << "\" "sv;
        out << "font-size=\""sv << font_size_ << "\""sv;
        if (font_family_.length())
        {
            out << " font-family=\""sv << font_family_ << "\""sv;
        }
        if (font_weight_.length())
        {
            out << " font-weight=\""sv << font_weight_ << "\""sv;
        }

        out << ">"sv << data_ << "<"sv;
        out << "/text>\n  "sv;
    }

    //-------------Polyline--------------------------------------------------------------------

    Polyline& Polyline::AddPoint(Point point)
    {
        polyline_.push_back(point);
        return *this;
    }

    void Polyline::RenderObject(const RenderContext& context) const
    {
        auto& out = context.out;
        out << "<polyline points=\""sv;
        for (size_t i = 0; i < polyline_.size(); ++i)
        {
            if (i != polyline_.size() - 1)
            {
                out << polyline_[i].x << "," << polyline_[i].y << " ";
            }
            else
            {
                out << polyline_[i].x << "," << polyline_[i].y;
            }
        }
        out << "\""sv;

        PathProps<Polyline>::RenderAttrs(context.out);

        out << "/>\n  "sv;
    }

    //-------------Document---------------------------------------------------------------------------------

    void Document::AddPtr(std::unique_ptr<Object>&& obj)
    {
        doc_.emplace_back(std::move(obj));
    }

    void Document::Render(std::ostream& out) const
    {
        out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
        out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  "sv;
        for (const auto& doc : doc_)
        {
            doc->Render(out);
        }
        out << "</svg>"sv;
    }

    //----------------------RenderContext-------------

    RenderContext::RenderContext(std::ostream& out)
        : out(out)
    {}

    RenderContext::RenderContext(std::ostream& out, int indent_step, int indent)
        : out(out)
        , indent_step(indent_step)
        , indent(indent)
    {}

    RenderContext RenderContext::Indented() const
    {
        return { out, indent_step, indent + indent_step };
    }

    void RenderContext::RenderIndent() const
    {
        for (int i = 0; i < indent; ++i)
        {
            out.put(' ');
        }
    }

}  // namespace svg