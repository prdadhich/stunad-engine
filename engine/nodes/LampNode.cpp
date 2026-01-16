#include "LampNode.h"
#include "../core/Kernel.h"

Solid* LampNode::Evaluate(Kernel& kernel)
{
    double r = params[0].value;
    double h = params[1].value;
    double t = params[2].value;
  
    Solid* outer = kernel.MakeCylinder(r, h);
    Solid* inner = kernel.MakeCylinder(r - t, h - t);

    Solid* hollow = kernel.BooleanCut(outer, inner);
    Solid* filletR = kernel.Fillet(hollow,0.2);
    return filletR;
}
