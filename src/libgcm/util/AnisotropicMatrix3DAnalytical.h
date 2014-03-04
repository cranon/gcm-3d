#ifndef ANISOTROPICMATRIX3DANALYTICAL_H
#define ANISOTROPICMATRIX3DANALYTICAL_H  1

#include <assert.h>

#include "util/matrixes.h"
#include "util/ThirdDegreePolynomial.h"
#include "Exception.h"
#include <iomanip>
using namespace gcm;

namespace gcm {
	
	class AnisotropicMatrix3DAnalytical
	{
	public:
		AnisotropicMatrix3DAnalytical();
		~AnisotropicMatrix3DAnalytical();
		void prepare_matrix(const AnisotropicNumbers &C, float rho, int stage);
		float max_lambda();

		void self_check();

		gcm_matrix A;
		gcm_matrix L;
		gcm_matrix U;
		gcm_matrix U1;

	private:
		void CreateAx(const AnisotropicNumbers &C, float rho);
		void CreateAy(const AnisotropicNumbers &C, float rho);
		void CreateAz(const AnisotropicNumbers &C, float rho);
		void createMatrixN(int i, int j, float *res);
		void zero_all();
		void findNonZeroSolution(float **M, float *x);
		void findNonZeroSolution(float **M, float *x, float *y);
		void findEigenVec(float *eigenVec,
					float l, float rho, const AnisotropicNumbers &C, int stage);
		void findEigenVec(float *eigenVec1, float *eigenVec2,
					float l, float rho, const AnisotropicNumbers &C, int stage);

		// TODO it is worth turning them into local vars
		float n[3][3];
	};
}

#endif

