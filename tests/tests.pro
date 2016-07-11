
TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

QMAKE_CXXFLAGS += \
	-std=c++11

SOURCES += \
	main.cpp \
	classcontract.cpp \
	ctorcontract.cpp \
	derivedcontract.cpp \
	disableinvariants.cpp \
	disablepostconditions.cpp \
	disablepreconditions.cpp \
	dtorcontract.cpp \
	examples.cpp \
	funcontract.cpp \
	loopcontract.cpp \
	mfuncontract.cpp \
	violationhandler.cpp

HEADERS += \
	contract_error.hpp

INCLUDEPATH += \
	../include

LIBS += \
	-lboost_unit_test_framework
