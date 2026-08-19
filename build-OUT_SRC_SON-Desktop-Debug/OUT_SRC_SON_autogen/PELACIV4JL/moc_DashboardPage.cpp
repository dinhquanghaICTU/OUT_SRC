/****************************************************************************
** Meta object code from reading C++ file 'DashboardPage.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../OUT_SRC_SON/OUT_SRC_SON/ui/pages/DashboardPage.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DashboardPage.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 68
#error "This file was generated using the moc from 6.5.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSDashboardPageENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSDashboardPageENDCLASS = QtMocHelpers::stringData(
    "DashboardPage",
    "relayControlRequested",
    "",
    "deviceId",
    "state",
    "deviceConfigRequested",
    "config",
    "addDeviceRequested",
    "releaseDeviceRequested",
    "updateReading",
    "SensorReading",
    "reading",
    "updateDeviceMetrics",
    "metrics",
    "setDeviceId",
    "setHasDevice",
    "hasDevice",
    "deviceName",
    "openPumpAutoConfig"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDashboardPageENDCLASS_t {
    uint offsetsAndSizes[38];
    char stringdata0[14];
    char stringdata1[22];
    char stringdata2[1];
    char stringdata3[9];
    char stringdata4[6];
    char stringdata5[22];
    char stringdata6[7];
    char stringdata7[19];
    char stringdata8[23];
    char stringdata9[14];
    char stringdata10[14];
    char stringdata11[8];
    char stringdata12[20];
    char stringdata13[8];
    char stringdata14[12];
    char stringdata15[13];
    char stringdata16[10];
    char stringdata17[11];
    char stringdata18[19];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDashboardPageENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDashboardPageENDCLASS_t qt_meta_stringdata_CLASSDashboardPageENDCLASS = {
    {
        QT_MOC_LITERAL(0, 13),  // "DashboardPage"
        QT_MOC_LITERAL(14, 21),  // "relayControlRequested"
        QT_MOC_LITERAL(36, 0),  // ""
        QT_MOC_LITERAL(37, 8),  // "deviceId"
        QT_MOC_LITERAL(46, 5),  // "state"
        QT_MOC_LITERAL(52, 21),  // "deviceConfigRequested"
        QT_MOC_LITERAL(74, 6),  // "config"
        QT_MOC_LITERAL(81, 18),  // "addDeviceRequested"
        QT_MOC_LITERAL(100, 22),  // "releaseDeviceRequested"
        QT_MOC_LITERAL(123, 13),  // "updateReading"
        QT_MOC_LITERAL(137, 13),  // "SensorReading"
        QT_MOC_LITERAL(151, 7),  // "reading"
        QT_MOC_LITERAL(159, 19),  // "updateDeviceMetrics"
        QT_MOC_LITERAL(179, 7),  // "metrics"
        QT_MOC_LITERAL(187, 11),  // "setDeviceId"
        QT_MOC_LITERAL(199, 12),  // "setHasDevice"
        QT_MOC_LITERAL(212, 9),  // "hasDevice"
        QT_MOC_LITERAL(222, 10),  // "deviceName"
        QT_MOC_LITERAL(233, 18)   // "openPumpAutoConfig"
    },
    "DashboardPage",
    "relayControlRequested",
    "",
    "deviceId",
    "state",
    "deviceConfigRequested",
    "config",
    "addDeviceRequested",
    "releaseDeviceRequested",
    "updateReading",
    "SensorReading",
    "reading",
    "updateDeviceMetrics",
    "metrics",
    "setDeviceId",
    "setHasDevice",
    "hasDevice",
    "deviceName",
    "openPumpAutoConfig"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDashboardPageENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
      11,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   80,    2, 0x06,    1 /* Public */,
       5,    2,   85,    2, 0x06,    4 /* Public */,
       7,    0,   90,    2, 0x06,    7 /* Public */,
       8,    1,   91,    2, 0x06,    8 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       9,    1,   94,    2, 0x0a,   10 /* Public */,
      12,    1,   97,    2, 0x0a,   12 /* Public */,
      14,    1,  100,    2, 0x0a,   14 /* Public */,
      15,    3,  103,    2, 0x0a,   16 /* Public */,
      15,    2,  110,    2, 0x2a,   20 /* Public | MethodCloned */,
      15,    1,  115,    2, 0x2a,   23 /* Public | MethodCloned */,
      18,    0,  118,    2, 0x0a,   25 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::Bool,    3,    4,
    QMetaType::Void, QMetaType::QString, QMetaType::QJsonObject,    3,    6,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 10,   11,
    QMetaType::Void, QMetaType::QJsonObject,   13,
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString, QMetaType::QString,   16,    3,   17,
    QMetaType::Void, QMetaType::Bool, QMetaType::QString,   16,    3,
    QMetaType::Void, QMetaType::Bool,   16,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject DashboardPage::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSDashboardPageENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSDashboardPageENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSDashboardPageENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<DashboardPage, std::true_type>,
        // method 'relayControlRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'deviceConfigRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonObject &, std::false_type>,
        // method 'addDeviceRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'releaseDeviceRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'updateReading'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const SensorReading &, std::false_type>,
        // method 'updateDeviceMetrics'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonObject &, std::false_type>,
        // method 'setDeviceId'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'setHasDevice'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'setHasDevice'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'setHasDevice'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<bool, std::false_type>,
        // method 'openPumpAutoConfig'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void DashboardPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DashboardPage *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->relayControlRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<bool>>(_a[2]))); break;
        case 1: _t->deviceConfigRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[2]))); break;
        case 2: _t->addDeviceRequested(); break;
        case 3: _t->releaseDeviceRequested((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 4: _t->updateReading((*reinterpret_cast< std::add_pointer_t<SensorReading>>(_a[1]))); break;
        case 5: _t->updateDeviceMetrics((*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[1]))); break;
        case 6: _t->setDeviceId((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 7: _t->setHasDevice((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[3]))); break;
        case 8: _t->setHasDevice((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 9: _t->setHasDevice((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1]))); break;
        case 10: _t->openPumpAutoConfig(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 4:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< SensorReading >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (DashboardPage::*)(const QString & , bool );
            if (_t _q_method = &DashboardPage::relayControlRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (DashboardPage::*)(const QString & , const QJsonObject & );
            if (_t _q_method = &DashboardPage::deviceConfigRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (DashboardPage::*)();
            if (_t _q_method = &DashboardPage::addDeviceRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (DashboardPage::*)(const QString & );
            if (_t _q_method = &DashboardPage::releaseDeviceRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 3;
                return;
            }
        }
    }
}

const QMetaObject *DashboardPage::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DashboardPage::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSDashboardPageENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int DashboardPage::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    return _id;
}

// SIGNAL 0
void DashboardPage::relayControlRequested(const QString & _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void DashboardPage::deviceConfigRequested(const QString & _t1, const QJsonObject & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void DashboardPage::addDeviceRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void DashboardPage::releaseDeviceRequested(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
