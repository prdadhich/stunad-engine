#include "../engine/occt/OcctKernel.h"
#include "../engine/occt/OcctSolid.h"
#include "../engine/models/LampModel.h"
#include <STEPControl_Writer.hxx>

int main()
{
    OcctKernel kernel;
    LampModel lamp(&kernel);

    lamp.base.height.Set(300);
    lamp.base.radius.Set(80);

    OcctSolid* solid = (OcctSolid*)lamp.Get();

    STEPControl_Writer writer;
    writer.Transfer(solid->shape, STEPControl_AsIs);
    writer.Write("lamp.step");

    return 0;
}
