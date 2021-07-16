#include "QJsonCaster.hpp"
#include "QvPlugin/Common/CommonTypes.hpp"
#include "QvPlugin/PluginInterface.hpp"
#include "QvPlugin/Utils/ForEachMacros.hpp"

#include <iostream>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace pybind11::detail
{
    // Thanks https://gist.github.com/imikejackson/d13a00a639271eb99379ebcc95d12424
    template<>
    struct type_caster<QString>
    {
      public:
        PYBIND11_TYPE_CASTER(QString, _("QString"));
        bool load(handle src, bool)
        {
            if (!src)
                return false;

            handle load_src = src;
            if (PyUnicode_Check(load_src.ptr()))
            {
                object temp = reinterpret_steal<object>(PyUnicode_AsUTF8String(load_src.ptr()));
                if (!temp) /* A UnicodeEncodeError occured */
                {
                    PyErr_Clear();
                    return false;
                }
                load_src = temp;
            }
            char *buffer = nullptr;
            ssize_t length = 0;
            int err = PYBIND11_BYTES_AS_STRING_AND_SIZE(load_src.ptr(), &buffer, &length);
            if (err == -1) /* A TypeError occured */
            {
                PyErr_Clear();
                return false;
            }
            value = QString::fromUtf8(buffer, length);
            return true;
        }

        static handle cast(const QString &src, return_value_policy /* policy */, handle /* parent */)
        {
#if PY_VERSION_HEX >= 0x03030000 // Python 3.3
            assert(sizeof(QChar) == 2);
            return PyUnicode_FromKindAndData(PyUnicode_2BYTE_KIND, src.constData(), src.length());
#else
            QByteArray a = src.toUtf8();
            return PyUnicode_FromStringAndSize(a.data(), (ssize_t) a.length());
#endif
        }
    };
} // namespace pybind11::detail

#define TAKE_FIRST_EXPAND(x, y) x
#define TAKE_FIRST_IMPL(x) TAKE_FIRST_EXPAND x
#define TAKE_FIRST(...) FOR_EACH_COMMA_DELIM(TAKE_FIRST_IMPL, __VA_ARGS__)

#define TAKE_SECOND_EXPAND(x, y) y
#define TAKE_SECOND_IMPL(x) TAKE_SECOND_EXPAND x
#define TAKE_SECOND(...) FOR_EACH_COMMA_DELIM(TAKE_SECOND_IMPL, __VA_ARGS__)

#define GEN_TYPES_WITH_COUNTER_PAIR_IMPL2(x, ctr) (const x &CONCATENATE1(x, ctr), CONCATENATE1(x, ctr))
#define GEN_TYPES_WITH_COUNTER_PAIR_IMPL(x) GEN_TYPES_WITH_COUNTER_PAIR_IMPL2(x, __COUNTER__)
#define GEN_TYPES_WITH_COUNTER_PAIR(...) FOR_EACH_COMMA_DELIM(GEN_TYPES_WITH_COUNTER_PAIR_IMPL, __VA_ARGS__)

#define EXPAND_CODE_IMPL(name, types, args) ProfileManagerModule.def(#name, [](types) { return Qv2rayPlugin::Qv2rayInterfaceImpl::ProfileManager()->name(args); });
#define EXPAND_TYPE_VARIABLES(name, ...) EXPAND_CODE_IMPL(name, TAKE_FIRST(__VA_ARGS__), TAKE_SECOND(__VA_ARGS__))
#define REGISTER_PROFILEMANAGER_FUNC(name, ...) EXPAND_TYPE_VARIABLES(name, GEN_TYPES_WITH_COUNTER_PAIR(__VA_ARGS__))
#define REGISTER_PROFILEMANAGER_FUNC_ARG0(name) EXPAND_CODE_IMPL(name, , )

#define REGISTER_ID_TYPE(type)                                                                                                                                           \
    py::class_<type>(m, #type).def(py::init<const QString &>()).def("toString", &type::toString).def("isNull", &type::isNull).def("__repr__", [](const type &a) {        \
        return "<Qv2rayBase." #type " with content '" + a.toString() + "'>";                                                                                             \
    })
#define REGISTER_ID_TYPE_VALUE(id) m.attr(#id) = &id

#define REGISTER_JSON_TYPE(type)                                                                                                                                         \
    py::class_<type>(m, #type).def(py::init()).def(py::init<const QJsonObject &>()).def("toJson", &type::toJson).def("loadJson", &type::loadJson);

#define REGISTER_DECORATOR(dec)                                                                                                                                          \
    m.def(#dec, [](py::object arg) {                                                                                                                                     \
        const auto pyfunc = py::function(arg);                                                                                                                           \
        const auto funcName = QString::fromStdString(py::str(pyfunc.attr("__name__")));                                                                                  \
        const auto moduleName = QString::fromStdString(py::str(pyfunc.attr("__module__")));                                                                              \
        qDebug() << "Decorator" << #dec << "called from function: " << moduleName << funcName;                                                                           \
        dec##s << pyfunc;                                                                                                                                                \
        return arg;                                                                                                                                                      \
    });

QList<py::function> objs;

PYBIND11_EMBEDDED_MODULE(Qv2rayBase, m)
{
    REGISTER_ID_TYPE(ConnectionId);
    REGISTER_ID_TYPE(GroupId);
    REGISTER_ID_TYPE(RoutingId);
    REGISTER_ID_TYPE(PluginId);
    REGISTER_ID_TYPE(KernelId);
    REGISTER_ID_TYPE(LatencyTestEngineId);
    REGISTER_ID_TYPE(SubscriptionDecoderId);

    REGISTER_ID_TYPE_VALUE(DefaultGroupId);
    REGISTER_ID_TYPE_VALUE(DefaultRoutingId);
    REGISTER_ID_TYPE_VALUE(NullConnectionId);
    REGISTER_ID_TYPE_VALUE(NullGroupId);
    REGISTER_ID_TYPE_VALUE(NullKernelId);
    REGISTER_ID_TYPE_VALUE(NullRoutingId);

    REGISTER_JSON_TYPE(IOProtocolSettings);
    REGISTER_JSON_TYPE(IOStreamSettings);
    REGISTER_JSON_TYPE(RuleExtraSettings);
    REGISTER_JSON_TYPE(BalancerSelectorSettings);

    {
        auto ProfileManagerModule = m.def_submodule("ProfileManager");

        REGISTER_PROFILEMANAGER_FUNC_ARG0(GetConnections);
        REGISTER_PROFILEMANAGER_FUNC_ARG0(GetGroups);
        REGISTER_PROFILEMANAGER_FUNC_ARG0(StopConnection);
        REGISTER_PROFILEMANAGER_FUNC_ARG0(RestartConnection);

        REGISTER_PROFILEMANAGER_FUNC(IsConnected, ProfileId);
        REGISTER_PROFILEMANAGER_FUNC(GetConnection, ConnectionId);
        REGISTER_PROFILEMANAGER_FUNC(GetConnectionObject, ConnectionId);
        REGISTER_PROFILEMANAGER_FUNC(GetGroupObject, GroupId);
        REGISTER_PROFILEMANAGER_FUNC(GetConnections, GroupId);
        REGISTER_PROFILEMANAGER_FUNC(GetGroups, ConnectionId);
        REGISTER_PROFILEMANAGER_FUNC(StartConnection, ProfileId);
        REGISTER_PROFILEMANAGER_FUNC(CreateConnection, ProfileContent, QString, GroupId);
        REGISTER_PROFILEMANAGER_FUNC(RenameConnection, ConnectionId, QString);
        REGISTER_PROFILEMANAGER_FUNC(UpdateConnection, ConnectionId, ProfileContent);
        REGISTER_PROFILEMANAGER_FUNC(RemoveFromGroup, ConnectionId, GroupId);
        REGISTER_PROFILEMANAGER_FUNC(MoveToGroup, ConnectionId, GroupId, GroupId);
        REGISTER_PROFILEMANAGER_FUNC(LinkWithGroup, ConnectionId, GroupId);
        REGISTER_PROFILEMANAGER_FUNC(CreateGroup, QString);
        REGISTER_PROFILEMANAGER_FUNC(DeleteGroup, GroupId, bool);
        REGISTER_PROFILEMANAGER_FUNC(RenameGroup, GroupId, QString);
        REGISTER_PROFILEMANAGER_FUNC(GetGroupRoutingId, GroupId);
        REGISTER_PROFILEMANAGER_FUNC(GetRouting, RoutingId);
        REGISTER_PROFILEMANAGER_FUNC(UpdateRouting, RoutingId, RoutingObject);
    }

    REGISTER_DECORATOR(obj)

    py::class_<ProfileId>(m, "ProfileId")
        .def(py::init())
        .def(py::init<const ConnectionId &, const GroupId &>())
        .def_readwrite("connectionId", &ProfileId::connectionId)
        .def_readwrite("groupId", &ProfileId ::groupId)
        .def("clear", &ProfileId::clear)
        .def("isNull", &ProfileId::isNull)
        .def("__repr__", [](const ProfileId &a) {
            return "<Qv2rayBase.ProfileId with connection id '" + a.connectionId.toString() + "' and group id '" + a.groupId.toString() + "'>";
        });
}

int main(int, char *[])
{
    py::scoped_interpreter guard{};
    py::eval_file("./test.py");

    for (const auto &f : objs)
    {
        py::print("Invoking callback: ", py::arg("end") = " ");
        py::print(f);
        f();
    }
    return 0;
}
