
#set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_library(
	${CORE_TARGET_NAME}-dd
	Edge.cpp
	Complex.hpp
	ComplexCache.hpp
	ComplexNumbers.hpp
	ComplexTable.hpp
	ComplexValue.hpp
	ComputeTable.hpp
	Control.hpp
	DDpackageConfig.hpp
	Definitions.hpp
	Edge.hpp
	Export.hpp
	GateMatrixDefinitions.hpp
	Node.hpp
	Operations.hpp
	Package.hpp
	Package_fwd.hpp
	Tdd.hpp
	Tensor.hpp
	UniqueTable.hpp
	Node.cpp
	Operations.cpp
)

find_package(xtl REQUIRED)
include_directories(${XTL_INCLUDE_DIRS})
find_package(xtensor REQUIRED)
include_directories(${XTENSOR_INCLUDE_DIRS})
#include_directories(${CORE_TARGET_NAME}-qc)

#target_include_directories(${CORE_TARGET_NAME}-dd PRIVATE ${CORE_TARGET_NAME}-qc)

#target_include_directories(${CORE_TARGET_NAME}-dd PUBLIC ${CORE_TARGET_NAME}-qc)
target_link_directories(${CORE_TARGET_NAME}-dd PRIVATE "../operations")

target_link_libraries(${CORE_TARGET_NAME}-dd xtl xtensor ${CORE_TARGET_NAME}-qc)



target_compile_features(${CORE_TARGET_NAME}-dd PRIVATE cxx_std_20)

#set(CORE_TARGETS ${CORE_TARGETS} ${CORE_TARGET_NAME}-dd PARENT_SCOPE)
