/****************************************************************************
** Meta object code from reading C++ file 'SkDebuggerGUI.h'
**
** Created: Mon Jul 16 16:36:08 2012
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
      25,   14, // methods
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
     175,   14,   14,   14, 0x08,
     193,   14,   14,   14, 0x08,
     206,   14,   14,   14, 0x08,
     233,  221,   14,   14, 0x08,
     252,   14,   14,   14, 0x08,
     269,   14,   14,   14, 0x08,
     286,   14,   14,   14, 0x08,
     311,  306,   14,   14, 0x08,
     338,   14,   14,   14, 0x08,
     358,  349,   14,   14, 0x08,
     377,   14,   14,   14, 0x28,
     392,  306,   14,   14, 0x08,
     428,   15,   14,   14, 0x08,
     447,   14,   14,   14, 0x08,
     461,   14,   14,   14, 0x08,
     480,   14,   14,   14, 0x08,
     505,  498,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SkDebuggerGUI[] = {
    "SkDebuggerGUI\0\0command\0commandChanged(int)\0"
    "actionBreakpoints()\0actionCancel()\0"
    "actionClearBreakpoints()\0actionClearDeletes()\0"
    "actionCommandFilter()\0actionClose()\0"
    "actionDelete()\0actionInspector()\0"
    "actionPlay()\0actionRewind()\0scaleFactor\0"
    "actionScale(float)\0actionSettings()\0"
    "actionStepBack()\0actionStepForward()\0"
    "item\0loadFile(QListWidgetItem*)\0"
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
        case 8: actionInspector(); break;
        case 9: actionPlay(); break;
        case 10: actionRewind(); break;
        case 11: actionScale((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 12: actionSettings(); break;
        case 13: actionStepBack(); break;
        case 14: actionStepForward(); break;
        case 15: loadFile((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 16: openFile(); break;
        case 17: pauseDrawing((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 18: pauseDrawing(); break;
        case 19: registerListClick((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 20: selectCommand((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 21: showDeletes(); break;
        case 22: toggleBreakpoint(); break;
        case 23: toggleDirectory(); break;
        case 24: toggleFilter((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 25;
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
