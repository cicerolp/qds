### Related Repositores [![Version](https://img.shields.io/badge/version-1.0-blue.svg)](https://github.com/cicerolp/qds) [![GitHub license](https://img.shields.io/github/license/cicerolp/qds.svg)](https://github.com/cicerolp/qds/blob/master/LICENSE)

- [Main Source Code](https://github.com/cicerolp/qds)
- [Web Interface built with Angular@5](https://github.com/cicerolp/qds-interface)
- [Conversion Tools](https://github.com/cicerolp/qds-tools)
- [Datasets](https://github.com/cicerolp/qds-data)

# Quantile Datacube Structure
Real-Time Exploration of Large Spatiotemporal Datasets based on Order Statistics

In recent years sophisticated data structures based on datacubes have been proposed to perform interactive visual exploration of large datasets. While powerful, these approaches overlook the important fact that aggregations used to produce datacubes do not represent the actual distribution of the data being analyzed. As a result, these methods might produce biased results as well as hide important features in the data. In this paper, we introduce the Quantile Datacube Structure (QDS) that bridges this gap by supporting interactive visual exploration based on order statistics. The idea behind our method is that while it is not possible to exactly store order statistics in a datacube it is indeed possible to do it approximately. To achieve this, QDS makes use of an efficient non-parametric distribution approximation scheme called p-digest. Furthermore, QDS employs a novel datacube indexing scheme that reduces the memory usage of previous datacube methods. This enables interactive slicing and dicing while accurately approximating the distribution of quantitative variables of interest. We present two case studies that illustrate the ability of QDS to not only build order statistics based visualizations interactively but also to perform event detection on datasets with up to hundreds of millions of records in real-time. Finally, we present extensive experimental results that validate the effectiveness of QDS regarding memory usage and accuracy in the approximation of order statistics for real-world datasets.

### Authors
- Cícero A. L. Pahins, Nivan Ferreira, and João L. Comba

# How to Build (Linux, Mac and Windows are supported)

- Dependencies: 
    * `Boost 1.62` or later.
    * `CMake 3.0` or later.
    * A modern C++ compiler that supports C++14.

- To build and run on Linux:

    1. `mkdir build-release && cd build-release`
    2. `cmake -DCMAKE_BUILD_TYPE=Release ..`
    3. `make -j 8`
    4. `cd .. && cp build-release/nds .`
    5. `./nds`


# How to Run the Demo

Below are the steps to run a demo that explore various QDS features. Follow them in order.

1. Clone [QDS](https://github.com/cicerolp/qds), [QDS Data](https://github.com/cicerolp/qds-data) and [QDS Interface](https://github.com/cicerolp/qds-interface) repositories.
    * Note that you'll need git-lfs installed to pull the data.
    
2. QDS Server
    * Change the current directory to `QDS` root
    * Build `QDS` following the instructions (master branch)
    * Export or set the enviroment variable `NDS_DATA=<QDS_DATA_REPO_DIR>`
    * Hit `./nds -x demo/demo1.xml -x demo/demo2.xml -x demo/demo3.xml -x demo/demo4.xml -x demo/demo5.xml`
        * The demo needs approximatly 4GB of memory to run

3. QDS Interface
    * Change the current directory to `QDS Interface` root
    * Type: `npm install` to install all requeriments
    * Type: `ng serve` to deploy the development server
    * Hit [localhost:4200](http://localhost:4200) to acess the interface
        * Only available after you setup QDS server
