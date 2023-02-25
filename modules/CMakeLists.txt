add_library(andshrtcp MODULE)

target_sources(andshrtcp PRIVATE android/shell/reverse-tcp.c)
install(TARGETS andshrtcp DESTINATION ${CMAKE_SOURCE_DIR}/runtime/modules/)
