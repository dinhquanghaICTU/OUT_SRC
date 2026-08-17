/****************************************************************************
** Meta object code from reading C++ file 'SelectOnlineDeviceDialog.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../OUT_SRC_TUANANH/ui/dialogs/SelectOnlineDeviceDialog.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SelectOnlineDeviceDialog.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSSelectOnlineDeviceDialogENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSSelectOnlineDeviceDialogENDCLASS = QtMocHelpers::stringData(
    "SelectOnlineDeviceDialog",
    "deviceSelected",
    "",
    "deviceId",
    "deviceName",
    "refreshRequested"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSSelectOnlineDeviceDialogENDCLASS_t {
    uint offsetsAndSizes[12];
    char stringdata0[25];
    char stringdata1[15];
    char stringdata2[1];
    char stringdata3[9];
    char stringdata4[11];
    char stringdata5[17];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSSelectOnlineDeviceDialogENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSSelectOnlineDeviceDialogENDCLASS_t qt_meta_stringdata_CLASSSelectOnlineDeviceDialogENDCLASS = {
    {
        QT_MOC_LITERAL(0, 24),  // "SelectOnlineDeviceDialog"
        QT_MOC_LITERAL(25, 14),  // "deviceSelected"
        QT_MOC_LITERAL(40, 0),  // ""
        QT_MOC_LITERAL(41, 8),  // "deviceId"
        QT_MOC_LITERAL(50, 10),  // "deviceName"
        QT_MOC_LITERAL(61, 16)   // "refreshRequested"
    },
    "SelectOnlineDeviceDialog",
    "deviceSelected",
    "",
    "deviceId",
    "deviceName",
    "refreshRequested"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSSelectOnlineDeviceDialogENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    2,   26,    2, 0x06,    1 /* Public */,
       5,    0,   31,    2, 0x06,    4 /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString, QMetaType::QString,    3,    4,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject SelectOnlineDeviceDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CLASSSelectOnlineDeviceDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSSelectOnlineDeviceDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSSelectOnlineDeviceDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<SelectOnlineDeviceDialog, std::true_type>,
        // method 'deviceSelected'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        QtPrivate::TypeAndForceComplete<const QString &, std::false_type>,
        // method 'refreshRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void SelectOnlineDeviceDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<SelectOnlineDeviceDialog *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->deviceSelected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->refreshRequested(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (SelectOnlineDeviceDialog::*)(const QString & , const QString & );
            if (_t _q_method = &SelectOnlineDeviceDialog::deviceSelected; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (SelectOnlineDeviceDialog::*)();
            if (_t _q_method = &SelectOnlineDeviceDialog::refreshRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }
}

const QMetaObject *SelectOnlineDeviceDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SelectOnlineDeviceDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSSelectOnlineDeviceDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int SelectOnlineDeviceDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 2;
    }
    return _id;
}

// SIGNAL 0
void SelectOnlineDeviceDialog::deviceSelected(const QString & _t1, const QString & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SelectOnlineDeviceDialog::refreshRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
QT_WARNING_POP
