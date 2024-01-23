#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "attachment.h"
#include "mesh.h"
#include "pinocchioApi.h"
#include "skeleton.h"
#include "vector.h"
#include "transform.h"


namespace py = pybind11;

PYBIND11_MAKE_OPAQUE(std::vector<Vector3>);
PYBIND11_MAKE_OPAQUE(std::vector<int>);
PYBIND11_MAKE_OPAQUE(std::vector<Transform<double> >);
PYBIND11_MAKE_OPAQUE(std::vector<MeshVertex>);
PYBIND11_MAKE_OPAQUE(std::vector<MeshEdge>);

class SkeletonBase : public Skeleton {
public:
    using Skeleton::makeJoint;
    using Skeleton::makeSymmetric;
    using Skeleton::initCompressed;
    using Skeleton::setFoot;
    using Skeleton::setFat;
};

PYBIND11_MODULE(pynocchio, m)
{
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif

    py::class_<Vector3>(m, "Vector3")
        .def(py::init<double, double, double>())
        .def("__getitem__", [](const Vector3 &v, int i) {
            if (i >= v.size())
                throw py::index_error();
            return v[i];
        })
        .def("__setitem__", [](Vector3 &v, int i, double value) {
            if (i >= v.size())
                throw py::index_error();
            v[i] = value;
        })
        .def("__len__", &Vector3::size);
    py::bind_vector<std::vector<Vector3> >(m, "Points");
    py::bind_vector<std::vector<int> >(m, "Indices");

    py::class_<Transform<double> >(m, "Transform")
        .def(py::init<>())
        .def(py::init<const Vector3 &>())
        .def("__mul__", [](const Transform<double> &t, const Vector3 &v) {
            return t * v;
        });
    py::bind_vector<std::vector<Transform<double> > >(m, "VectorTransform");

    py::class_<MeshVertex>(m, "MeshVertex")
        .def(py::init<>())
        .def(py::init<const Vector3 &>())
        .def_readwrite("position", &MeshVertex::pos)
        .def_readwrite("normal", &MeshVertex::normal);
    py::bind_vector<std::vector<MeshVertex> >(m, "VectorMeshVertex");

    py::class_<MeshEdge>(m, "MeshEdge")
        .def(py::init<>())
        .def(py::init<int>())
        .def_readwrite("vertex", &MeshEdge::vertex);
    py::bind_vector<std::vector<MeshEdge> >(m, "VectorMeshEdge");
    
    py::class_<Mesh>(m, "Mesh")
        .def(py::init<>())
        .def(py::init<const std::string &>())
        .def(py::init<const std::vector<Vector3> &, const std::vector<int> &>())
        .def_readwrite("vertices", &Mesh::vertices)
        .def_readwrite("edges", &Mesh::edges)
        .def("calculate_normals", &Mesh::computeVertexNormals);

    py::module skeletons_module = m.def_submodule("skeletons");
    py::class_<Skeleton> skeleton(skeletons_module, "Skeleton");
    skeleton
        .def_property_readonly("vertices", &Skeleton::fVerts)
        .def_property_readonly("parent_indices", &Skeleton::fPrev)
        .def("scale", &Skeleton::scale);

    py::class_<SkeletonBase>(skeletons_module, "SkeletonBase", skeleton)
        .def(py::init<>())
        .def("_make_joint", 
            static_cast<void (Skeleton::*)(const string &, const Vector3 &, const string &)>(&SkeletonBase::makeJoint),
            py::arg("name"), py::arg("position"), py::arg("previous") = py::str())
        .def("_make_symmetric", 
            static_cast<void (Skeleton::*)(const string &, const string &)>(&SkeletonBase::makeSymmetric),
            py::arg("name_1"), py::arg("name_2"))
        .def("_make_compressed", 
            static_cast<void (Skeleton::*)()>(&SkeletonBase::initCompressed))
        .def("_set_foot", 
            static_cast<void (Skeleton::*)(const string &)>(&SkeletonBase::setFoot),
            py::arg("name"))
        .def("_set_fat", 
            static_cast<void (Skeleton::*)(const string &)>(&SkeletonBase::setFat),
            py::arg("name"));

    py::class_<HumanSkeleton>(skeletons_module, "HumanSkeleton", skeleton)
        .def(py::init<>());

    py::class_<QuadSkeleton>(skeletons_module, "QuadSkeleton", skeleton)
        .def(py::init<>());

    py::class_<HorseSkeleton>(skeletons_module, "HorseSkeleton", skeleton)
        .def(py::init<>());

    py::class_<CentaurSkeleton>(skeletons_module, "CentaurSkeleton", skeleton)
        .def(py::init<>());

    py::class_<FileSkeleton>(skeletons_module, "FileSkeleton", skeleton)
        .def(py::init<const std::string &>());

    py::class_<Attachment>(m, "Attachment")
        .def(py::init<>())
        .def_property_readonly("embedding", &Attachment::getEmbedding)
        .def_property_readonly("full_weights", &Attachment::getFullWeights)
        .def_property_readonly("bones_per_vertex", &Attachment::getBonesPerVertex)
        .def("deform", &Attachment::deform);

    m.def("auto_rig", &autorig);
}
