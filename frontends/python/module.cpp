/*******************************************************************************
 * frontends/python/module.cpp
 *
 * Part of Project Thrill - http://project-thrill.org
 *
 *
 * All rights reserved. Published under the BSD-2 license in the LICENSE file.
 ******************************************************************************/

#include <pybind11/pybind11.h>

#include <thrill/thrill.hpp>

#include <pybind11/stl.h>

#include <bytesobject.h>
#include <marshal.h>

using namespace thrill;

namespace py = pybind11;

static constexpr bool debug = false;

/******************************************************************************/

namespace thrill {
namespace data {

/*!
 * Thrill serialization interface for py::object: call the PyMarshal C API.
 */
#define MAKE_THRILL_SERIALIZER(T)                                             \
    template <typename Archive>                                               \
    struct Serialization<Archive, T> {                                        \
        static void Serialize(const T& obj, Archive & ar) {                   \
            PyObject* mar =                                                   \
                PyMarshal_WriteObjectToString(obj.ptr(), Py_MARSHAL_VERSION); \
            char* data;                                                       \
            Py_ssize_t len;                                                   \
            PyBytes_AsStringAndSize(mar, &data, &len);                        \
            ar.PutVarint(len).Append(data, len);                              \
            Py_DECREF(mar);                                                   \
        }                                                                     \
        static py::object Deserialize(Archive & ar) {                         \
            std::string data = ar.Read(ar.GetVarint());                       \
            return py::reinterpret_steal<T>(                                  \
                PyMarshal_ReadObjectFromString(                               \
                    const_cast<char*>(data.data()), data.size()));            \
        }                                                                     \
        static const bool   is_fixed_size = false;                            \
        static const size_t fixed_size = 0;                                   \
    }

MAKE_THRILL_SERIALIZER(py::object);
MAKE_THRILL_SERIALIZER(py::bool_);
MAKE_THRILL_SERIALIZER(py::int_);
MAKE_THRILL_SERIALIZER(py::float_);
MAKE_THRILL_SERIALIZER(py::str);

} // namespace data
} // namespace thrill

/******************************************************************************/

namespace std {
template <>
struct hash<py::object>: public std::unary_function<py::object, size_t> {
    size_t operator () (const py::object& ob) const {
        auto h = PyObject_Hash(ob.ptr());
        if (h == -1)
            throw std::exception();
        return h;
    }
};

} // namespace std

/******************************************************************************/

template <typename Return, typename Arg1>
class PyFunction1 : public py::function
{
public:
    PYBIND11_OBJECT_DEFAULT(PyFunction1, py::function, PyCallable_Check)
    Return operator () (Arg1 arg1) const {
        return py::function::operator () (arg1);
    }
};

template <typename Return, typename Arg1, typename Arg2>
class PyFunction2 : public py::function
{
public:
    PYBIND11_OBJECT_DEFAULT(PyFunction2, py::function, PyCallable_Check)
    Return operator () (Arg1 arg1, Arg2 arg2) const {
        return py::function::operator () (arg1, arg2);
    }
};

template <typename Return, typename Arg1>
class PyFunction1Cast : public py::function
{
public:
    PYBIND11_OBJECT_DEFAULT(PyFunction1Cast, py::function, PyCallable_Check)
    Return operator () (Arg1 arg1) const {
        return py::function::operator () (arg1).template cast<Return>();
    }
};

template <typename Return, typename Arg1, typename Arg2>
class PyFunction2Cast : public py::function
{
public:
    PYBIND11_OBJECT_DEFAULT(PyFunction2Cast, py::function, PyCallable_Check)
    Return operator () (Arg1 arg1, Arg2 arg2) const {
        return py::function::operator () (arg1, arg2).template cast<Return>();
    }
};

/******************************************************************************/
// Make HostContext and Context
// TODO: later auto-detect network environment

std::unique_ptr<HostContext> MakeHostContext() {
    std::vector<std::unique_ptr<HostContext> > hosts =
        HostContext::ConstructLoopback(
            /* num_hosts */ 1, /* workers_per_host */ 1);

    return std::move(hosts[0]);
}

std::unique_ptr<Context> MakeContext(HostContext& host_context) {
    return std::make_unique<Context>(host_context, /* local_worker_id */ 0);
}

/******************************************************************************/

template <typename DIAType, typename PybindModule>
auto RegisterDIAType(PybindModule& m, const char* py_name) {
    using ValueType = typename DIAType::ValueType;

    auto x =
        py::class_<DIAType>(m, py_name)
        .def("ctx", &DIAType::ctx)
        .def("id", &DIAType::id)
        .def("label", &DIAType::label)
        .def("Execute", &DIAType::Execute)
        .def("Map",
             [](const DIAType& dia, const PyFunction1<py::object, py::object>& func) {
                 return dia.Map(func).Collapse();
             })
        .def("Filter",
             [](const DIAType& dia, const PyFunction1<py::bool_, py::object>& func) {
                 return dia.Filter(func).Collapse();
             })
        .def("FlatMap",
             [](const DIAType& dia, const py::function& func) {
                 return dia.template FlatMap<py::object>(
                     // construct a C++ FlatMap lambda
                     [func](const py::object& obj, auto emit) {
                         return func(obj,
                                     // construct a Python-callable emitter
                                     py::cpp_function(
                                         [emit](const py::object& out) mutable {
                                             emit(out);
                                         }));
                     }).Collapse();
             })
        .def("BernoulliSample", &DIAType::BernoulliSample)
        .def("Union",
             [](const DIAType& dia1, const DIAType& dia2) {
                 return dia1.Union(dia2);
             })
        .def("Size", &DIAType::Size)
        .def("AllGather",
             [](DIAType& dia) { return dia.AllGather(); })
        .def("Print",
             [](const DIAType& dia, const std::string& name) {
                 return dia.Print(name);
             },
             py::arg("name") = "")
        .def("Gather",
             [](DIAType& dia, size_t target_id) {
                 return dia.Gather(target_id);
             },
             py::arg("target_id") = 0)
        .def("Sample", &DIAType::Sample)
        .def("AllReduce",
             [](const DIAType& dia,
                const PyFunction2<py::object, py::object, py::object>& func,
                const py::object& initial_value) {
                 if (initial_value.is_none())
                     return dia.AllReduce(func);
                 else
                     return dia.AllReduce(func, initial_value);
             },
             py::arg("func"),
             py::arg("initial_value") = py::none())
        .def("Sum",
             [](const DIAType& dia,
                const PyFunction2<py::object, py::object, py::object>& func,
                const py::object& initial_value) {
                 if (func.is_none()) {
                     if (initial_value.is_none())
                         return dia.Sum();
                     else
                         return dia.Sum(std::plus<py::object>(), initial_value);
                 }
                 else {
                     if (initial_value.is_none())
                         return dia.Sum(func);
                     else
                         return dia.Sum(func, initial_value);
                 }
             },
             py::arg("func") = py::none(),
             py::arg("initial_value") = py::none())
        .def("Min",
             [](const DIAType& dia, const py::object& initial_value) {
                 if (initial_value.is_none())
                     return dia.Min();
                 else
                     return dia.Min(initial_value);
             },
             py::arg("initial_value") = py::none())
        .def("Max",
             [](const DIAType& dia, const py::object& initial_value) {
                 if (initial_value.is_none())
                     return dia.Max();
                 else
                     return dia.Max(initial_value);
             },
             py::arg("initial_value") = py::none())
        .def("WriteBinary", &DIAType::WriteBinary)
        .def("ReduceByKey",
             [](const DIAType& dia,
                const PyFunction1<py::object, ValueType>& key_extractor,
                const PyFunction2<ValueType, ValueType, ValueType>& reduce) {
                 return dia.ReduceByKey(key_extractor, reduce);
             })
        .def("ReduceToIndex",
             [](const DIAType& dia,
                const PyFunction1Cast<size_t, ValueType>& key_extractor,
                const PyFunction2<ValueType, ValueType, ValueType>& reduce,
                size_t size) {
                 return dia.ReduceToIndex(key_extractor, reduce, size);
             })
        .def("Zip",
             [](const DIAType& dia1, const DIAType& dia2,
                const PyFunction2<py::object, py::object, py::object>& zip_function) {
                 return dia1.Zip(dia2, zip_function);
             })
        .def("ZipCut",
             [](const DIAType& dia1, const DIAType& dia2,
                const PyFunction2<py::object, py::object, py::object>& zip_function) {
                 return dia1.Zip(CutTag, dia2, zip_function);
             })
        .def("ZipPad",
             [](const DIAType& dia1, const DIAType& dia2,
                const PyFunction2<py::object, py::object, py::object>& zip_function) {
                 return dia1.Zip(PadTag, dia2, zip_function);
             })
        .def("ZipWithIndex",
             [](const DIAType& dia,
                const PyFunction2<py::object, py::object, size_t>& zip_function) {
                 return dia.ZipWithIndex(zip_function);
             })
        .def("Sort",
             [](const DIAType& dia) { return dia.Sort(); })
        .def("Sort",
             [](const DIAType& dia,
                const PyFunction2Cast<bool, py::object, py::object>& compare_function) {
                 return dia.Sort(compare_function);
             },
             py::arg("compare_function"))
        .def("PrefixSum",
             [](const DIAType& dia,
                const PyFunction2<py::object, py::object, py::object>& sum_function,
                const py::object& initial_element) {
                 if (initial_element.is_none())
                     return dia.PrefixSum(sum_function);
                 else
                     return dia.PrefixSum(sum_function, initial_element);
             },
             py::arg("func"),
             py::arg("initial_element") = py::none())
        .def("Cache", &DIAType::Cache)
    ;

    return x;
}

/******************************************************************************/

PYBIND11_MODULE(thrill, m) {
    m.doc() = R"pbdoc(
        Thrill API Reference
        --------------------
        .. currentmodule:: thrill

        .. rubric:: Classes and Types
        .. autosummary::
           :toctree: _generated

           HostContext
           Context
           DIA
    )pbdoc";

    /**************************************************************************/

    py::class_<HostContext>(
        m, "HostContext")
    .def(py::init(&MakeHostContext),
         "default constructor")
    .def("host_rank", &HostContext::host_rank)
    .def("workers_per_host", &HostContext::workers_per_host)
    ;

    py::class_<Context>(
        m, "Context")
    .def(py::init(&MakeContext),
         "default constructor")
    .def("num_hosts", &Context::num_hosts)
    .def("num_workers", &Context::num_workers)
    .def("workers_per_host", &Context::workers_per_host)
    .def("my_rank", &Context::my_rank)
    .def("host_rank", &Context::host_rank)
    .def("local_worker_id", &Context::local_worker_id)
    ;

    /**************************************************************************/

    using PyDIA = DIA<py::object>;
    RegisterDIAType<PyDIA>(m, "DIA");

    using BoolDIA = DIA<py::bool_>;
    RegisterDIAType<BoolDIA>(m, "BoolDIA");

    using IntDIA = DIA<py::int_>;
    RegisterDIAType<IntDIA>(m, "IntDIA");

    using FloatDIA = DIA<py::float_>;
    RegisterDIAType<FloatDIA>(m, "FloatDIA");

    using StrDIA = DIA<py::str>;
    RegisterDIAType<StrDIA>(m, "StrDIA")
    .def("WriteLinesOne",
         [](const StrDIA& dia, const std::string& file_path) {
             return dia
             // TODO: find a way without copying data
             .Map([](const py::str& s) { return std::string(s); })
             .WriteLinesOne(file_path);
         })
    .def("WriteLines",
         [](const StrDIA& dia, const std::string& file_path,
            size_t target_file_size) {
             return dia
             // TODO: find a way without copying data
             .Map([](const py::str& s) { return std::string(s); })
             .WriteLines(file_path, target_file_size);
         },
         py::arg("file_path"),
         py::arg("target_file_size") = 128 * 1024 * 1024)
    ;

    /**************************************************************************/

    m.def("Generate",
          [](Context& ctx, size_t size) {
              return Generate(
                  ctx, size, [](size_t index) { return py::int_(index); });
          },
          py::arg("ctx"),
          py::arg("size"));

    m.def("Generate",
          [](Context& ctx, size_t size, PyFunction1<py::object, size_t> func) {
              return Generate(ctx, size, func);
          },
          py::arg("ctx"),
          py::arg("size"),
          py::arg("func"));

    /**************************************************************************/

    m.def("ReadLines",
          [](Context& ctx, const std::string& filepath) {
              return ReadLines(ctx, filepath)
              // TODO: find a way without copying data
              .Map([](const std::string& s) { return py::str(s); })
              .Collapse();
          },
          py::arg("ctx"),
          py::arg("filepath"));

    m.def("ReadBinary",
          [](Context& ctx, const std::string& filepath) {
              return ReadBinary<py::object>(ctx, filepath);
          },
          py::arg("ctx"),
          py::arg("filepath"));

    /**************************************************************************/

#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}

/******************************************************************************/
