/****************************************************************************
** Meta object code from reading C++ file 'SkDebuggerGUI.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#ifndef NOMINMAX
#define NOMINMAX
#endif

#define QT_NO_KEYWORDS 1

#include "SkDebuggerGUI.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SkDebuggerGUI.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_SkDebuggerGUI[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      33,   14, // methods
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
     138,   14,   14,   14, 0x08,
     153,   14,   14,   14, 0x08,
     171,   14,   14,   14, 0x08,
     184,   14,   14,   14, 0x08,
     214,   14,   14,   14, 0x08,
     244,   14,   14,   14, 0x08,
     266,   14,   14,   14, 0x08,
     281,   14,   14,   14, 0x08,
     294,   14,   14,   14, 0x08,
     321,  309,   14,   14, 0x08,
     340,   14,   14,   14, 0x08,
     357,   14,   14,   14, 0x08,
     374,   14,   14,   14, 0x08,
     394,   14,   14,   14, 0x08,
     414,  409,   14,   14, 0x08,
     441,   14,   14,   14, 0x08,
     461,  452,   14,   14, 0x08,
     480,   14,   14,   14, 0x28,
     495,   14,   14,   14, 0x08,
     519,   15,   14,   14, 0x08,
     538,   14,   14,   14, 0x08,
     552,   14,   14,   14, 0x08,
     571,   14,   14,   14, 0x08,
     589,   14,   14,   14, 0x08,
     622,  615,   14,   14, 0x08,
     651,  644,   14,   14, 0x08,
     666,   14,   14,   14, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_SkDebuggerGUI[] = {
    "SkDebuggerGUI\0\0command\0commandChanged(int)\0"
    "actionBreakpoints()\0actionCancel()\0"
    "actionClearBreakpoints()\0actionClearDeletes()\0"
    "actionClose()\0actionDelete()\0"
    "actionInspector()\0actionPlay()\0"
    "actionRasterSettingsChanged()\0"
    "actionVisualizationsChanged()\0"
    "actionTextureFilter()\0actionRewind()\0"
    "actionSave()\0actionSaveAs()\0scaleFactor\0"
    "actionScale(float)\0actionSettings()\0"
    "actionStepBack()\0actionStepForward()\0"
    "drawComplete()\0item\0loadFile(QListWidgetItem*)\0"
    "openFile()\0isPaused\0pauseDrawing(bool)\0"
    "pauseDrawing()\0updateDrawCommandInfo()\0"
    "selectCommand(int)\0showDeletes()\0"
    "toggleBreakpoint()\0toggleDirectory()\0"
    "populateDirectoryWidget()\0string\0"
    "toggleFilter(QString)\0newHit\0"
    "updateHit(int)\0updateImage()\0"
};

void SkDebuggerGUI::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        SkDebuggerGUI *_t = static_cast<SkDebuggerGUI *>(_o);
        switch (_id) {
        case 0: _t->commandChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->actionBreakpoints(); break;
        case 2: _t->actionCancel(); break;
        case 3: _t->actionClearBreakpoints(); break;
        case 4: _t->actionClearDeletes(); break;
        case 5: _t->actionClose(); break;
        case 6: _t->actionDelete(); break;
        case 7: _t->actionInspector(); break;
        case 8: _t->actionPlay(); break;
        case 9: _t->actionRasterSettingsChanged(); break;
        case 10: _t->actionVisualizationsChanged(); break;
        case 11: _t->actionTextureFilter(); break;
        case 12: _t->actionRewind(); break;
        case 13: _t->actionSave(); break;
        case 14: _t->actionSaveAs(); break;
        case 15: _t->actionScale((*reinterpret_cast< float(*)>(_a[1]))); break;
        case 16: _t->actionSettings(); break;
        case 17: _t->actionStepBack(); break;
        case 18: _t->actionStepForward(); break;
        case 19: _t->drawComplete(); break;
        case 20: _t->loadFile((*reinterpret_cast< QListWidgetItem*(*)>(_a[1]))); break;
        case 21: _t->openFile(); break;
        case 22: _t->pauseDrawing((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 23: _t->pauseDrawing(); break;
        case 24: _t->updateDrawCommandInfo(); break;
        case 25: _t->selectCommand((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 26: _t->showDeletes(); break;
        case 27: _t->toggleBreakpoint(); break;
        case 28: _t->toggleDirectory(); break;
        case 29: _t->populateDirectoryWidget(); break;
        case 30: _t->toggleFilter((*reinterpret_cast< QString(*)>(_a[1]))); break;
        case 31: _t->updateHit((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 32: _t->updateImage(); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData SkDebuggerGUI::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject SkDebuggerGUI::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_SkDebuggerGUI,
      qt_meta_data_SkDebuggerGUI, &staticMetaObjectExtraData }
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
        if (_id < 33)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 33;
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
