/****************************************************************************
** Meta object code from reading C++ file 'SkDebuggerGUI.h'
**
** Created: Wed Aug 1 14:54:26 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "SkDebuggerGUI.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SkDebuggerGUI.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SkDebuggerGUI[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
      30,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      23,   15,   14,   14, 0x05,

 // slots: signature, parameters, type, tag, flags
      43,   14,   14,   14, 0x08,
      63,   14,   14,   14, 0x08,
      78,   14,   14,   14, 0x08,
     103,   14,   14,   14, 0x08,
     124,   14,   14,   14, 0x08,
     146,   14,   14,   14, 0x08,
     160,   14,   14,   14, 0x08,
     185,  175,   14,   14, 0x08,
     206,   14,   14,   14, 0x08,
     224,   14,   14,   14, 0x08,
     237,  175,   14,   14, 0x08,
     262,   14,   14,   14, 0x08,
     277,   14,   14,   14, 0x08,
     290,   14,   14,   14, 0x08,
     317,  305,   14,   14, 0x08,
     336,   14,   14,   14, 0x08,
     353,   14,   14,   14, 0x08,
     370,   14,   14,   14, 0x08,
     390,   14,   14,   14, 0x08,
     410,  405,   14,   14, 0x08,
     437,   14,   14,   14, 0x08,
     457,  448,   14,   14, 0x08,
     476,   14,   14,   14, 0x28,
     491,  405,   14,   14, 0x08,
     527,   15,   14,   14, 0x08,
     546,   14,   14,   14, 0x08,
     560,   14,   14,   14, 0x08,
     579,   14,   14,   14, 0x08,
     604,  597,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SkDebuggerGUI[] = {
    "SkDebuggerGUI\0\0command\0commandChanged(int)\0"
    "actionBreakpoints()\0actionCancel()\0"
    "actionClearBreakpoints()\0actionClearDeletes()\0"
    "actionCommandFilter()\0actionClose()\0"
    "actionDelete()\0isToggled\0actionGLWidget(bool)\0"
    "actionInspector()\0actionPlay()\0"
    "actionRasterWidget(bool)\0actionRewind()\0"
    "actionSave()\0actionSaveAs()\0scaleFactor\0"
    "actionScale(float)\0actionSettings()\0"
    "actionStepBack()\0actionStepForward()\0"
    "drawComplete()\0item\0loadFile(QListWidgetItem*)\0"
    "openFile()\0isPaused\0pauseDrawing(bool)\0"
    "pauseDrawing()\0registerListClick(QListWidgetItem*)\0"
    "selectCommand(int)\0showDeletes()\0"
    "toggleBreakpoint()\0toggleDirectory()\0"
    "string\0toggleFilter(QString)\0"
};

const QMetaObject SkDebuggerGUI::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_SkDebuggerGUI,
      qt_meta_data_SkDebuggerGUI, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &SkDebuggerGUI::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *SkDebuggerGUI::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *SkDebuggerGUI::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_SkDebuggerGUI))
        return static_cast<void*>(const_cast< SkDebuggerGUI*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int SkDebuggerGUI::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: commandChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: actionBreakpoints(); break;
        case 2: actionCancel(); break;
        case 3: actionClearBreakpoints(); break;
        case 4: actionClearDeletes(); break;
        case 5: actionCommandFilter(); break;
        case 6: actionClose(); break;
        case 7: actionDelete(); break;
        case 8: actionGLWidget((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 9: actionInspector(); break;
        case 10: actionPlay(); break;
        case 11: actionRasterWidget((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 12: actionRewind(); break;
        case 13: actionSave(); break;
        case 14: actionSaveAs(); break;
        case 15: actionScale((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 16: actionSettings(); break;
        case 17: actionStepBack(); break;
        case 18: actionStepForward(); break;
        case 19: drawComplete(); break;
        case 20: loadFile((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 21: openFile(); break;
        case 22: pauseDrawing((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 23: pauseDrawing(); break;
        case 24: registerListClick((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 25: selectCommand((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 26: showDeletes(); break;
        case 27: toggleBreakpoint(); break;
        case 28: toggleDirectory(); break;
        case 29: toggleFilter((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 30;
    }
    return _id;
}

// SIGNAL 0
void SkDebuggerGUI::commandChanged(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
