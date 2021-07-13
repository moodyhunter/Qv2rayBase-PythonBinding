#include "QvPlugin/Common/CommonTypes.hpp"
#include "QvPlugin/PluginInterface.hpp"
#include "QvPlugin/Utils/ForEachMacros.hpp"

#include <iostream>
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
#define PROFILE_MANAGER_COMMANDS_ARG_N(name, ...) EXPAND_TYPE_VARIABLES(name, GEN_TYPES_WITH_COUNTER_PAIR(__VA_ARGS__))
#define PROFILE_MANAGER_COMMANDS_ARG_0(name) EXPAND_CODE_IMPL(name, , )

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

    auto ProfileManagerModule = m.def_submodule("ProfileManager");

    PROFILE_MANAGER_COMMANDS_ARG_0(GetConnections);
    PROFILE_MANAGER_COMMANDS_ARG_0(GetGroups);
    PROFILE_MANAGER_COMMANDS_ARG_0(StopConnection);
    PROFILE_MANAGER_COMMANDS_ARG_0(RestartConnection);

    PROFILE_MANAGER_COMMANDS_ARG_N(IsConnected, ProfileId);
    PROFILE_MANAGER_COMMANDS_ARG_N(GetConnection, ConnectionId);
    PROFILE_MANAGER_COMMANDS_ARG_N(GetConnectionObject, ConnectionId);
    PROFILE_MANAGER_COMMANDS_ARG_N(GetGroupObject, GroupId);
    PROFILE_MANAGER_COMMANDS_ARG_N(GetConnections, GroupId);
    PROFILE_MANAGER_COMMANDS_ARG_N(GetGroups, ConnectionId);
    PROFILE_MANAGER_COMMANDS_ARG_N(StartConnection, ProfileId);
    PROFILE_MANAGER_COMMANDS_ARG_N(CreateConnection, ProfileContent, QString, GroupId);
    PROFILE_MANAGER_COMMANDS_ARG_N(RenameConnection, ConnectionId, QString);
    PROFILE_MANAGER_COMMANDS_ARG_N(UpdateConnection, ConnectionId, ProfileContent);
    PROFILE_MANAGER_COMMANDS_ARG_N(RemoveFromGroup, ConnectionId, GroupId);
    PROFILE_MANAGER_COMMANDS_ARG_N(MoveToGroup, ConnectionId, GroupId, GroupId);
    PROFILE_MANAGER_COMMANDS_ARG_N(LinkWithGroup, ConnectionId, GroupId);
    PROFILE_MANAGER_COMMANDS_ARG_N(CreateGroup, QString);
    PROFILE_MANAGER_COMMANDS_ARG_N(DeleteGroup, GroupId, bool);
    PROFILE_MANAGER_COMMANDS_ARG_N(RenameGroup, GroupId, QString);
    PROFILE_MANAGER_COMMANDS_ARG_N(GetGroupRoutingId, GroupId);
    PROFILE_MANAGER_COMMANDS_ARG_N(GetRouting, RoutingId);
    PROFILE_MANAGER_COMMANDS_ARG_N(UpdateRouting, RoutingId, RoutingObject);
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
    py::exec("print(Qv2rayBase.ProfileManager.GetConnections())");
    return 0;
}
