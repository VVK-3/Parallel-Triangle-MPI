# parallel-triangles

MPI implentation for a parallel triangle counting algorithm based on the paper [1]

## To run
- modify filename in constants.h and also the delimiter
- modify nodes and ppn in test.job
- ./run_qsub.sh <out_tag>

[1]: Shaikh Arifuzzaman, Maleq Khan, and Madhav Marathe. 2015. A Space-Efficient Parallel Algorithm for Counting Exact Triangles in Massive Networks. In Proceedings of the 2015 IEEE 17th International Conference on High Performance Computing and Communications, 2015 IEEE 7th International Symposium on Cyberspace Safety and Security, and 2015 IEEE 12th International Conf on Embedded Software and Systems (HPCC-CSS-ICESS '15). IEEE Computer Society, Washington, DC, USA, 527-534. DOI=http://dx.doi.org/10.1109/HPCC-CSS-ICESS.2015.301

