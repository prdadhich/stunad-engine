#pragma once
#include "../core/Profile.h"
#include <TopoDS_Wire.hxx>

class OcctProfile : public Profile {
public:
    
    // Parameters (keep these for metadata/debugging)
    double radius = 0.0;
    double width = 0.0, height = 0.0;

    // The actual OCCT geometry
    TopoDS_Wire wire;

    // Circle constructor
    explicit OcctProfile(double r){
            this->kind =(ProfileKind::Circle);
            this->radius = r;
    }
        

    // Rect constructor
    OcctProfile(double w, double h) {
        this->kind = ProfileKind::Rect;
        this->width = w;
        this->height = h;
    }
      
    explicit OcctProfile(ProfileKind k) {
        this->kind = k;
    }

    // NEW: Copy constructor to ensure wires are passed correctly
    OcctProfile(const OcctProfile& other) : Profile(other) {
        //this->kind = other.kind;
        //this->x = other.x;
        //this->y = other.y;
        //this->z = other.z;
        //this->rotDeg = other.rotDeg;
        this->radius = other.radius;
        this->width = other.width;
        this->height = other.height;
        this->wire = other.wire; // OCCT handles handle-based copying
    }
};