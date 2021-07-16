#include "QContainerCaster.hpp"
#include "QJsonCaster.hpp"
#include "QvPlugin/Common/CommonTypes.hpp"
#include "QvPlugin/PluginInterface.hpp"
#include "QvPlugin/Utils/ForEachMacros.hpp"

#include <iostream>
#include <pybind11/chrono.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>

#ifdef QV2RAY_PYTHON_BINDING_LIBRARY
#define QV2RAY_PYBIND_MODULE PYBIND11_MODULE
#else
#define QV2RAY_PYBIND_MODULE PYBIND11_EMBEDDED_MODULE
#endif

namespace py = pybind11;

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

#define REGISTER_CLASS_COMMAND_I(...) .def(py::init<__VA_ARGS__>())
#define REGISTER_CLASS_COMMAND_F(field) .def(#field, &_T::field)
#define REGISTER_CLASS_COMMAND_RW(field) .def_readwrite(#field, &_T::field)
#define REGISTER_CLASS_COMMAND_REPR(field) .def("__repr__", [](const _T &a) { return field; });

#define REGISTER_CLASS_IMPL(command) REGISTER_CLASS_COMMAND_##command

#define CLASS(Class, ...)                                                                                                                                                \
    {                                                                                                                                                                    \
        using _T = Class;                                                                                                                                                \
        py::class_<Class>(m, #Class).def(py::init<>()) FOR_EACH(REGISTER_CLASS_IMPL, __VA_ARGS__);                                                                       \
    }
#define CLASS_WITH_BASE(Class, Base, ...)                                                                                                                                \
    {                                                                                                                                                                    \
        using _T = Class;                                                                                                                                                \
        py::class_<Class, Base>(m, #Class).def(py::init<>()) FOR_EACH(REGISTER_CLASS_IMPL, __VA_ARGS__);                                                                 \
    }

#define CLASS_WITH_JSON(Class, ...) CLASS(Class, F(toJson), F(loadJson), __VA_ARGS__)
#define CLASS_WITH_BASE_JSON(Class, Base, ...) CLASS_WITH_BASE(Class, Base, F(toJson), F(loadJson), __VA_ARGS__)

QList<py::function> objs;

QV2RAY_PYBIND_MODULE(Qv2rayBase, m)
{
    PyCapsule_Import(PyDateTime_CAPSULE_NAME, 0);
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

    REGISTER_DECORATOR(obj)

    CLASS_WITH_JSON(ProfileId,                                //
                    I(const ConnectionId &, const GroupId &), //
                    RW(connectionId),                         //
                    RW(groupId),                              //
                    F(clear),                                 //
                    F(isNull),                                //
                    REPR("<Qv2rayBase.ProfileId with connection id '" + a.connectionId.toString() + "' and group id '" + a.groupId.toString() + "'>"))

    py::enum_<StatisticsObject::StatisticsType>(m, "StatisticsType")
        .value("ALL", StatisticsObject::ALL)
        .value("DIRECT", StatisticsObject::DIRECT)
        .value("PROXY", StatisticsObject::PROXY);

    py::enum_<OutboundObject::OutboundObjectType>(m, "OutboundObjectType")
        .value("BALANCER", OutboundObject::BALANCER)
        .value("CHAIN", OutboundObject::CHAIN)
        .value("EXTERNAL", OutboundObject::EXTERNAL)
        .value("ORIGINAL", OutboundObject::ORIGINAL);

    py::enum_<SubscriptionConfigObject::SubscriptionFilterRelation>(m, "SubscriptionFilterRelation")
        .value("RELATION_OR", SubscriptionConfigObject::SubscriptionFilterRelation::RELATION_OR)
        .value("RELATION_AND", SubscriptionConfigObject::SubscriptionFilterRelation::RELATION_AND);

    CLASS_WITH_JSON(StatisticsObject, F(clear), RW(directUp), RW(directDown));
    CLASS_WITH_JSON(BaseTaggedObject, RW(name), RW(options))
    CLASS_WITH_BASE_JSON(BaseConfigTaggedObject, BaseTaggedObject, RW(created), RW(updated))
    CLASS_WITH_BASE_JSON(ConnectionObject, BaseConfigTaggedObject, RW(last_connected), RW(statistics), RW(latency))
    CLASS_WITH_BASE_JSON(SubscriptionConfigObject, BaseTaggedObject, //
                         RW(isSubscription),                         //
                         RW(address),                                //
                         RW(type),                                   //
                         RW(updateInterval),                         //
                         RW(includeKeywords),                        //
                         RW(excludeKeywords),                        //
                         RW(includeRelation),                        //
                         RW(excludeRelation))
    CLASS_WITH_BASE_JSON(GroupObject, BaseConfigTaggedObject, RW(connections), RW(route_id), RW(subscription_config))

    // Do not use CLASS_WITH* macros since "from" is a python keyword.
    py::class_<PortRange>(m, "PortRange")
        .def(py::init<>())
        .def_readwrite("from_port", &PortRange::from)
        .def_readwrite("to_port", &PortRange::to)
        .def("loadJson", &PortRange::loadJson)
        .def("toJson", &PortRange::toJson);

    CLASS_WITH_BASE_JSON(RuleObject, BaseTaggedObject, //
                         RW(enabled),                  //
                         RW(inboundTags),              //
                         RW(outboundTag),              //
                         RW(sourceAddresses),          //
                         RW(targetDomains),            //
                         RW(targetIPs),                //
                         RW(sourcePort),               //
                         RW(targetPort),               //
                         RW(networks),                 //
                         RW(protocols),                //
                         RW(processes),                //
                         RW(extraSettings))
    CLASS_WITH_JSON(RoutingObject, RW(overrideRules), //
                    RW(rules),                        //
                    RW(overrideDNS),                  //
                    RW(dns),                          //
                    RW(fakedns),                      //
                    RW(extraOptions))
    CLASS_WITH_JSON(MultiplexerObject, RW(enabled), RW(concurrency))
    CLASS_WITH_JSON(IOConnectionSettings, RW(protocol), RW(address), RW(port), RW(protocolSettings), RW(streamSettings), RW(muxSettings))
    CLASS_WITH_BASE_JSON(InboundObject, BaseTaggedObject, RW(inboundSettings))
    CLASS_WITH_BASE_JSON(BalancerSettings, BaseTaggedObject, RW(selectorType), RW(selectorSettings))
    CLASS_WITH_BASE_JSON(ChainSettings, BaseTaggedObject, RW(chaining_port), RW(chains))
    CLASS_WITH_BASE_JSON(OutboundObject, BaseTaggedObject, //
                         I(const IOConnectionSettings &),  //
                         I(const ConnectionId &),          //
                         I(const BalancerSettings &),      //
                         I(const ChainSettings &),         //
                         RW(objectType),                   //
                         RW(kernel),                       //
                         RW(externalId),                   //
                         RW(outboundSettings),             //
                         RW(balancerSettings),             //
                         RW(chainSettings))
    CLASS_WITH_JSON(BasicDNSServerObject, RW(address), RW(port))
    CLASS_WITH_JSON(BasicDNSObject, RW(servers), RW(hosts), RW(extraOptions))
    CLASS_WITH_JSON(ProfileContent, I(const OutboundObject &), RW(defaultKernel), RW(inbounds), RW(outbounds), RW(routing), RW(extraOptions))

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

#ifdef QV2RAY_PYBIND_STUBGEN
int main(int, char *[])
{
    py::scoped_interpreter guard{};
    py::exec(R"(
import mypy.stubgen
options = mypy.stubgen.parse_options(["-mQv2rayBase", "-mQv2rayBase.ProfileManager"])
mypy.stubgen.generate_stubs(options))");
    return 0;
}
#else
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
#endif
