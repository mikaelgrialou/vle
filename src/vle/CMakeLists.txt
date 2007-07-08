ADD_SUBDIRECTORY(geometry)
ADD_SUBDIRECTORY(utils)
ADD_SUBDIRECTORY(data)
ADD_SUBDIRECTORY(value)
ADD_SUBDIRECTORY(graph)
ADD_SUBDIRECTORY(vpz)
ADD_SUBDIRECTORY(devs)
ADD_SUBDIRECTORY(stream)
ADD_SUBDIRECTORY(extension)
ADD_SUBDIRECTORY(manager)
ADD_SUBDIRECTORY(translator)

CONFIGURE_FILE(vle.pc.in ${VLE_INSTALL_PREFIX}/lib/pkgconfig/vle.pc)