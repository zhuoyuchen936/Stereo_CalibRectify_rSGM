/****************************************************************************
** Meta object code from reading C++ file 'stereo_rectify.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../stereo_rectify.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'stereo_rectify.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_stereo_rectify_t {
    QByteArrayData data[11];
    char stringdata0[170];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_stereo_rectify_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_stereo_rectify_t qt_meta_stringdata_stereo_rectify = {
    {
QT_MOC_LITERAL(0, 0, 14), // "stereo_rectify"
QT_MOC_LITERAL(1, 15, 20), // "on_LeftImage_clicked"
QT_MOC_LITERAL(2, 36, 0), // ""
QT_MOC_LITERAL(3, 37, 16), // "on_Calib_clicked"
QT_MOC_LITERAL(4, 54, 17), // "on_Import_clicked"
QT_MOC_LITERAL(5, 72, 20), // "on_TestImage_clicked"
QT_MOC_LITERAL(6, 93, 18), // "on_Rectify_clicked"
QT_MOC_LITERAL(7, 112, 21), // "on_showRectify_cliked"
QT_MOC_LITERAL(8, 134, 11), // "param_fresh"
QT_MOC_LITERAL(9, 146, 14), // "updateProgress"
QT_MOC_LITERAL(10, 161, 8) // "progress"

    },
    "stereo_rectify\0on_LeftImage_clicked\0"
    "\0on_Calib_clicked\0on_Import_clicked\0"
    "on_TestImage_clicked\0on_Rectify_clicked\0"
    "on_showRectify_cliked\0param_fresh\0"
    "updateProgress\0progress"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_stereo_rectify[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       8,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   54,    2, 0x08 /* Private */,
       3,    0,   55,    2, 0x08 /* Private */,
       4,    0,   56,    2, 0x08 /* Private */,
       5,    0,   57,    2, 0x08 /* Private */,
       6,    0,   58,    2, 0x08 /* Private */,
       7,    0,   59,    2, 0x08 /* Private */,
       8,    0,   60,    2, 0x08 /* Private */,
       9,    1,   61,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Float,   10,

       0        // eod
};

void stereo_rectify::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<stereo_rectify *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->on_LeftImage_clicked(); break;
        case 1: _t->on_Calib_clicked(); break;
        case 2: _t->on_Import_clicked(); break;
        case 3: _t->on_TestImage_clicked(); break;
        case 4: _t->on_Rectify_clicked(); break;
        case 5: _t->on_showRectify_cliked(); break;
        case 6: _t->param_fresh(); break;
        case 7: _t->updateProgress((*reinterpret_cast< float(*)>(_a[1]))); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject stereo_rectify::staticMetaObject = { {
    &QMainWindow::staticMetaObject,
    qt_meta_stringdata_stereo_rectify.data,
    qt_meta_data_stereo_rectify,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *stereo_rectify::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *stereo_rectify::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_stereo_rectify.stringdata0))
        return static_cast<void*>(this);
    return QMainWindow::qt_metacast(_clname);
}

int stereo_rectify::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 8)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 8;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 8)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 8;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
