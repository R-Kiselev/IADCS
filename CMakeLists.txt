cmake_minimum_required(VERSION 3.22)
project(InterfacesAndDevicesOfPC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- ИЗМЕНЕНИЕ 1: Путь к Qt MSVC ---
# Укажите точный путь, куда установился Qt 6.5.3 MSVC
set(CMAKE_PREFIX_PATH "C:/Qt/6.5.3/msvc2019_64")

# --- ИЗМЕНЕНИЕ 2: Добавлен компонент Bluetooth ---
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets Bluetooth)

if(WIN32)
    # Манифест для прав администратора (нужен для Lab 1-3, 5)
    file(WRITE ${CMAKE_BINARY_DIR}/app.manifest
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>
<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">
  <assemblyIdentity version=\"1.0.0.0\" processorArchitecture=\"*\" name=\"${PROJECT_NAME}\" type=\"win32\"/>
  <description>Interfaces and Devices of PC Labs</description>
  <trustInfo xmlns=\"urn:schemas-microsoft-com:asm.v3\">
    <security>
      <requestedPrivileges>
        <requestedExecutionLevel level=\"requireAdministrator\" uiAccess=\"false\"/>
      </requestedPrivileges>
    </security>
  </trustInfo>
</assembly>")
endif()

add_executable(InterfacesAndDevicesOfPC
        main.cpp
        app.qrc
        MyMainWindow.cpp
        MyMainWindow.h
        labs/lab1/Lab1Widget.cpp
        labs/lab1/Lab1Widget.h
        labs/lab2/Lab2Widget.cpp
        labs/lab2/Lab2Widget.h
        labs/lab3/Lab3Widget.cpp
        labs/lab3/Lab3Widget.h
        labs/lab4/Lab4Widget.cpp
        labs/lab4/Lab4Widget.h
        labs/lab5/Lab5Widget.cpp
        labs/lab5/Lab5Widget.h
        labs/lab6/Lab6Widget.cpp
        labs/lab6/Lab6Widget.h
)

if(WIN32)
    target_sources(InterfacesAndDevicesOfPC PRIVATE ${CMAKE_BINARY_DIR}/app.manifest)
endif()

target_link_libraries(InterfacesAndDevicesOfPC PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::MultimediaWidgets
        Qt6::Bluetooth  # <--- ИЗМЕНЕНИЕ 3: Линковка Qt Bluetooth
        # Системные библиотеки Windows (оставляем для Lab 1-5)
        powrprof
        setupapi
        cfgmgr32
        user32    # Часто нужны для MSVC
        kernel32
        # bthprops # <--- УДАЛЕНО: Больше не нужно, используем Qt
)

if(WIN32)
    set_target_properties(InterfacesAndDevicesOfPC PROPERTIES WIN32_EXECUTABLE ON)
endif()

add_custom_command(TARGET InterfacesAndDevicesOfPC POST_BUILD
        COMMAND "${CMAKE_PREFIX_PATH}/bin/windeployqt.exe"
        "$<TARGET_FILE:InterfacesAndDevicesOfPC>"
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:InterfacesAndDevicesOfPC>"
        COMMENT "Running windeployqt..."
)