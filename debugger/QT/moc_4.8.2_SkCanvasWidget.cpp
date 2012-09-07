/****************************************************************************
** Meta object code from reading C++ file 'SkCanvasWidget.h'
**
** Created: Thu Sep 6 11:16:07 2012
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "SkCanvasWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SkCanvasWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SkCanvasWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: signature, parameters, type, tag, flags
      31,   16,   15,   15, 0x05,
      68,   57,   15,   15, 0x05,
      92,   88,   15,   15, 0x05,

 // slots: signature, parameters, type, tag, flags
     122,  108,   15,   15, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SkCanvasWidget[] = {
    "SkCanvasWidget\0\0newScaleFactor\0"
    "scaleFactorChanged(float)\0newCommand\0"
    "commandChanged(int)\0hit\0hitChanged(int)\0"
    "zoomIncrement\0keyZoom(int)\0"
};

void SkCanvasWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SkCanvasWidget *_t = static_cast<SkCanvasWidget *>(_o);
        switch (_id) {
        case 0: _t->scaleFactorChanged((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 1: _t->commandChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->hitChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 3: _t->keyZoom((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData SkCanvasWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall
};

const QMetaObject SkCanvasWidget::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_SkCanvasWidget,
      qt_meta_data_SkCanvasWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SkCanvasWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SkCanvasWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SkCanvasWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SkCanvasWidget))
        return static_cast<void*>(const_cast< SkCanvasWidget*>(this));
    return QWidget::qt_metacast(_clname);
}

int SkCanvasWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void SkCanvasWidget::scaleFactorChanged(float _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void SkCanvasWidget::commandChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void SkCanvasWidget::hitChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}
QT_END_MOC_NAMESPACE
