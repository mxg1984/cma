/****************************************************************************
** Meta object code from reading C++ file 'tabledialog.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.2.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "tabledialog.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'tabledialog.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.2.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_tableDialog_t {
    QByteArrayData data[12];
    char stringdata[130];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_tableDialog_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_tableDialog_t qt_meta_stringdata_tableDialog = {
    {
QT_MOC_LITERAL(0, 0, 11),
QT_MOC_LITERAL(1, 12, 8),
QT_MOC_LITERAL(2, 21, 0),
QT_MOC_LITERAL(3, 22, 4),
QT_MOC_LITERAL(4, 27, 16),
QT_MOC_LITERAL(5, 44, 9),
QT_MOC_LITERAL(6, 54, 8),
QT_MOC_LITERAL(7, 63, 7),
QT_MOC_LITERAL(8, 71, 8),
QT_MOC_LITERAL(9, 80, 8),
QT_MOC_LITERAL(10, 89, 18),
QT_MOC_LITERAL(11, 108, 20)
    },
    "tableDialog\0sendData\0\0data\0loadTableContent\0"
    "writeFile\0fileName\0content\0isAppend\0"
    "readFile\0on_btnSure_clicked\0"
    "on_btnCancle_clicked\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_tableDialog[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06,

 // slots: name, argc, parameters, tag, flags
       4,    0,   47,    2, 0x08,
       5,    3,   48,    2, 0x08,
       9,    1,   55,    2, 0x08,
      10,    0,   58,    2, 0x08,
      11,    0,   59,    2, 0x08,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString, QMetaType::QString, QMetaType::Bool,    6,    7,    8,
    QMetaType::QString, QMetaType::QString,    6,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void tableDialog::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        tableDialog *_t = static_cast<tableDialog *>(_o);
        switch (_id) {
        case 0: _t->sendData((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 1: _t->loadTableContent(); break;
        case 2: _t->writeFile((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2])),(*reinterpret_cast< bool(*)>(_a[3]))); break;
        case 3: { QString _r = _t->readFile((*reinterpret_cast< QString(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QString*>(_a[0]) = _r; }  break;
        case 4: _t->on_btnSure_clicked(); break;
        case 5: _t->on_btnCancle_clicked(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (tableDialog::*_t)(QString );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&tableDialog::sendData)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject tableDialog::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_tableDialog.data,
      qt_meta_data_tableDialog,  qt_static_metacall, 0, 0}
};


const QMetaObject *tableDialog::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *tableDialog::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_tableDialog.stringdata))
        return static_cast<void*>(const_cast< tableDialog*>(this));
    return QDialog::qt_metacast(_clname);
}

int tableDialog::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void tableDialog::sendData(QString _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
