#pragma once

// Opaque profile handle
// Actual implementation lives inside kernel (OCCT, etc.)


enum class ProfileKind {
    Circle,
    Rect,
    Polygon,
    Spline
};


class Profile {
public:
    Profile() = default;

    Profile(const Profile& other) = default;
    virtual ~Profile() = default;

    ProfileKind kind;
    // We keep basic metadata that any kernel might find useful
    double x = 0, y = 0, z = 0;
    double rotDeg = 0;
};
