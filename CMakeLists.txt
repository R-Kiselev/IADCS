cmake_minimum_required(VERSION 3.22)
project(InterfacesAndDevicesOfPC)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Укажите путь к вашей установке Qt (более короткий путь, как в примере)
set(CMAKE_PREFIX_PATH "C:/Qt/6.9.2/mingw_64")

# Находим все необходимые компоненты Qt
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets)

# Создаем манифест приложения для запроса прав администратора (взято из вашего примера)
if(WIN32)
    file(WRITE ${CMAKE_BINARY_DIR}/app.manifest
            "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>
<assembly xmlns=\"urn:schemas-microsoft-com:asm.v1\" manifestVersion=\"1.0\">
  <assemblyIdentity
    version=\"1.0.0.0\"
    processorArchitecture=\"*\"
    name=\"${PROJECT_NAME}\"
    type=\"win32\"
  />
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

# Определяем исполняемый файл и все его исходники
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
)

# Добавляем манифест к исполняемому файлу
if(WIN32)
    target_sources(InterfacesAndDevicesOfPC PRIVATE ${CMAKE_BINARY_DIR}/app.manifest)
endif()

# Связываем исполняемый файл со всеми необходимыми библиотеками
target_link_libraries(InterfacesAndDevicesOfPC PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Multimedia
        Qt6::MultimediaWidgets
        # Системные библиотеки для лабораторных работ
        powrprof
        setupapi
        cfgmgr32
)

# Устанавливаем подсистему Windows для скрытия консоли
if(WIN32)
    set_target_properties(InterfacesAndDevicesOfPC PROPERTIES WIN32_EXECUTABLE ON)
endif()

# [ГЛАВНОЕ ИЗМЕНЕНИЕ]
# Автоматически копируем все зависимости Qt (DLL, плагины и т.д.) после сборки
# с помощью утилиты windeployqt. Это заменяет старый ручной метод.
add_custom_command(TARGET InterfacesAndDevicesOfPC POST_BUILD
        COMMAND "${CMAKE_PREFIX_PATH}/bin/windeployqt.exe"
        # Указываем путь к нашему exe-файлу
        "$<TARGET_FILE:InterfacesAndDevicesOfPC>"
        # Указываем рабочую директорию, чтобы все файлы скопировались куда нужно
        WORKING_DIRECTORY "$<TARGET_FILE_DIR:InterfacesAndDevicesOfPC>"
        COMMENT "Running windeployqt to deploy dependencies..."
)