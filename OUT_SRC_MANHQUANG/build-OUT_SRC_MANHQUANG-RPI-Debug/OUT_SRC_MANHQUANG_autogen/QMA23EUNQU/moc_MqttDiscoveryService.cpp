/****************************************************************************
** Meta object code from reading C++ file 'MqttDiscoveryService.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../OUT_SRC_MANHQUANG/server/mqtt/MqttDiscoveryService.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'MqttDiscoveryService.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSMqttDiscoveryServiceENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSMqttDiscoveryServiceENDCLASS = QtMocHelpers::stringData(
    "MqttDiscoveryService",
    "brokerConnected",
    "",
    "brokerDisconnected",
    "telemetryReceived",
    "deviceId",
    "metrics",
    "connectToBroker",
    "sendConnect",
    "sendSubscribe",
    "processPackets",
    "scheduleReconnect"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSMqttDiscoveryServiceENDCLASS_t {
    uint offsetsAndSizes[24];
    char stringdata0[21];
    char stringdata1[16];
    char stringdata2[1];
    char stringdata3[19];
    char stringdata4[18];
    char stringdata5[9];
    char stringdata6[8];
    char stringdata7[16];
    char stringdata8[12];
    char stringdata9[14];
    char stringdata10[15];
    char stringdata11[18];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSMqttDiscoveryServiceENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSMqttDiscoveryServiceENDCLASS_t qt_meta_stringdata_CLASSMqttDiscoveryServiceENDCLASS = {
    {
        QT_MOC_LITERAL(0, 20),  // "MqttDiscoveryService"
        QT_MOC_LITERAL(21, 15),  // "brokerConnected"
        QT_MOC_LITERAL(37, 0),  // ""
        QT_MOC_LITERAL(38, 18),  // "brokerDisconnected"
        QT_MOC_LITERAL(57, 17),  // "telemetryReceived"
        QT_MOC_LITERAL(75, 8),  // "deviceId"
        QT_MOC_LITERAL(84, 7),  // "metrics"
        QT_MOC_LITERAL(92, 15),  // "connectToBroker"
        QT_MOC_LITERAL(108, 11),  // "sendConnect"
        QT_MOC_LITERAL(120, 13),  // "sendSubscribe"
        QT_MOC_LITERAL(134, 14),  // "processPackets"
        QT_MOC_LITERAL(149, 17)   // "scheduleReconnect"
    },
    "MqttDiscoveryService",
    "brokerConnected",
    "",
    "brokerDisconnected",
    "telemetryReceived",
    "deviceId",
    "metrics",
    "connectToBroker",
    "sendConnect",
    "sendSubscribe",
    "processPackets",
    "scheduleReconnect"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSMqttDiscoveryServiceENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   62,    2, 0x06,    1 /* Public */,
       3,    0,   63,    2, 0x06,    2 /* Public */,
       4,    2,   64,    2, 0x06,    3 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       7,    0,   69,    2, 0x08,    6 /* Private */,
       8,    0,   70,    2, 0x08,    7 /* Private */,
       9,    0,   71,    2, 0x08,    8 /* Private */,
      10,    0,   72,    2, 0x08,    9 /* Private */,
      11,    0,   73,    2, 0x08,   10 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QJsonObject,    5,    6,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject MqttDiscoveryService::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_CLASSMqttDiscoveryServiceENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSMqttDiscoveryServiceENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSMqttDiscoveryServiceENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<MqttDiscoveryService, std::true_type>,
        // method 'brokerConnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'brokerDisconnected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'telemetryReceived'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonObject &, std::false_type>,
        // method 'connectToBroker'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sendConnect'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'sendSubscribe'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'processPackets'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'scheduleReconnect'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void MqttDiscoveryService::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MqttDiscoveryService *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->brokerConnected(); break;
        case 1: _t->brokerDisconnected(); break;
        case 2: _t->telemetryReceived((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QJsonObject>>(_a[2]))); break;
        case 3: _t->connectToBroker(); break;
        case 4: _t->sendConnect(); break;
        case 5: _t->sendSubscribe(); break;
        case 6: _t->processPackets(); break;
        case 7: _t->scheduleReconnect(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MqttDiscoveryService::*)();
            if (_t _q_method = &MqttDiscoveryService::brokerConnected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MqttDiscoveryService::*)();
            if (_t _q_method = &MqttDiscoveryService::brokerDisconnected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MqttDiscoveryService::*)(const QString & , const QJsonObject & );
            if (_t _q_method = &MqttDiscoveryService::telemetryReceived; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 2;
                return;
            }
        }
    }
}

const QMetaObject *MqttDiscoveryService::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MqttDiscoveryService::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSMqttDiscoveryServiceENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int MqttDiscoveryService::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 8;
    }
    return _id;
}

// SIGNAL 0
void MqttDiscoveryService::brokerConnected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void MqttDiscoveryService::brokerDisconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void MqttDiscoveryService::telemetryReceived(const QString & _t1, const QJsonObject & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_WARNING_POP
