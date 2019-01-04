//
// Created by cicerolp on 11/5/17.
//

#ifndef NDS_CONFIG_H_IN
#define NDS_CONFIG_H_IN

#include <cmath>

#define ENABLE_METRICS
/* #undef ENABLE_GPERF */

// nds options
#define NDS_ENABLE_PAYLOAD
#define NDS_ENABLE_CRS_SIMPLE

// nds tweaks
/* #undef NDS_OPTIMIZE_LEAF */

// nds sharing
#define NDS_SHARE_PIVOT
#define NDS_SHARE_PAYLOAD

#ifdef NDS_ENABLE_PAYLOAD
/* #undef ENABLE_GAUSSIAN */

/* #undef ENABLE_PDIGEST */

    #define ENABLE_RAW

    #ifdef ENABLE_PDIGEST
        #define PDIGEST_OPTIMIZE_ARRAY
        #define PDIGEST_PIECE_WISE_APPROXIMATION
        #define PDIGEST_COMPRESSION 50.f
        #define PDIGEST_ARRAY_SIZE ((uint32_t)50.f * 2)

        #define PDIGEST_BUFFER_FACTOR 5
        #define PDIGEST_BUFFER_SIZE PDIGEST_BUFFER_FACTOR * PDIGEST_ARRAY_SIZE
    #endif // ENABLE_PDIGEST
#endif // ENABLE_PDIGEST

#endif //NDS_CONFIG_H_IN
