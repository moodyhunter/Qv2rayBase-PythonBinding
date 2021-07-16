#pragma once

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <pybind11/pybind11.h>

namespace py = pybind11;

namespace PyQtJson
{
    inline py::object from_json(const QJsonValue &j)
    {
        if (j.isNull())
        {
            return py::none();
        }
        else if (j.isBool())
        {
            return py::bool_(j.toBool());
        }
        else if (j.isDouble())
        {
            const auto val = j.toDouble();
            if (val == std::floor(val))
                return py::int_(j.toInt());
            return py::float_(val);
        }
        else if (j.isString())
        {
            return py::str(j.toString().toStdString());
        }
        else if (j.isArray())
        {
            py::list obj;
            for (const auto &el : j.toArray())
            {
                obj.append(from_json(el));
            }
            return std::move(obj);
        }
        else // Object
        {
            py::dict obj;
            const auto o = j.toObject();
            for (auto it = o.constBegin(); it != o.constEnd(); ++it)
            {
                obj[py::str(it.key().toStdString())] = from_json(it.value());
            }
            return std::move(obj);
        }
    }

    inline QJsonValue to_json(const py::handle &obj)
    {
        if (obj.ptr() == nullptr || obj.is_none())
        {
            return QJsonValue::Null;
        }
        if (py::isinstance<py::bool_>(obj))
        {
            return obj.cast<bool>();
        }
        if (py::isinstance<py::int_>(obj))
        {
            return obj.cast<long long>();
        }
        if (py::isinstance<py::float_>(obj))
        {
            return obj.cast<double>();
        }
        if (py::isinstance<py::bytes>(obj))
        {
            py::module base64 = py::module::import("base64");
            return QString::fromStdString(base64.attr("b64encode")(obj).attr("decode")("utf-8").cast<std::string>());
        }
        if (py::isinstance<py::str>(obj))
        {
            return QString::fromStdString(obj.cast<std::string>());
        }
        if (py::isinstance<py::tuple>(obj) || py::isinstance<py::list>(obj))
        {
            QJsonArray out;
            for (const py::handle value : obj)
            {
                out.push_back(to_json(value));
            }
            return out;
        }
        if (py::isinstance<py::dict>(obj))
        {
            QJsonObject out;
            for (const py::handle key : obj)
            {
                out.insert(QString::fromStdString(py::str(key).cast<std::string>()), to_json(obj[key]));
            }
            return out;
        }
        throw std::runtime_error("to_json not implemented for this type of object: " + py::repr(obj).cast<std::string>());
    }
} // namespace PyQtJson

namespace pybind11::detail
{
    template<>
    struct type_caster<QJsonValue>
    {
      public:
        PYBIND11_TYPE_CASTER(QJsonValue, _("json.json"));

        bool load(handle src, bool)
        {
            value = PyQtJson::to_json(src);
            return true;
        }

        static handle cast(QJsonValue src, return_value_policy /* policy */, handle /* parent */)
        {
            object obj = PyQtJson::from_json(src);
            return obj.release();
        }
    };

    template<>
    struct type_caster<QJsonObject>
    {
      public:
        PYBIND11_TYPE_CASTER(QJsonObject, _("json.json"));

        bool load(handle src, bool)
        {
            value = PyQtJson::to_json(src).toObject();
            return true;
        }

        static handle cast(QJsonValue src, return_value_policy /* policy */, handle /* parent */)
        {
            object obj = PyQtJson::from_json(src);
            return obj.release();
        }
    };
} // namespace pybind11::detail
