ROOT = ../..
EIGEN = $(ROOT)/eigen-eigen-10219c95fe65
MDT_CORE = $(ROOT)/mdt_core
SSVM = $(ROOT)/ssvm
SSVM_TEST = $(ROOT)/ssvm_test
FEATURE = $(ROOT)/feature
DESVM = $(ROOT)/desvm
MDT_LIB = Integer.o Number.o in_range.o file_read_error.o file_write_error.o file_open.o count_line.o SparseFormatter.o
SSVM_LIB = SparseData.o SSVC.o SSVR.o LinearSSVC.o LinearSSVR.o RBFRSVC.o RBFRSVR.o

CXX = clang++
CXXFLAGS = -std=c++0x -O3 -W -I$(EIGEN)
OBJ_EXECUTE = $(CXX) $(CXXFLAGS) -c
SSVM_TEST_EXECUTE = $(CXX) $(CXXFLAGS) -lboost_serialization $(MDT_LIB) $(SSVM_LIB)
FEATURE_EXECUTE = $(CXX) $(CXXFLAGS) -lboost_system -lboost_filesystem -lboost_serialization -lmpi -lboost_mpi $(MDT_LIB) $(SSVM_LIB)
DESVM_EXECUTE = $(CXX) $(CXXFLAGS) -lboost_serialization -lmpi -lboost_mpi $(MDT_LIB) $(SSVM_LIB)



all: ssvm_test feature desvm
#	export DYLD_LIBRARY_PATH="/usr/local/Cellar/gcc/4.9.2_1/lib/gcc/4.9"

mdt_core: $(MDT_LIB)

Integer.o: $(MDT_CORE)/Integer.cpp
	$(OBJ_EXECUTE) $(MDT_CORE)/Integer.cpp -o Integer.o

Number.o: $(MDT_CORE)/Number.cpp
	$(OBJ_EXECUTE) $(MDT_CORE)/Number.cpp -o Number.o

in_range.o: $(MDT_CORE)/in_range.cpp
	$(OBJ_EXECUTE) $(MDT_CORE)/in_range.cpp -o in_range.o

file_read_error.o: $(MDT_CORE)/file_read_error.cpp
	$(OBJ_EXECUTE) $(MDT_CORE)/file_read_error.cpp -o file_read_error.o

file_write_error.o: $(MDT_CORE)/file_write_error.cpp
	$(OBJ_EXECUTE) $(MDT_CORE)/file_write_error.cpp -o file_write_error.o

file_open.o: $(MDT_CORE)/file_open.cpp
	$(OBJ_EXECUTE) $(MDT_CORE)/file_open.cpp -o file_open.o

count_line.o: $(MDT_CORE)/count_line.cpp
	$(OBJ_EXECUTE) $(MDT_CORE)/count_line.cpp -o count_line.o

SparseFormatter.o: $(MDT_CORE)/SparseFormatter.cpp
	$(OBJ_EXECUTE) $(MDT_CORE)/SparseFormatter.cpp -o SparseFormatter.o



ssvm: mdt_core $(SSVM_LIB)

SparseData.o: $(SSVM)/SparseData.cpp
	$(OBJ_EXECUTE) $(SSVM)/SparseData.cpp -o SparseData.o

SSVC.o: $(SSVM)/SSVC.cpp
	$(OBJ_EXECUTE) $(SSVM)/SSVC.cpp -o SSVC.o

SSVR.o: $(SSVM)/SSVR.cpp
	$(OBJ_EXECUTE) $(SSVM)/SSVR.cpp -o SSVR.o

LinearSSVC.o: $(SSVM)/LinearSSVC.cpp
	$(OBJ_EXECUTE) $(SSVM)/LinearSSVC.cpp -o LinearSSVC.o

LinearSSVR.o: $(SSVM)/LinearSSVR.cpp
	$(OBJ_EXECUTE) $(SSVM)/LinearSSVR.cpp -o LinearSSVR.o

RBFRSVC.o: $(SSVM)/RBFRSVC.cpp
	$(OBJ_EXECUTE) $(SSVM)/RBFRSVC.cpp -o RBFRSVC.o

RBFRSVR.o: $(SSVM)/RBFRSVR.cpp
	$(OBJ_EXECUTE) $(SSVM)/RBFRSVR.cpp -o RBFRSVR.o



ssvm_test: ssvm linear_ssvc_predict linear_ssvc_test linear_ssvc_train linear_ssvc_train_sparse linear_ssvr_predict linear_ssvr_test linear_ssvr_train rbf_rsvc_predict rbf_rsvc_test rbf_rsvc_train rbf_rsvr_predict rbf_rsvr_test rbf_rsvr_train test_sparse test_ssvc test_ssvc_sparse test_ssvr

linear_ssvc_predict: $(SSVM_TEST)/linear_ssvc_predict.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/linear_ssvc_predict.cpp -o linear_ssvc_predict

linear_ssvc_test: $(SSVM_TEST)/linear_ssvc_test.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/linear_ssvc_test.cpp -o linear_ssvc_test

linear_ssvc_train: $(SSVM_TEST)/linear_ssvc_train.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/linear_ssvc_train.cpp -o linear_ssvc_train

linear_ssvc_train_sparse: $(SSVM_TEST)/linear_ssvc_train_sparse.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/linear_ssvc_train_sparse.cpp -o linear_ssvc_train_sparse

linear_ssvr_predict: $(SSVM_TEST)/linear_ssvr_predict.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/linear_ssvr_predict.cpp -o linear_ssvr_predict

linear_ssvr_test: $(SSVM_TEST)/linear_ssvr_test.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/linear_ssvr_test.cpp -o linear_ssvr_test

linear_ssvr_train: $(SSVM_TEST)/linear_ssvr_train.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/linear_ssvr_train.cpp -o linear_ssvr_train

rbf_rsvc_predict: $(SSVM_TEST)/rbf_rsvc_predict.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/rbf_rsvc_predict.cpp -o rbf_rsvc_predict

rbf_rsvc_test: $(SSVM_TEST)/rbf_rsvc_test.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/rbf_rsvc_test.cpp -o rbf_rsvc_test

rbf_rsvc_train: $(SSVM_TEST)/rbf_rsvc_train.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/rbf_rsvc_train.cpp -o rbf_rsvc_train

rbf_rsvr_predict: $(SSVM_TEST)/rbf_rsvr_predict.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/rbf_rsvr_predict.cpp -o rbf_rsvr_predict

rbf_rsvr_test: $(SSVM_TEST)/rbf_rsvr_test.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/rbf_rsvr_test.cpp -o rbf_rsvr_test

rbf_rsvr_train: $(SSVM_TEST)/rbf_rsvr_train.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/rbf_rsvr_train.cpp -o rbf_rsvr_train

test_sparse: $(SSVM_TEST)/test_sparse.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/test_sparse.cpp -o test_sparse

test_ssvc: $(SSVM_TEST)/test_ssvc.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/test_ssvc.cpp -o test_ssvc

test_ssvc_sparse: $(SSVM_TEST)/test_ssvc_sparse.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/test_ssvc_sparse.cpp -o test_ssvc_sparse

test_ssvr: $(SSVM_TEST)/test_ssvr.cpp
	$(SSVM_TEST_EXECUTE) $(SSVM_TEST)/test_ssvr.cpp -o test_ssvr



feature: ssvm clean_element cor_feature count_feature extract_dimension extract_instance_number extract_label_info extract_target_info libsvm_separate_target_sparse mean_feature merge_element_to_colume_sparse min_max_feature mrmr_feature orange_to_libsvm sample_split sax_feature sparse_separate_pos_neg split_by_row split_row_sparse_to_element squared_sum_feature std_feature

clean_element: $(FEATURE)/clean_element.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/clean_element.cpp -o clean_element

cor_feature: $(FEATURE)/cor_feature.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/cor_feature.cpp -o cor_feature

count_feature: $(FEATURE)/count_feature.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/count_feature.cpp -o count_feature

extract_dimension: $(FEATURE)/extract_dimension.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/extract_dimension.cpp -o extract_dimension

extract_instance_number: $(FEATURE)/extract_instance_number.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/extract_instance_number.cpp -o extract_instance_number

extract_label_info: $(FEATURE)/extract_label_info.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/extract_label_info.cpp -o extract_label_info

extract_target_info: $(FEATURE)/extract_target_info.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/extract_target_info.cpp -o extract_target_info

libsvm_separate_target_sparse: $(FEATURE)/libsvm_separate_target_sparse.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/libsvm_separate_target_sparse.cpp -o libsvm_separate_target_sparse

mean_feature: $(FEATURE)/mean_feature.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/mean_feature.cpp -o mean_feature

merge_element_to_colume_sparse: $(FEATURE)/merge_element_to_colume_sparse.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/merge_element_to_colume_sparse.cpp -o merge_element_to_colume_sparse

min_max_feature: $(FEATURE)/min_max_feature.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/min_max_feature.cpp -o min_max_feature

mrmr_feature: $(FEATURE)/mrmr_feature.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/mrmr_feature.cpp -o mrmr_feature

orange_to_libsvm: $(FEATURE)/orange_to_libsvm.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/orange_to_libsvm.cpp -o orange_to_libsvm

sample_split: $(FEATURE)/sample_split.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/sample_split.cpp -o sample_split

sax_feature: $(FEATURE)/sax_feature.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/sax_feature.cpp -o sax_feature

sparse_separate_pos_neg: $(FEATURE)/sparse_separate_pos_neg.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/sparse_separate_pos_neg.cpp -o sparse_separate_pos_neg

split_by_row: $(FEATURE)/split_by_row.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/split_by_row.cpp -o split_by_row

split_row_sparse_to_element: $(FEATURE)/split_row_sparse_to_element.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/split_row_sparse_to_element.cpp -o split_row_sparse_to_element

squared_sum_feature: $(FEATURE)/squared_sum_feature.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/squared_sum_feature.cpp -o squared_sum_feature

std_feature: $(FEATURE)/std_feature.cpp
	$(FEATURE_EXECUTE) $(FEATURE)/std_feature.cpp -o std_feature



desvm: ssvm desvc_distributed desvc_ensemble desvc_test desvr_distributed desvr_ensemble desvr_test

desvc_distributed: $(DESVM)/desvc_distributed.cpp
	$(DESVM_EXECUTE) $(DESVM)/desvc_distributed.cpp -o desvc_distributed

desvc_ensemble: $(DESVM)/desvc_ensemble.cpp
	$(DESVM_EXECUTE) $(DESVM)/desvc_ensemble.cpp -o desvc_ensemble

desvc_test: $(DESVM)/desvc_test.cpp
	$(DESVM_EXECUTE) $(DESVM)/desvc_test.cpp -o desvc_test

desvr_distributed: $(DESVM)/desvr_distributed.cpp
	$(DESVM_EXECUTE) $(DESVM)/desvr_distributed.cpp -o desvr_distributed

desvr_ensemble: $(DESVM)/desvr_ensemble.cpp
	$(DESVM_EXECUTE) $(DESVM)/desvr_ensemble.cpp -o desvr_ensemble

desvr_test: $(DESVM)/desvr_test.cpp
	$(DESVM_EXECUTE) $(DESVM)/desvr_test.cpp -o desvr_test



clean:
	find . ! -name Makefile -maxdepth 1 -type f -delete
