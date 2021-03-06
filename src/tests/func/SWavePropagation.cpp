#include <vector>
#include <gtest/gtest.h>


#include "tests/func/util.hpp"

#include "libgcm/Exception.hpp"
#include "libgcm/materials/IsotropicElasticMaterial.hpp"
#include "libgcm/node/CalcNode.hpp"

/*
 * Analytics for the test described in tasks/tests/s-wave-test.xml
 * Sets analytical values for CalcNode object provided
 * It does not take into account waves from border, so it works only for wave axis and not too long
 * For high accuracy node should have coordinates (0; 0; z), where -5 < z < 5
 */
void setSWaveAnalytics(CalcNode& node, float t, Engine& engine)
{
    // Parameters from task file
    float LEFT_MARK_START = 1.0;
    float RIGHT_MARK_START = 3.0;
    float WAVE_AMPLITUDE_SCALE = 0.01;

    IsotropicElasticMaterial* mat = dynamic_cast<IsotropicElasticMaterial*> (engine.getMaterial("testMaterial"));
    ASSERT_TRUE(mat);
    float mu = mat->getMu();
    float rho = mat->getRho();

    if (node.z < -5 || node.z > 5)
        THROW_INVALID_INPUT("Z is out of acceptable range");
    if (t < 0 || t > 0.02)
        THROW_INVALID_INPUT("T is out of acceptable range");

    float sWaveVelocity = sqrt(mu / rho);
    float leftMark = LEFT_MARK_START - t * sWaveVelocity;
    float rightMark = RIGHT_MARK_START - t * sWaveVelocity;

    node.vx = node.vy = node.vz = 0;
    node.sxx = node.sxy = node.sxz = node.syy = node.syz = node.szz = 0;

    if (node.z >= leftMark && node.z <= rightMark) {
        node.vx = sWaveVelocity * WAVE_AMPLITUDE_SCALE;
        node.sxz = mu * WAVE_AMPLITUDE_SCALE;
    }
}

TEST(Waves, SWavePropagation)
{
    // Major test parameters

    // Number of time steps
    int STEPS = 10;

    // Check values and draw graphs along this line
    SnapshotLine line;
    line.startPoint = {0.0, 0.0, -5.0};
    line.endPoint = {0.0, 0.0, 5.0};
    line.numberOfPoints = 50;

    // Thresholds
    float ALLOWED_VALUE_DEVIATION_PERCENT = 0.1;
    int ALLOWED_NUMBER_OF_BAD_NODES = 8; // 2 fronts x 2 nodes per front x 2 without any reason

    runTaskAsTest("tasks/tests/s-wave-test.xml", setSWaveAnalytics, STEPS, line, { "vx", "sxz" }, ALLOWED_VALUE_DEVIATION_PERCENT, ALLOWED_NUMBER_OF_BAD_NODES);
}
