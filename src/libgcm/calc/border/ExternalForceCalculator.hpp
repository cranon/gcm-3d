#ifndef _GCM_EXTERNAL_FORCE_CALCULATOR_H
#define _GCM_EXTERNAL_FORCE_CALCULATOR_H  1

#include <gsl/gsl_linalg.h>

#include "libgcm/calc/border/BorderCalculator.hpp"
#include "libgcm/Math.hpp"

class ExternalForceCalculator : public BorderCalculator
{
public:
    ExternalForceCalculator();
    ~ExternalForceCalculator();
    void doCalc(CalcNode& cur_node, CalcNode& new_node, RheologyMatrix3D& matrix,
                            vector<CalcNode>& previousNodes, bool inner[],
                            float outer_normal[], float scale) = 0;
    inline string getType() {
        return "ExternalForceCalculator";
    }
    void set_parameters(float sn, float st, float xv, float yv, float zv);

protected:

private:
    float normal_stress;
    float tangential_stress;
    float tangential_direction[3];

    // Used for border calculation
    gsl_matrix *U_gsl;
    gsl_vector *om_gsl;
    gsl_vector *x_gsl;
    gsl_permutation *p_gsl;
};

#endif
