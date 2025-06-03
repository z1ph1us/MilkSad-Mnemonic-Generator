# Define the application target name
TARGET = milk_sad_generator_gui

# Specify the template for the project (e.g., app for application)
TEMPLATE = app

# Specify the source files
SOURCES += milk_sad_generator_gui.cpp \
           WorkerThread.cpp \
           MainWindow.cpp

# Specify the header files (important for moc to find Q_OBJECT classes)
HEADERS += WorkerThread.h \
           MainWindow.h \
           constants.h \
           helpers.h

# Specify the Qt modules your project needs
QT += core gui widgets

# If you're using C++11, C++14, etc.
CONFIG += c++11

# Include headers from the current directory
INCLUDEPATH += .

# Add Pic folder to resources if you have images there
# You will need to create a .qrc file if you want to embed resources like Pic/1_0LOPQwRdahE_ABkF8idXgg.png
# For example, create 'resources.qrc' with:
# <RCC>
#     <qresource prefix="/">
#         <file>Pic/1_0LOPQwRdahE_ABkF8idXgg.png</file>
#     </qresource>
# </RCC>
# Then uncomment the following line:
# RESOURCES += resources.qrc

# For OpenSSL libraries (if you are compiling with OpenSSL)
LIBS += -lcrypto -lssl