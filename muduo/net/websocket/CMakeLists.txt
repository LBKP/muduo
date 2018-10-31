set(wss_SRCS
  WebSocketContext.cpp
  WebSocketServer.cpp
  WebSocketConnection.cpp
  )

add_library(muduo_wss ${wss_SRCS})
target_link_libraries(muduo_wss muduo_net)

install(TARGETS muduo_wss DESTINATION lib)
set(HEADERS
  WebSocketContext.h
  WebSocketServer.h
  WebSocketConnection.h
  WebSocketTypes.h
  )
install(FILES ${HEADERS} DESTINATION include/muduo/net/websocket)

if(NOT CMAKE_BUILD_NO_EXAMPLES)
add_executable(websocketserver main.cpp)
target_link_libraries(websocketserver muduo_wss)

#if(BOOSTTEST_LIBRARY)
#add_executable(httprequest_unittest tests/HttpRequest_unittest.cc)
#target_link_libraries(httprequest_unittest muduo_http boost_unit_test_framework)
#endif()

endif()

# add_subdirectory(tests)
