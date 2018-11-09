### Related Repositores [![Version](https://img.shields.io/badge/version-1.0-blue.svg)](https://github.com/cicerolp/qds) [![GitHub license](https://img.shields.io/github/license/cicerolp/qds.svg)](https://github.com/cicerolp/qds/blob/master/LICENSE)



- [Main Source Code](https://github.com/cicerolp/qds)
- [Web Interface built with Angular@5](https://github.com/cicerolp/qds-interface)
- [Conversion Tools](https://github.com/cicerolp/qds-tools)
- [Datasets](https://github.com/cicerolp/qds-data)


# Quantile Datacube Structure
Real-Time Exploration of Large Spatiotemporal Datasets based on Order Statistics

In recent years sophisticated data structures based on datacubes have been proposed to perform interactive visual exploration of large datasets. While powerful, these approaches overlook an essential aspect of data analysis: the inherent uncertainty due to data aggregation. In this paper, we introduce the Quantile Datacube Structure (QDS) to bridge this gap by supporting interactive uncertainty visualization and exploration based on order statistics. The idea behind our method is that while it is not possible to exactly store order statistics in a datacube it is indeed possible to do it approximately. To achieve this, QDS makes use of an efficient non-parametric distribution approximation scheme called p-digest. To avoid the large memory footprint common to datacubes, QDS employs a novel datacube indexing scheme that reduces the memory usage of previous datacube methods. This enables interactive slicing and dicing while accurately approximating the distribution of quantitative variables of interest. We present two case studies that showcase the ability of QDS to not only build order statistics based visualizations but also to perform interactive event detection on datasets with up to hundreds of millions of records. Furthermore, we present extensive experimental results that validate the usefulness of QDS regarding memory usage and accuracy in the approximation of order statistics for real-world datasets

### Authors
- Cícero L. Pahins, Nivan Ferreira, and João L. Comba

## How to Build (Linux, Mac and Windows are supported)

- Dependencies: Boost 1.62 or later, CMake 3.0 or later

- To build and run on Linux:

1- mkdir build-release && cd build-release

3- cmake -DCMAKE_BUILD_TYPE=Release ..

4- make

5- cd .. && cp build-release/nds .

6- ./nds
