/****************************************************************************
** Meta object code from reading C++ file 'inputdialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "inputdialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'inputdialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_inputDialog_t {
    QByteArrayData data[16];
    char stringdata[156];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_inputDialog_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_inputDialog_t qt_meta_stringdata_inputDialog = {
    {
QT_MOC_LITERAL(0, 0, 11),
QT_MOC_LITERAL(1, 12, 8),
QT_MOC_LITERAL(2, 21, 0),
QT_MOC_LITERAL(3, 22, 4),
QT_MOC_LITERAL(4, 27, 8),
QT_MOC_LITERAL(5, 36, 11),
QT_MOC_LITERAL(6, 48, 9),
QT_MOC_LITERAL(7, 58, 21),
QT_MOC_LITERAL(8, 80, 8),
QT_MOC_LITERAL(9, 89, 10),
QT_MOC_LITERAL(10, 100, 9),
QT_MOC_LITERAL(11, 110, 8),
QT_MOC_LITERAL(12, 119, 7),
QT_MOC_LITERAL(13, 127, 8),
QT_MOC_LITERAL(14, 136, 8),
QT_MOC_LITERAL(15, 145, 9)
    },
    "inputDialog\0sendData\0\0data\0SaveData\0"
    "ColumnParse\0coulumnID\0receiveMainWindowData\0"
    "LoadView\0CancelData\0writeFile\0fileName\0"
    "content\0isAppend\0readFile\0chooseDir\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_inputDialog[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   59,    2, 0x06,

 // slots: name, argc, parameters, tag, flags
       4,    0,   62,    2, 0x08,
       5,    1,   63,    2, 0x08,
       7,    1,   66,    2, 0x08,
       8,    1,   69,    2, 0x08,
       9,    0,   72,    2, 0x08,
      10,    3,   73,    2, 0x08,
      14,    1,   80,    2, 0x08,
      15,    0,   83,    2, 0x08,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void,
    QMetaType::QString, QMetaType::Int,    6,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void, QMetaType::QString,    2,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::Bool,   11,   12,   13,
    QMetaType::QString, QMetaType::QString,   11,
    QMetaType::Void,

       0        // eod
};

void inputDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        inputDialog *_t = static_cast<inputDialog *>(_o);
        switch (_id) {
        case 0: _t->sendData((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->SaveData(); break;
        case 2: { QString _r = _t->ColumnParse((*reinterpret_cast< int(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 3: _t->receiveMainWindowData((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 4: _t->LoadView((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 5: _t->CancelData(); break;
        case 6: _t->writeFile((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 7: { QString _r = _t->readFile((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 8: _t->chooseDir(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (inputDialog::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&inputDialog::sendData)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject inputDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_inputDialog.data,
      qt_meta_data_inputDialog,  qt_static_metacall, 0, 0}
};


const QMetaObject *inputDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *inputDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_inputDialog.stringdata))
        return static_cast<void*>(const_cast< inputDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int inputDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 9)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 9)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 9;
    }
    return _id;
}

// SIGNAL 0
void inputDialog::sendData(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
