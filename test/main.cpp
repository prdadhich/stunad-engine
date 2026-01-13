#include <BRepPrimAPI_MakeBox.hxx>
#include <STEPControl_Writer.hxx>

int main()
{
    TopoDS_Shape box = BRepPrimAPI_MakeBox(100, 80, 60).Shape();

    STEPControl_Writer writer;
    writer.Transfer(box, STEPControl_AsIs);
    writer.Write("box.step");

    return 0;
}
