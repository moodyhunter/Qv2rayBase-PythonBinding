import Qv2rayBase

x = Qv2rayBase.ConnectionId("OK")
print(type(x))
print(Qv2rayBase.DefaultGroupId)
y = Qv2rayBase.DefaultGroupId
print(y)
print(y.toString())

print(type(Qv2rayBase.DefaultGroupId))
print(Qv2rayBase.DefaultGroupId.toString())


@Qv2rayBase.callback
def _():
    print("func1")


@Qv2rayBase.callback
def f2():
    print("func2")
