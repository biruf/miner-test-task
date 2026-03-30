/****************************************************************************
** Meta object code from reading C++ file 'mineclient.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "mineclient.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'mineclient.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_MineClient_t {
    QByteArrayData data[22];
    char stringdata0[279];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_MineClient_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_MineClient_t qt_meta_stringdata_MineClient = {
    {
QT_MOC_LITERAL(0, 0, 10), // "MineClient"
QT_MOC_LITERAL(1, 11, 9), // "connected"
QT_MOC_LITERAL(2, 21, 0), // ""
QT_MOC_LITERAL(3, 22, 12), // "disconnected"
QT_MOC_LITERAL(4, 35, 14), // "statusReceived"
QT_MOC_LITERAL(5, 50, 10), // "totalMetal"
QT_MOC_LITERAL(6, 61, 11), // "minersCount"
QT_MOC_LITERAL(7, 73, 21), // "journalEventsReceived"
QT_MOC_LITERAL(8, 95, 6), // "events"
QT_MOC_LITERAL(9, 102, 13), // "errorReceived"
QT_MOC_LITERAL(10, 116, 5), // "error"
QT_MOC_LITERAL(11, 122, 14), // "sendStartMiner"
QT_MOC_LITERAL(12, 137, 13), // "sendGetStatus"
QT_MOC_LITERAL(13, 151, 17), // "sendSetLoggerType"
QT_MOC_LITERAL(14, 169, 4), // "type"
QT_MOC_LITERAL(15, 174, 11), // "onConnected"
QT_MOC_LITERAL(16, 186, 14), // "onDisconnected"
QT_MOC_LITERAL(17, 201, 11), // "onReadyRead"
QT_MOC_LITERAL(18, 213, 7), // "onError"
QT_MOC_LITERAL(19, 221, 28), // "QAbstractSocket::SocketError"
QT_MOC_LITERAL(20, 250, 11), // "socketError"
QT_MOC_LITERAL(21, 262, 16) // "attemptReconnect"

    },
    "MineClient\0connected\0\0disconnected\0"
    "statusReceived\0totalMetal\0minersCount\0"
    "journalEventsReceived\0events\0errorReceived\0"
    "error\0sendStartMiner\0sendGetStatus\0"
    "sendSetLoggerType\0type\0onConnected\0"
    "onDisconnected\0onReadyRead\0onError\0"
    "QAbstractSocket::SocketError\0socketError\0"
    "attemptReconnect"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MineClient[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
      13,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       5,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    0,   79,    2, 0x06 /* Public */,
       3,    0,   80,    2, 0x06 /* Public */,
       4,    2,   81,    2, 0x06 /* Public */,
       7,    1,   86,    2, 0x06 /* Public */,
       9,    1,   89,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    0,   92,    2, 0x0a /* Public */,
      12,    0,   93,    2, 0x0a /* Public */,
      13,    1,   94,    2, 0x0a /* Public */,
      15,    0,   97,    2, 0x08 /* Private */,
      16,    0,   98,    2, 0x08 /* Private */,
      17,    0,   99,    2, 0x08 /* Private */,
      18,    1,  100,    2, 0x08 /* Private */,
      21,    0,  103,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    5,    6,
    QMetaType::Void, QMetaType::QJsonArray,    8,
    QMetaType::Void, QMetaType::QString,   10,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::QString,   14,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, 0x80000000 | 19,   20,
    QMetaType::Void,

       0        // eod
};

void MineClient::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<MineClient *>(_o);
        (void)_t;
        switch (_id) {
        case 0: _t->connected(); break;
        case 1: _t->disconnected(); break;
        case 2: _t->statusReceived((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 3: _t->journalEventsReceived((*reinterpret_cast< const QJsonArray(*)>(_a[1]))); break;
        case 4: _t->errorReceived((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 5: _t->sendStartMiner(); break;
        case 6: _t->sendGetStatus(); break;
        case 7: _t->sendSetLoggerType((*reinterpret_cast< const QString(*)>(_a[1]))); break;
        case 8: _t->onConnected(); break;
        case 9: _t->onDisconnected(); break;
        case 10: _t->onReadyRead(); break;
        case 11: _t->onError((*reinterpret_cast< QAbstractSocket::SocketError(*)>(_a[1]))); break;
        case 12: _t->attemptReconnect(); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 11:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QAbstractSocket::SocketError >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (MineClient::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MineClient::connected)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (MineClient::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MineClient::disconnected)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (MineClient::*)(int , int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MineClient::statusReceived)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (MineClient::*)(const QJsonArray & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MineClient::journalEventsReceived)) {
                *result = 3;
                return;
            }
        }
        {
            using _t = void (MineClient::*)(const QString & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&MineClient::errorReceived)) {
                *result = 4;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject MineClient::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_meta_stringdata_MineClient.data,
    qt_meta_data_MineClient,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *MineClient::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MineClient::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_MineClient.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int MineClient::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 13)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 13;
    }
    return _id;
}

// SIGNAL 0
void MineClient::connected()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void MineClient::disconnected()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void MineClient::statusReceived(int _t1, int _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 2, _a);
}

// SIGNAL 3
void MineClient::journalEventsReceived(const QJsonArray & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}

// SIGNAL 4
void MineClient::errorReceived(const QString & _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 4, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
