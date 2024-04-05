
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
	${CORE_TARGET_NAME}-qc
	ClassicControlledOperation.hpp
	CompoundOperation.hpp
	Control.hpp
	Expression.hpp
	NonUnitaryOperation.hpp
	Operation.hpp
	OpType.hpp
	StandardOperation.hpp
	SymbolicOperation.hpp
	Expression.cpp
	NonUnitaryOperation.cpp
	Operation.cpp
	StandardOperation.cpp
	SymbolicOperation.cpp
)

#find_package(xtl REQUIRED)
#include_directories(${XTL_INCLUDE_DIRS})
#find_package(xtensor REQUIRED)
#include_directories(${XTENSOR_INCLUDE_DIRS})


#target_link_libraries(${CORE_TARGET_NAME}-qc ${CORE_TARGET_NAME}-dd)

target_compile_features(${CORE_TARGET_NAME}-qc PRIVATE cxx_std_17)

#set(CORE_TARGETS ${CORE_TARGETS} ${CORE_TARGET_NAME}-dd PARENT_SCOPE)
