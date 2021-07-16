#pragma once

#include <QList>
#include <QMap>
#include <chrono>
#include <pybind11/chrono.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

namespace pybind11::detail
{
    // Thanks https://gist.github.com/imikejackson/d13a00a639271eb99379ebcc95d12424
    template<>
    struct type_caster<QString>
    {
      public:
        PYBIND11_TYPE_CASTER(QString, _("str"));
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

    template<typename T>
    struct type_caster<QList<T>>
    {
      public:
        using t_conv = make_caster<T>;
        PYBIND11_TYPE_CASTER(QList<T>, _("List[") + t_conv::name + _("]"));

        bool load(handle src, bool convert)
        {
            if (!isinstance<sequence>(src) || isinstance<bytes>(src) || isinstance<str>(src))
                return false;
            auto s = reinterpret_borrow<sequence>(src);
            value.clear();
            value.reserve(s.size());
            for (auto it : s)
            {
                t_conv conv;
                if (!conv.load(it, convert))
                    return false;
                value.push_back(cast_op<T &&>(std::move(conv)));
            }
            return true;
        }

        static handle cast(QList<T> src, return_value_policy policy, handle parent)
        {
            if (!std::is_lvalue_reference<T>::value)
                policy = return_value_policy_override<T>::policy(policy);
            list l(src.size());
            size_t index = 0;
            for (auto &&value : src)
            {
                auto value_ = reinterpret_steal<object>(t_conv::cast(value, policy, parent));
                if (!value_)
                    return handle();
                PyList_SET_ITEM(l.ptr(), (ssize_t) index++, value_.release().ptr()); // steals a reference
            }
            return l.release();
        }
    };

    template<typename Key, typename Value>
    struct type_caster<QMap<Key, Value>>
    {
        using key_conv = make_caster<Key>;
        using value_conv = make_caster<Value>;

        bool load(handle src, bool convert)
        {
            if (!isinstance<dict>(src))
                return false;
            auto d = reinterpret_borrow<dict>(src);
            value.clear();
            for (auto it : d)
            {
                key_conv kconv;
                value_conv vconv;
                if (!kconv.load(it.first.ptr(), convert) || !vconv.load(it.second.ptr(), convert))
                    return false;
                value.insert(cast_op<Key &&>(std::move(kconv)), cast_op<Value &&>(std::move(vconv)));
            }
            return true;
        }

        static handle cast(const QMap<Key, Value> &src, return_value_policy policy, handle parent)
        {
            dict d;
            for (auto it = src.constKeyValueBegin(); it != src.constKeyValueEnd(); it++)
            {
                auto key = reinterpret_steal<object>(key_conv::cast(it->first, policy, parent));
                auto value = reinterpret_steal<object>(value_conv::cast(it->second, policy, parent));
                if (!key || !value)
                    return handle();
                d[key] = value;
            }
            return d.release();
        }

        PYBIND11_TYPE_CASTER(PYBIND11_TYPE(QMap<Key, Value>), _("Dict[") + key_conv::name + _(", ") + value_conv::name + _("]"));
    };

} // namespace pybind11::detail
