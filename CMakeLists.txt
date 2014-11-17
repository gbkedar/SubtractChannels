cmake_minimum_required(VERSION 2.8)

if(COMMAND CMAKE_POLICY)
  CMAKE_POLICY(SET CMP0003 NEW)
  CMAKE_POLICY(SET CMP0005 OLD)
endif(COMMAND CMAKE_POLICY)

project( BrainChannels )

find_package(ITK REQUIRED)
include( ${ITK_USE_FILE} )

# Find Qt
find_package(Qt4)
if(QT4_FOUND)
	SET(QT_USE_QTOPENGL TRUE)
    include( ${QT_USE_FILE} )
else(QT4_FOUND)
    message(FATAL_ERROR "Cannot build without Qt4")
endif(QT4_FOUND)

if(MSVC)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /openmp")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /openmp")
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fopenmp")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
endif (MSVC)
#set(CMAKE_INCLUDE_CURRENT_DIR ON)

SET( ChannelsEditor_SRCS
  chip.cpp
  imageManager.cpp
  view.cpp
  mainwindow.cpp
)

SET( ChannelsEditor_HDRS
  chip.h
  imageManager.h
  view.h
  mainwindow.h
)

SET( ChannelsEditor_MOC_REQD_HDRS
  chip.h
  view.h
  mainwindow.h
)

SET( ChannelsEditor_RCS
  images.qrc
)

SET( ChannelsEditor_UIS
)

# this command will generate rules that will run rcc on all files from SAMPLE_RCS
# in result SAMPLE_RC_SRCS variable will contain paths to files produced by rcc
QT4_ADD_RESOURCES( ChannelsEditor_RC_SRCS ${ChannelsEditor_RCS} )

# this will run uic on .ui files:
QT4_WRAP_UI( ChannelsEditor_UI_HDRS ${ChannelsEditor_UIS} )

# and finally this will run moc:
QT4_WRAP_CPP( ChannelsEditor_MOC_SRCS ${ChannelsEditor_MOC_REQD_HDRS} )

SET (CHANNELSEDITOR_SOURCES
  main.cpp
  ${ChannelsEditor_SRCS}
  ${ChannelsEditor_HDRS}
  ${ChannelsEditor_MOC_SRCS}
  ${ChannelsEditor_UI_HDRS}
  ${ChannelsEditor_RC_SRCS}
)

SET (CHANNELSEDITOR_LIBRARIES
  ${ITK_LIBRARIES}
  ${QT_LIBRARIES}
)

ADD_EXECUTABLE(ChannelsEditor ${CHANNELSEDITOR_SOURCES})
target_link_libraries(ChannelsEditor ${CHANNELSEDITOR_LIBRARIES})
#MAKE A GUI EXECUTABLE, NOT A CONSOLE APPLICATION
#SET_TARGET_PROPERTIES( ChannelsEditor PROPERTIES WIN32_EXECUTABLE 1)
#OR MAKE A CONSOLE APPLICATION
SET_TARGET_PROPERTIES( ChannelsEditor PROPERTIES WIN32_EXECUTABLE 0)