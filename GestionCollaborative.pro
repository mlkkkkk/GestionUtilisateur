#-------------------------------------------------
# GestionCollaborative - Qt Project File
#-------------------------------------------------

QT += core gui widgets sql network printsupport charts
CONFIG += c++17

TARGET   = GestionCollaborative
TEMPLATE = app

# ---- Sources ----
SOURCES += \
    main.cpp \
    connection/connection.cpp \
    utulisateur/loginwindow.cpp \
    utulisateur/userwindow.cpp \
    matriel/matriele.cpp \
    matriel/qrcodegen.cpp

# ---- Headers ----
HEADERS += \
    connection/connection.h \
    utulisateur/loginwindow.h \
    utulisateur/userwindow.h \
    matriel/matriele.h \
    matriel/qrcodegen.hpp

# ---- Formulaires UI ----
FORMS += \
    userwindow.ui \
    matriel/matriele.ui

# ---- Répertoires d'inclusion ----
INCLUDEPATH += \
    connection \
    utulisateur \
    matriel
