set(COMPONENT_ADD_INCLUDEDIRS ". include ../Actuator/include")
set(COMPONENT_SRCS "gatts.cpp")
set(COMPONENT_REQUIRES nvs_flash bt)
register_component()
