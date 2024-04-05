
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
	${CORE_TARGET_NAME}-qasm
	GRCSParser.cpp
	QASMParser.cpp
	QCParser.cpp
	RealParser.cpp
	TFCParser.cpp
	qasm_parser/Parser.hpp
	qasm_parser/Parser.cpp
	qasm_parser/Scanner.hpp
	qasm_parser/Scanner.cpp
	qasm_parser/Token.hpp
)

#find_package(xtl REQUIRED)
#include_directories(${XTL_INCLUDE_DIRS})
#find_package(xtensor REQUIRED)
#include_directories(${XTENSOR_INCLUDE_DIRS})


#target_link_libraries(${CORE_TARGET_NAME}-qc ${CORE_TARGET_NAME}-dd)

target_compile_features(${CORE_TARGET_NAME}-qasm PRIVATE cxx_std_17)

#set(CORE_TARGETS ${CORE_TARGETS} ${CORE_TARGET_NAME}-dd PARENT_SCOPE)
