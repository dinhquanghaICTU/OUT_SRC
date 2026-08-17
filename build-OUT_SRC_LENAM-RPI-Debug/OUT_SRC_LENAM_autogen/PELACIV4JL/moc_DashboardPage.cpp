/****************************************************************************
** Meta object code from reading C++ file 'DashboardPage.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../OUT_SRC_LENAM/ui/pages/DashboardPage.h"
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
    "updateReading",
    "",
    "SensorReading",
    "reading",
    "setDevices",
    "devices"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSDashboardPageENDCLASS_t {
    uint offsetsAndSizes[14];
    char stringdata0[14];
    char stringdata1[14];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[8];
    char stringdata5[11];
    char stringdata6[8];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSDashboardPageENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSDashboardPageENDCLASS_t qt_meta_stringdata_CLASSDashboardPageENDCLASS = {
    {
        QT_MOC_LITERAL(0, 13),  // "DashboardPage"
        QT_MOC_LITERAL(14, 13),  // "updateReading"
        QT_MOC_LITERAL(28, 0),  // ""
        QT_MOC_LITERAL(29, 13),  // "SensorReading"
        QT_MOC_LITERAL(43, 7),  // "reading"
        QT_MOC_LITERAL(51, 10),  // "setDevices"
        QT_MOC_LITERAL(62, 7)   // "devices"
    },
    "DashboardPage",
    "updateReading",
    "",
    "SensorReading",
    "reading",
    "setDevices",
    "devices"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSDashboardPageENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       1,    1,   26,    2, 0x0a,    1 /* Public */,
       5,    1,   29,    2, 0x0a,    3 /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Void, QMetaType::QJsonArray,    6,

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
        // method 'updateReading'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const SensorReading &, std::false_type>,
        // method 'setDevices'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QJsonArray &, std::false_type>
    >,
    nullptr
} };

void DashboardPage::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<DashboardPage *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->updateReading((*reinterpret_cast< std::add_pointer_t<SensorReading>>(_a[1]))); break;
        case 1: _t->setDevices((*reinterpret_cast< std::add_pointer_t<QJsonArray>>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< SensorReading >(); break;
            }
            break;
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
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_WARNING_POP
