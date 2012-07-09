/****************************************************************************
** Meta object code from reading C++ file 'SkDebuggerGUI.h'
**
** Created: Mon Jul 9 13:45:07 2012
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
      21,   14, // methods
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
     100,   14,   14,   14, 0x08,
     114,   14,   14,   14, 0x08,
     129,   14,   14,   14, 0x08,
     147,   14,   14,   14, 0x08,
     160,   14,   14,   14, 0x08,
     175,   14,   14,   14, 0x08,
     190,   14,   14,   14, 0x08,
     207,   14,   14,   14, 0x08,
     224,   14,   14,   14, 0x08,
     249,  244,   14,   14, 0x08,
     276,   14,   14,   14, 0x08,
     287,  244,   14,   14, 0x08,
     335,  323,   14,   14, 0x08,
     354,   14,   14,   14, 0x08,
     373,   14,   14,   14, 0x08,
     398,  391,   14,   14, 0x08,
     426,  420,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SkDebuggerGUI[] = {
    "SkDebuggerGUI\0\0command\0commandChanged(int)\0"
    "actionBreakpoints()\0actionCancel()\0"
    "actionCommandFilter()\0actionClose()\0"
    "actionDelete()\0actionInspector()\0"
    "actionPlay()\0actionReload()\0actionRewind()\0"
    "actionSettings()\0actionStepBack()\0"
    "actionStepForward()\0item\0"
    "loadFile(QListWidgetItem*)\0openFile()\0"
    "registerListClick(QListWidgetItem*)\0"
    "scaleFactor\0actionScale(float)\0"
    "toggleBreakpoint()\0toggleDirectory()\0"
    "string\0toggleFilter(QString)\0state\0"
    "pauseDrawing(int)\0"
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
        case 3: actionCommandFilter(); break;
        case 4: actionClose(); break;
        case 5: actionDelete(); break;
        case 6: actionInspector(); break;
        case 7: actionPlay(); break;
        case 8: actionReload(); break;
        case 9: actionRewind(); break;
        case 10: actionSettings(); break;
        case 11: actionStepBack(); break;
        case 12: actionStepForward(); break;
        case 13: loadFile((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 14: openFile(); break;
        case 15: registerListClick((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 16: actionScale((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 17: toggleBreakpoint(); break;
        case 18: toggleDirectory(); break;
        case 19: toggleFilter((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 20: pauseDrawing((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
        _id -= 21;
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
