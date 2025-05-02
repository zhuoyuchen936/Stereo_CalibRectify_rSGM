/****************************************************************************
** Meta object code from reading C++ file 'Worker.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../Worker.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Worker.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_StereoCalibrationWorker_t {
    QByteArrayData data[13];
    char stringdata0[134];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_StereoCalibrationWorker_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_StereoCalibrationWorker_t qt_meta_stringdata_StereoCalibrationWorker = {
    {
QT_MOC_LITERAL(0, 0, 23), // "StereoCalibrationWorker"
QT_MOC_LITERAL(1, 24, 10), // "updateText"
QT_MOC_LITERAL(2, 35, 0), // ""
QT_MOC_LITERAL(3, 36, 4), // "text"
QT_MOC_LITERAL(4, 41, 11), // "param_fresh"
QT_MOC_LITERAL(5, 53, 12), // "workFinished"
QT_MOC_LITERAL(6, 66, 14), // "updateProgress"
QT_MOC_LITERAL(7, 81, 8), // "progress"
QT_MOC_LITERAL(8, 90, 11), // "updateImage"
QT_MOC_LITERAL(9, 102, 8), // "cv::Mat&"
QT_MOC_LITERAL(10, 111, 7), // "images1"
QT_MOC_LITERAL(11, 119, 7), // "images2"
QT_MOC_LITERAL(12, 127, 6) // "doWork"

    },
    "StereoCalibrationWorker\0updateText\0\0"
    "text\0param_fresh\0workFinished\0"
    "updateProgress\0progress\0updateImage\0"
    "cv::Mat&\0images1\0images2\0doWork"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_StereoCalibrationWorker[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       4,    0,   47,    2, 0x06 /* Public */,
       5,    0,   48,    2, 0x06 /* Public */,
       6,    1,   49,    2, 0x06 /* Public */,
       8,    2,   52,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      12,    0,   57,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::QString,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Float,    7,
    QMetaType::Void, 0x80000000 | 9, 0x80000000 | 9,   10,   11,

 // slots: parameters
    QMetaType::Void,

       0        // eod
};

void StereoCalibrationWorker::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<StereoCalibrationWorker *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->updateText((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 1: _t->param_fresh(); break;
        case 2: _t->workFinished(); break;
        case 3: _t->updateProgress((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 4: _t->updateImage((*reinterpret_cast< cv::Mat(*)>(_a[1])),(*reinterpret_cast< cv::Mat(*)>(_a[2]))); break;
        case 5: _t->doWork(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (StereoCalibrationWorker::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StereoCalibrationWorker::updateText)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (StereoCalibrationWorker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StereoCalibrationWorker::param_fresh)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (StereoCalibrationWorker::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StereoCalibrationWorker::workFinished)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (StereoCalibrationWorker::*)(float );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StereoCalibrationWorker::updateProgress)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (StereoCalibrationWorker::*)(cv::Mat & , cv::Mat & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&StereoCalibrationWorker::updateImage)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject StereoCalibrationWorker::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_StereoCalibrationWorker.data,
    qt_meta_data_StereoCalibrationWorker,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *StereoCalibrationWorker::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *StereoCalibrationWorker::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_StereoCalibrationWorker.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int StereoCalibrationWorker::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
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
void StereoCalibrationWorker::updateText(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void StereoCalibrationWorker::param_fresh()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void StereoCalibrationWorker::workFinished()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void StereoCalibrationWorker::updateProgress(float _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void StereoCalibrationWorker::updateImage(cv::Mat & _t1, cv::Mat & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(&_t1)), const_cast<void*>(reinterpret_cast<const void*>(&_t2)) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
