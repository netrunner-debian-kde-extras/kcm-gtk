project(kcm-gtk)

# Find the required Libaries
find_package(KDE4 REQUIRED)
include(KDE4Defaults)

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
include_directories(
   ${CMAKE_SOURCE_DIR}
   ${CMAKE_BINARY_DIR}
   ${KDE4_INCLUDES}
)

# Add translations
include(MacroOptionalAddSubdirectory)
macro_optional_add_subdirectory( po )

# Sources
SET(kcm_gtk_SOURCES
	kcmgtk.cpp
	gtkrcfile.cpp
	searchpaths.cpp
)

# UI files
SET(kcm_gtk_UI
	kcmgtkwidget.ui
	searchpaths.ui
)

KDE4_ADD_UI_FILES(kcm_gtk_SOURCES ${kcm_gtk_UI})

KDE4_ADD_PLUGIN(kcm_gtk ${kcm_gtk_SOURCES})

INCLUDE_DIRECTORIES(${KDE4_INCLUDE_DIR})
LINK_DIRECTORIES(${KDE4_LIB_DIR})

TARGET_LINK_LIBRARIES(kcm_gtk ${KDE4_KDECORE_LIBRARY} ${KDE4_KDEUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY})

INSTALL(
	TARGETS kcm_gtk
	DESTINATION ${PLUGIN_INSTALL_DIR}
)

INSTALL(
	FILES kcmgtk.desktop
	DESTINATION ${SERVICES_INSTALL_DIR}
)

INSTALL(
	FILES kcm_gtk.png
	DESTINATION ${ICON_INSTALL_DIR}
)
