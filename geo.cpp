#define _USE_MATH_DEFINES
#include "geo.h"

#include <cmath>

namespace geo
{
    double ComputeDistance(Coordinates from, Coordinates to)
    {
        using namespace std;
        static const double dr = 3.1415926535 / 180.;
        static const int radius_of_earth = 6371000;
        return acos(sin(from.lat * dr) * sin(to.lat * dr)
            + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr))
            * radius_of_earth;
    }
    bool Coordinates::operator==(const Coordinates& rhs)
    {
        return this->lat == rhs.lat && this->lng == rhs.lng;
    }

    bool operator==(const Coordinates& lhs, const Coordinates& rhs)

    {
        return lhs.lat == rhs.lat && lhs.lng == rhs.lng;
    }
}



