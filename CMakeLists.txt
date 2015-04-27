cmake_minimum_required(VERSION 2.6)

project(mpi_data_tool CXX)

get_filename_component(REPO_DIR ${PROJECT_SOURCE_DIR} DIRECTORY)

# Eigen setting
include_directories(SYSTEM "eigen-eigen-10219c95fe65")

# mpi_data_tool_core setting
set(MDT
  mdt_core/Integer.cpp
  mdt_core/Number.cpp
  mdt_core/in_range.cpp
  mdt_core/file_read_error.cpp
  mdt_core/file_write_error.cpp
  mdt_core/file_open.cpp
  mdt_core/count_line.cpp
  mdt_core/SparseFormatter.cpp
)
add_library(mdt_core STATIC ${MDT})

# ssvm setting
set(SSVM
  ssvm/SparseData.cpp
  ssvm/SSVC.cpp
  ssvm/SSVR.cpp
  ssvm/LinearSSVC.cpp
  ssvm/LinearSSVR.cpp
  ssvm/RBFRSVC.cpp
  ssvm/RBFRSVR.cpp
)
add_library(ssvm STATIC ${SSVM})

# vc setting
if (${CMAKE_GENERATOR} MATCHES "^Visual Studio")
  set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Od /Zi")
  set(CMAKE_CXX_FLAGS_RELEASE "/MD /Ox")

  # mpi setting
  set(MPI_ROOT "C:/Program Files (x86)/MPICH2")
  include_directories(SYSTEM "${MPI_ROOT}/include")
  add_library(mpi STATIC IMPORTED)
  set_property(TARGET mpi PROPERTY IMPORTED_LOCATION "${MPI_ROOT}/lib/mpich2.lib")

  # boost-1_56 setting
  set(BOOST_ROOT "C:/Boost")
  set(BOOST_LIB "${BOOST_ROOT}/lib")
  include_directories(SYSTEM "${BOOST_ROOT}/include/boost-1_56")
  add_library(boost_system STATIC IMPORTED)
  add_library(boost_filesystem STATIC IMPORTED)
  add_library(boost_serialization STATIC IMPORTED)
  add_library(boost_mpi STATIC IMPORTED)
  set_property(TARGET boost_system PROPERTY IMPORTED_LOCATION "${BOOST_LIB}/libboost_system-vc110-mt-1_56.lib")
  set_property(TARGET boost_filesystem PROPERTY IMPORTED_LOCATION "${BOOST_LIB}/libboost_filesystem-vc110-mt-1_56.lib")
  set_property(TARGET boost_serialization PROPERTY IMPORTED_LOCATION "${BOOST_LIB}/libboost_serialization-vc110-mt-1_56.lib")
  set_property(TARGET boost_mpi PROPERTY IMPORTED_LOCATION "${BOOST_LIB}/libboost_mpi-vc110-mt-1_56.lib")

  # ssvm_test setting
  function(SSVM_TEST_EXECUTE NAME)
    add_executable(${NAME} ssvm_test/${NAME}.cpp)
    target_link_libraries(${NAME} mdt_core ssvm boost_serialization)
  endfunction(SSVM_TEST_EXECUTE)
  SSVM_TEST_EXECUTE(linear_ssvc_predict)
  SSVM_TEST_EXECUTE(linear_ssvc_test)
  SSVM_TEST_EXECUTE(linear_ssvc_train)
  SSVM_TEST_EXECUTE(linear_ssvc_train_sparse)
  SSVM_TEST_EXECUTE(linear_ssvr_predict)
  SSVM_TEST_EXECUTE(linear_ssvr_test)
  SSVM_TEST_EXECUTE(linear_ssvr_train)
  SSVM_TEST_EXECUTE(rbf_rsvc_predict)
  SSVM_TEST_EXECUTE(rbf_rsvc_test)
  SSVM_TEST_EXECUTE(rbf_rsvc_train)
  SSVM_TEST_EXECUTE(rbf_rsvr_predict)
  SSVM_TEST_EXECUTE(rbf_rsvr_test)
  SSVM_TEST_EXECUTE(rbf_rsvr_train)
  SSVM_TEST_EXECUTE(test_sparse)
  SSVM_TEST_EXECUTE(test_ssvc)
  SSVM_TEST_EXECUTE(test_ssvc_sparse)
  SSVM_TEST_EXECUTE(test_ssvr)

  # feature setting
  function(FEATURE_EXECUTE NAME)
    add_executable(${NAME} feature/${NAME}.cpp)
    target_link_libraries(${NAME} mdt_core ssvm boost_system boost_filesystem boost_serialization mpi boost_mpi)
  endfunction(FEATURE_EXECUTE)
  FEATURE_EXECUTE(clean_element)
  FEATURE_EXECUTE(count_feature)
  FEATURE_EXECUTE(extract_dimension)
  FEATURE_EXECUTE(extract_instance_number)
  FEATURE_EXECUTE(extract_label_info)
  FEATURE_EXECUTE(extract_target_info)
  FEATURE_EXECUTE(libsvm_separate_target_sparse)
  FEATURE_EXECUTE(merge_element_to_colume_sparse)
  FEATURE_EXECUTE(mrmr_feature)
  FEATURE_EXECUTE(orange_to_libsvm)
  FEATURE_EXECUTE(sample_split)
  FEATURE_EXECUTE(sax_feature)
  FEATURE_EXECUTE(sparse_separate_pos_neg)
  FEATURE_EXECUTE(split_by_row)
  FEATURE_EXECUTE(split_row_sparse_to_element)
  FEATURE_EXECUTE(feature_understanding)

  # desvm setting
  function(DESVM_EXECUTE NAME)
    add_executable(${NAME} desvm/${NAME}.cpp)
    target_link_libraries(${NAME} mdt_core ssvm boost_serialization mpi boost_mpi)
  endfunction(DESVM_EXECUTE)
  DESVM_EXECUTE(desvc_distributed)
  DESVM_EXECUTE(desvc_ensemble)
  DESVM_EXECUTE(desvc_test)
  DESVM_EXECUTE(desvr_distributed)
  DESVM_EXECUTE(desvr_ensemble)
  DESVM_EXECUTE(desvr_test)
# generator error
else()
  message(FATAL_ERROR "${CMAKE_GENERATOR} is not allowed")
endif()
