import Qv2rayBase
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


conn = Qv2rayBase.IOProtocolSettings()
j = json.loads('{"foo": "bar"}')
print(type(j))
conn.loadJson(j)

print(conn.toJson())
