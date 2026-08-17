/****************************************************************************
** Meta object code from reading C++ file 'VirtualKeyboard.h'
**
** Created by: The Qt Meta Object Compiler version 68 (Qt 6.5.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../OUT_SRC_TUANANH/VirtualKeyboard.h"
#include <QtCore/qmetatype.h>

#if __has_include(<QtCore/qtmochelpers.h>)
#include <QtCore/qtmochelpers.h>
#else
QT_BEGIN_MOC_NAMESPACE
#endif


#include <memory>

#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'VirtualKeyboard.h' doesn't include <QObject>."
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
struct qt_meta_stringdata_CLASSVirtualKeyboardENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSVirtualKeyboardENDCLASS = QtMocHelpers::stringData(
    "VirtualKeyboard",
    "enterPressed",
    "",
    "hideRequested",
    "onKeyClicked",
    "toggleShift",
    "toggleSymbols"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSVirtualKeyboardENDCLASS_t {
    uint offsetsAndSizes[14];
    char stringdata0[16];
    char stringdata1[13];
    char stringdata2[1];
    char stringdata3[14];
    char stringdata4[13];
    char stringdata5[12];
    char stringdata6[14];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSVirtualKeyboardENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSVirtualKeyboardENDCLASS_t qt_meta_stringdata_CLASSVirtualKeyboardENDCLASS = {
    {
        QT_MOC_LITERAL(0, 15),  // "VirtualKeyboard"
        QT_MOC_LITERAL(16, 12),  // "enterPressed"
        QT_MOC_LITERAL(29, 0),  // ""
        QT_MOC_LITERAL(30, 13),  // "hideRequested"
        QT_MOC_LITERAL(44, 12),  // "onKeyClicked"
        QT_MOC_LITERAL(57, 11),  // "toggleShift"
        QT_MOC_LITERAL(69, 13)   // "toggleSymbols"
    },
    "VirtualKeyboard",
    "enterPressed",
    "",
    "hideRequested",
    "onKeyClicked",
    "toggleShift",
    "toggleSymbols"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSVirtualKeyboardENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags, initial metatype offsets
       1,    0,   44,    2, 0x06,    1 /* Public */,
       3,    0,   45,    2, 0x06,    2 /* Public */,

 // slots: name, argc, parameters, tag, flags, initial metatype offsets
       4,    0,   46,    2, 0x08,    3 /* Private */,
       5,    0,   47,    2, 0x08,    4 /* Private */,
       6,    0,   48,    2, 0x08,    5 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

Q_CONSTINIT const QMetaObject VirtualKeyboard::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_meta_stringdata_CLASSVirtualKeyboardENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSVirtualKeyboardENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSVirtualKeyboardENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<VirtualKeyboard, std::true_type>,
        // method 'enterPressed'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'hideRequested'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'onKeyClicked'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'toggleShift'
        QtPrivate::TypeAndForceComplete<void, std::false_type>,
        // method 'toggleSymbols'
        QtPrivate::TypeAndForceComplete<void, std::false_type>
    >,
    nullptr
} };

void VirtualKeyboard::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<VirtualKeyboard *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->enterPressed(); break;
        case 1: _t->hideRequested(); break;
        case 2: _t->onKeyClicked(); break;
        case 3: _t->toggleShift(); break;
        case 4: _t->toggleSymbols(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (VirtualKeyboard::*)();
            if (_t _q_method = &VirtualKeyboard::enterPressed; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (VirtualKeyboard::*)();
            if (_t _q_method = &VirtualKeyboard::hideRequested; *reinterpret_cast<_t *>(_a[1]) == _q_method) {
                *result = 1;
                return;
            }
        }
    }
    (void)_a;
}

const QMetaObject *VirtualKeyboard::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VirtualKeyboard::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSVirtualKeyboardENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int VirtualKeyboard::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 5;
    }
    return _id;
}

// SIGNAL 0
void VirtualKeyboard::enterPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void VirtualKeyboard::hideRequested()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}
namespace {

#ifdef QT_MOC_HAS_STRINGDATA
struct qt_meta_stringdata_CLASSVirtualKeyboardDialogENDCLASS_t {};
static constexpr auto qt_meta_stringdata_CLASSVirtualKeyboardDialogENDCLASS = QtMocHelpers::stringData(
    "VirtualKeyboardDialog"
);
#else  // !QT_MOC_HAS_STRING_DATA
struct qt_meta_stringdata_CLASSVirtualKeyboardDialogENDCLASS_t {
    uint offsetsAndSizes[2];
    char stringdata0[22];
};
#define QT_MOC_LITERAL(ofs, len) \
    uint(sizeof(qt_meta_stringdata_CLASSVirtualKeyboardDialogENDCLASS_t::offsetsAndSizes) + ofs), len 
Q_CONSTINIT static const qt_meta_stringdata_CLASSVirtualKeyboardDialogENDCLASS_t qt_meta_stringdata_CLASSVirtualKeyboardDialogENDCLASS = {
    {
        QT_MOC_LITERAL(0, 21)   // "VirtualKeyboardDialog"
    },
    "VirtualKeyboardDialog"
};
#undef QT_MOC_LITERAL
#endif // !QT_MOC_HAS_STRING_DATA
} // unnamed namespace

Q_CONSTINIT static const uint qt_meta_data_CLASSVirtualKeyboardDialogENDCLASS[] = {

 // content:
      11,       // revision
       0,       // classname
       0,    0, // classinfo
       0,    0, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

       0        // eod
};

Q_CONSTINIT const QMetaObject VirtualKeyboardDialog::staticMetaObject = { {
    QMetaObject::SuperData::link<QDialog::staticMetaObject>(),
    qt_meta_stringdata_CLASSVirtualKeyboardDialogENDCLASS.offsetsAndSizes,
    qt_meta_data_CLASSVirtualKeyboardDialogENDCLASS,
    qt_static_metacall,
    nullptr,
    qt_incomplete_metaTypeArray<qt_meta_stringdata_CLASSVirtualKeyboardDialogENDCLASS_t,
        // Q_OBJECT / Q_GADGET
        QtPrivate::TypeAndForceComplete<VirtualKeyboardDialog, std::true_type>
    >,
    nullptr
} };

void VirtualKeyboardDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    (void)_o;
    (void)_id;
    (void)_c;
    (void)_a;
}

const QMetaObject *VirtualKeyboardDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *VirtualKeyboardDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_CLASSVirtualKeyboardDialogENDCLASS.stringdata0))
        return static_cast<void*>(this);
    return QDialog::qt_metacast(_clname);
}

int VirtualKeyboardDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    return _id;
}
QT_WARNING_POP
