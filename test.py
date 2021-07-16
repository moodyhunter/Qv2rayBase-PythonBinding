import Qv2rayBase
import sys
import json

x = Qv2rayBase.ConnectionId("OK")
print(type(x))
print(Qv2rayBase.DefaultGroupId)
y = Qv2rayBase.DefaultGroupId
print(y)
print(y.toString())

print(type(Qv2rayBase.DefaultGroupId))
print(Qv2rayBase.DefaultGroupId.toString())


@Qv2rayBase.obj
def _():
    print("func1")


@Qv2rayBase.obj
def f2():
    print("func2")


proto = Qv2rayBase.IOProtocolSettings()
j = json.loads('{"foo": {"bar":["baz", null, 1.0, 2]}}')
print(type(j))
proto.loadJson(j)

print(proto.toJson())

profId = Qv2rayBase.ProfileId()
print(profId.connectionId)
profId.groupId = Qv2rayBase.GroupId("group")
print(profId)

conn = Qv2rayBase.ConnectionObject()
print(conn.toJson())


print(Qv2rayBase.ProfileContent())
print(Qv2rayBase.ProfileContent().toJson())
