/****************************************************************************
** Meta object code from reading C++ file 'SkDebuggerGUI.h'
**
** Created: Thu Jun 28 17:18:47 2012
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
      18,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      15,   14,   14,   14, 0x08,
      35,   14,   14,   14, 0x08,
      50,   14,   14,   14, 0x08,
      72,   14,   14,   14, 0x08,
      86,   14,   14,   14, 0x08,
     101,   14,   14,   14, 0x08,
     119,   14,   14,   14, 0x08,
     132,   14,   14,   14, 0x08,
     147,   14,   14,   14, 0x08,
     162,   14,   14,   14, 0x08,
     179,   14,   14,   14, 0x08,
     196,   14,   14,   14, 0x08,
     221,  216,   14,   14, 0x08,
     248,   14,   14,   14, 0x08,
     259,  216,   14,   14, 0x08,
     295,   14,   14,   14, 0x08,
     314,   14,   14,   14, 0x08,
     339,  332,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SkDebuggerGUI[] = {
    "SkDebuggerGUI\0\0actionBreakpoints()\0"
    "actionCancel()\0actionCommandFilter()\0"
    "actionClose()\0actionDelete()\0"
    "actionInspector()\0actionPlay()\0"
    "actionReload()\0actionRewind()\0"
    "actionSettings()\0actionStepBack()\0"
    "actionStepForward()\0item\0"
    "loadFile(QListWidgetItem*)\0openFile()\0"
    "registerListClick(QListWidgetItem*)\0"
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
        case 0: actionBreakpoints(); break;
        case 1: actionCancel(); break;
        case 2: actionCommandFilter(); break;
        case 3: actionClose(); break;
        case 4: actionDelete(); break;
        case 5: actionInspector(); break;
        case 6: actionPlay(); break;
        case 7: actionReload(); break;
        case 8: actionRewind(); break;
        case 9: actionSettings(); break;
        case 10: actionStepBack(); break;
        case 11: actionStepForward(); break;
        case 12: loadFile((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 13: openFile(); break;
        case 14: registerListClick((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 15: toggleBreakpoint(); break;
        case 16: toggleDirectory(); break;
        case 17: toggleFilter((*reinterpret_cast< QString(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 18;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
