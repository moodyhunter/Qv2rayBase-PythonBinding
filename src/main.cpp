#include "QvPlugin/Common/CommonTypes.hpp"

#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace pybind11::detail
{
    template<>
    struct type_caster<QString>
    {
      public:
        PYBIND11_TYPE_CASTER(QString, _("QString"));
        bool load(handle src, bool)
        {
            value = QString::fromStdString(py::str(src));
            return true;
        }
        static handle cast(const QString &src, return_value_policy /* policy */, handle /* parent */)
        {
            return py::cast(src.toStdString());
        }
    };
} // namespace pybind11::detail

#define REGISTER_ID_TYPE(type) py::class_<type>(m, #type).def(py::init<const QString &>()).def("toString", &type::toString).def("isNull", &type::isNull)
#define REGISTER_ID_TYPE_VALUE(id) m.attr(#id) = id;

PYBIND11_EMBEDDED_MODULE(Qv2rayBase, m)
{
    REGISTER_ID_TYPE(GroupId);
    REGISTER_ID_TYPE(ConnectionId);
    REGISTER_ID_TYPE(RoutingId);
    REGISTER_ID_TYPE(PluginId);
    REGISTER_ID_TYPE(KernelId);
    REGISTER_ID_TYPE(LatencyTestEngineId);
    REGISTER_ID_TYPE(SubscriptionDecoderId);

    REGISTER_ID_TYPE_VALUE(DefaultGroupId)
    REGISTER_ID_TYPE_VALUE(DefaultRoutingId)
    REGISTER_ID_TYPE_VALUE(NullConnectionId)
    REGISTER_ID_TYPE_VALUE(NullGroupId)
    REGISTER_ID_TYPE_VALUE(NullKernelId)
    REGISTER_ID_TYPE_VALUE(NullRoutingId)
}

int main(int, char *[])
{
    py::scoped_interpreter guard{};
    py::exec("import Qv2rayBase");
    py::exec("x = Qv2rayBase.ConnectionId(\"OK\")");
    py::exec("print(type(x))");
    py::exec("print(x.toString())");
    py::exec("print(type(x.toString()))");
    py::exec("print(Qv2rayBase.DefaultGroupId)");
    py::exec("print(Qv2rayBase.DefaultGroupId.toString())");
    py::eval<py::eval_statements>("def test():\n    print(2)");
    return 0;
}
