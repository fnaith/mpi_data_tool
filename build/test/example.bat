..\vc\Release\test_sparse
..\vc\Release\test_ssvc
..\vc\Release\test_ssvc_sparse
..\vc\Release\test_ssvr



..\vc\Release\linear_ssvc_train iris.pos iris.neg 4 100 iris.model
..\vc\Release\linear_ssvc_predict iris.model iris.pos
..\vc\Release\linear_ssvc_predict iris.model iris.neg
..\vc\Release\linear_ssvc_test iris.model iris.pos iris.neg

..\vc\Release\linear_ssvc_train_sparse iris.pos iris.neg 4 100 iris.model
..\vc\Release\linear_ssvc_predict iris.model iris.pos
..\vc\Release\linear_ssvc_predict iris.model iris.neg
..\vc\Release\linear_ssvc_test iris.model iris.pos iris.neg

..\vc\Release\linear_ssvr_train iris.A iris.y 4 100 0.01 iris.model
..\vc\Release\linear_ssvr_predict iris.model iris.A
..\vc\Release\linear_ssvr_test iris.model iris.A iris.y



..\vc\Release\rbf_rsvc_train iris.pos iris.neg 4 0.2 0.1 100 iris.model
..\vc\Release\rbf_rsvc_predict iris.model iris.pos
..\vc\Release\rbf_rsvc_predict iris.model iris.neg
..\vc\Release\rbf_rsvc_test iris.model iris.pos iris.neg

..\vc\Release\rbf_rsvr_train iris.A iris.y 4 0.2 0.1 100 0.01 iris.model
..\vc\Release\rbf_rsvr_predict iris.model iris.A
..\vc\Release\rbf_rsvr_test iris.model iris.A iris.y



..\vc\Release\orange_to_libsvm . g1_5.orange g1_5.libsvm
..\vc\Release\libsvm_separate_target_sparse . g1_5.libsvm g1_5.X g1_5.y
..\vc\Release\extract_instance_number . g1_5.y
..\vc\Release\extract_dimension . g1_5.X
..\vc\Release\extract_target_info . g1_5.y
..\vc\Release\count_feature . g1_5.X
..\vc\Release\split_row_sparse_to_element . g1_5.X feature 20
..\vc\Release\merge_element_to_colume_sparse . g1_5.X feature 20
..\vc\Release\clean_element . g1_5.X feature 20

"C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\min_max_feature . g1_5.X feature 20
"C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\mean_feature . g1_5.X feature 20 g1_5.y
"C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\squared_sum_feature . g1_5.X feature 20
"C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\std_feature . g1_5.X feature 20 g1_5.y
"C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\cor_feature . g1_5.X feature 20 g1_5.y
REM "C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\sax_feature . g1_5.X feature 20 g1_5.y 2
"C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\mrmr_feature . g1_5.X feature 20 g1_5.y 1
"C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\feature_understanding . g1_5.X feature 20 g1_5.y min max mean std cor

..\vc\Release\split_by_row . g1_5.X train 24 g1_5.y
..\vc\Release\split_by_row . g1_5.y train 24 g1_5.y
..\vc\Release\sample_split . train 24 g1_5.X g1_5.X.sample 0.2 4
..\vc\Release\sample_split . train 24 g1_5.y g1_5.y.sample 0.2 4
"C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\desvr_distributed . train g1_5.model 24 100 g1_5.X g1_5.y 0.2 0.1 100 0.01
"C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\desvr_ensemble . train g1_5.model 24 100 g1_5.X g1_5.y 100 0.01 3 g1_5.X.sample g1_5.y.sample 4
"C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\desvr_test . train train g1_5.model 24 100 g1_5.X g1_5.y 24 3 train\24\g1_5.output



..\vc\Release\extract_label_info . g1_5.y
..\vc\Release\sparse_separate_pos_neg . g1_5.X g1_5.y 1 g1_5.pos g1_5.neg
..\vc\Release\split_by_row . g1_5.pos train 32 g1_5.y
..\vc\Release\split_by_row . g1_5.neg train 32 g1_5.y
..\vc\Release\sample_split . train 32 g1_5.pos g1_5.pos.sample 0.2 4
..\vc\Release\sample_split . train 32 g1_5.neg g1_5.neg.sample 0.2 4
"C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\desvc_distributed . train g1_5.model 32 100 g1_5.pos g1_5.neg 0.2 0.1 100
"C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\desvc_ensemble . train g1_5.model 32 100 g1_5.pos g1_5.neg 100 3 g1_5.pos.sample g1_5.neg.sample 4
"C:\Program Files (x86)\MPICH2\bin\mpiexec" -n 4 ..\vc\Release\desvc_test . train train g1_5.model 32 100 g1_5.pos g1_5.neg 32 3 train\32\g1_5.output
