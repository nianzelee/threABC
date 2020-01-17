rt exp_TCAD/dnn_mnist/benchmark/binarized_dnn_32_3_16.th
t2m
w dnn_1.aig
rt exp_TCAD/dnn_mnist/benchmark/binarized_dnn_32_3_256.th
t2m
w dnn_2.aig
time
cec -n dnn_1.aig dnn_2.aig
time
q
