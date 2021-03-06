#ifndef GCM_LINE_FIRST_ORDER_INTERPOLATOR_H
#define    GCM_LINE_FIRST_ORDER_INTERPOLATOR_H

#include <string>

#include "libgcm/Math.hpp"
#include "libgcm/Logging.hpp"
#include "libgcm/interpolator/TetrInterpolator.hpp"

using namespace std;
using namespace gcm;

namespace gcm {
    class CalcNode;

    class LineFirstOrderInterpolator : public TetrInterpolator {
    public:
        LineFirstOrderInterpolator();
        ~LineFirstOrderInterpolator();
        void interpolate( CalcNode& node, CalcNode& node0, CalcNode& node1 );
    private:
        USE_LOGGER;
    };
}

#endif    /* GCM_LINE_FIRST_ORDER_INTERPOLATOR_H */

