#pragma once


namespace geo
{
    struct Coordinates
    {
        double lat;
        double lng;
        bool operator==(const Coordinates& rhs);

    };

    bool operator==(const Coordinates& lhs, const Coordinates& rhs);

    double ComputeDistance(Coordinates from, Coordinates to);
}

