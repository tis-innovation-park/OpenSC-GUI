include_directories ("../lib/opensc/include/" "../lib/opensc/include/libopensc" "../lib/opensc/include/common")
set( LIB_DIR ${PROJECT_SOURCE_DIR}/lib ) 

find_package(Qt4 REQUIRED)
include(${QT_USE_FILE})
SET(QT_USE_QTSVG TRUE)

find_package( ZLIB REQUIRED )
include_directories( ${ZLIB_INCLUDE_DIRS} )

find_package(OpenSSL REQUIRED)
include_directories( ${OPENSSL_INCLUDE_DIRS} )


#Adding forms
set( FORMS forms/mainwidget.ui forms/pukdialog.ui forms/aboutdialog.ui) 

#Adding source files
set(SRCS main.cpp)
set(SRCS ${SRCS} mainwidget.cpp cardcontrolhandler.cpp error.cpp logger.cpp statusbar.cpp pukdialog.cpp aboutdialog.cpp x509certificatehandler.cpp )

#Adding headers
set(HEADERS mainwidget.h cardcontrolhandler.h statusbar.h logger.h pukdialog.h aboutdialog.h )

#Adding Resources
set(RESOURCES ../buergerkarte.qrc)

#Generation of Qt Stuff (moc files, ui headers and resource files)
QT4_WRAP_CPP( HEADERS_MOC ${HEADERS})
QT4_WRAP_UI( FORMS_HEADERS ${FORMS})
QT4_ADD_RESOURCES( RESOURCES_RCC ${RESOURCES})

#We have to include the binary directory because the qt stuff like ui headers will be generated there
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR})

#Creates executable from sources, moc headers (Qt), forms headers (Qt) and resources (Qt)
add_executable (${PROJECT_NAME} ${SRCS} ${HEADERS_MOC} ${FORMS_HEADERS} ${RESOURCES_RCC} )

#Linking with special arguments for windows or linux platform
if( WIN32 ) 
  set(LIBOPENSC ${LIB_DIR}/opensc/lib/windows/libopensc.a)
  target_link_libraries(${PROJECT_NAME} ${QT_QTMAIN_LIBRARY} ${QT_LIBRARIES} ${LIBOPENSC} ${ZLIB_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_DL_LIBS} -llcms -llzma -Wl,-subsystem,windows )  
  #target_link_libraries(${PROJECT_NAME} ${QT_QTMAIN_LIBRARY} ${QT_LIBRARIES} /tmp/libpkcs15init.a ${LIBOPENSC} ${ZLIB_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_DL_LIBS} -llcms -llzma)  
elseif( APPLE )
  set(LIBOPENSC ${LIB_DIR}/opensc/lib/mac_osx/libopensc.dylib)
  target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${LIBOPENSC}  ${ZLIB_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_DL_LIBS} )

  #set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
else()
  set(LIBOPENSC ${LIB_DIR}/opensc/lib/linux/libopensc.a)
  target_link_libraries(${PROJECT_NAME} ${QT_LIBRARIES} ${LIBOPENSC} ${ZLIB_LIBRARIES} ${OPENSSL_LIBRARIES} ${CMAKE_DL_LIBS} )

  set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")
endif()
